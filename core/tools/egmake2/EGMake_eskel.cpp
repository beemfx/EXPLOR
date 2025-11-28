// (c) 2016 Beem Media

#include "EGXMLBase.h"
#include "EGMake.h"
#include "EGCrcDb.h"
#include "EGSkelBase.h"


class EGSkelCompiler
: private IXmlBase
{
private:
	eg_byte*      m_pMem = nullptr;
	eg_size_t     m_nMemSize;

	eg_size_t     m_NextStrOfs;
	eg_uint       m_CurrentFrame;
	eg_bool       m_GotJoint:1;
	eg_bool       m_GotFrame:1;
	eg_bool       m_bUseBase64Floats:1;

	EGSkelBase::egHeader m_XmlH; //For loading with XML.
	#define SKEL_DATA_SECTION( _type_ , _var_ ) EGArray<_type_> m_##_var_##Array;
	#include "EGSkelDataSections.inc"
	#undef SKEL_DATA_SECTION

public:
	EGSkelCompiler()
	: m_NextStrOfs(0)
	, m_CurrentFrame(0)
	, m_GotJoint(false)
	, m_GotFrame(false)
	, m_bUseBase64Floats(false)
	{

	}

	~EGSkelCompiler()
	{
		Unload();
	}

	eg_bool LoadFromXML(eg_cpstr szFile)
	{
		Unload();
		zero(&m_XmlH);
		XMLLoad(szFile);
		eg_bool bSuccess = (m_XmlH.nVersion==EGSkelBase::SKEL_VERSION);
		if(bSuccess)
		{
			CalcExData();
			BuildData();
		}
		return bSuccess;
	}
private:
	void Unload()
	{
		DeallocateMemory();
		zero(&m_XmlH);
		m_GotJoint = false;
		m_GotFrame = false;
	}

/*****************************
*** XML Loading functions: ***
*****************************/
private:
	virtual void OnTag(const eg_string_base& Tag, const EGXmlAttrGetter& Getter) override final
	{
		if(m_XmlH.nVersion!=EGSkelBase::SKEL_VERSION && "eskel"!=Tag)
			return;
		
		if("eskel"==Tag)
		{
			m_XmlH.nID=EGSkelBase::SKEL_ID;
			m_NextStrOfs = 0;
		
			eg_string_big strFormat = Getter.GetString( "format" );
		
			m_XmlH.nVersion      = Getter.GetUInt( "version" );

			if(m_XmlH.nVersion != EGSkelBase::SKEL_VERSION)
			{
				EGLogf( eg_log_t::Warning , ("EGSkelBase::LoadFromXML Warning: It may have been a while since this skeleton was exported, not all data may be present."));
				m_XmlH.nVersion = EGSkelBase::SKEL_VERSION;
			}
		
			if(m_XmlH.nVersion==EGSkelBase::SKEL_VERSION)
			{
				// It's good...
			}
		
			m_bUseBase64Floats = strFormat.EqualsI("base64");		
		}
		else if("animation"==Tag)
		{
			eg_uint nID=Getter.GetUInt( "id" );
			eg_string_big strName = Getter.GetString( "name" );
			EGCrcDb::AddAndSaveIfInTool( strName );
			eg_string_big strMode = Getter.GetString( "mode" );
			EGSkelBase::egAnim anim;
			anim.nFirstFrame=Getter.GetUInt( "first_frame" );
			anim.nNumFrames=Getter.GetUInt( "frames" );
			anim.nNext=Getter.GetUInt( "next" );
			anim.fRate=30.0f;
			anim.nStrOfsName=0;
			anim.NameCrc = eg_crc("");
			anim.LoopMode=anim.LOOP_LOOPING;
		
			if(strMode.EqualsI("looping") || strMode.EqualsI(("default")) || strMode.Len() < 1)
			{
				anim.LoopMode=anim.LOOP_LOOPING;
			}
			else if( strMode.EqualsI( "once" ) )
			{
				anim.LoopMode = anim.LOOP_ONCE;
			}
			else if( strMode.EqualsI( "loop_back" ) )
			{
				anim.LoopMode = anim.LOOP_LOOP_BACK;
			}
			else
			{
				//assert(false);//This animation mode is not supported.
				EGLogf(eg_log_t::Warning , ("Warning: Animation mode %s is not supported."), strMode.String());
			}
				
			anim.NameCrc = eg_string_crc(strName);
			if( nID > 0 )
			{
				anim.nStrOfsName = AddString(strName);
				m_AnimsArray.ExtendToAtLeast( nID );
				m_AnimsArray[nID-1]=anim;
			}
		}
		else if("joint"==Tag)
		{
			m_GotJoint = true;

			if( m_GotFrame )
			{
				EGLogf( eg_log_t::Error , "Failed compilation: A joint was declared after the frames. All joints must be declared before frames." );
				m_XmlH.nVersion = 0;
				return;
			}

			EGSkelBase::egJoint joint;
			zero(&joint);
			eg_uint nID       = Getter.GetUInt( "id" );
			eg_string_big strName = Getter.GetString( "name" );
			EGCrcDb::AddAndSaveIfInTool( strName );
			joint.nParent     = Getter.GetUInt( "parent" );
		
			eg_vec3 ReadTrans;
			Getter.GetVec( "position" , reinterpret_cast<eg_real*>(&ReadTrans) , 3 , m_bUseBase64Floats );
			joint.Trans.SetTranslation( ReadTrans );
			joint.Trans.SetScale( 1.f );
			eg_vec3 v3RotT;
			Getter.GetVec( "rotation" , reinterpret_cast<eg_real*>(&v3RotT) , 3 , m_bUseBase64Floats );
			joint.Trans.SetRotation( EulerToQuat(v3RotT) );
		
			if( nID > 0 )
			{
				joint.nStrOfsName = AddString(strName);
				m_BaseSkelJointsArray.ExtendToAtLeast( nID );
				m_BaseSkelJointsArray[nID-1]=joint;
			}
		}
		else if("frame"==Tag)
		{
			m_GotFrame = true;

			if( !m_GotJoint )
			{
				EGLogf( eg_log_t::Error , "Failed compilation: A frame was declared before any joints. All joints must be declared before frames." );
				m_XmlH.nVersion = 0;
				return;
			}

			eg_uint nID=Getter.GetUInt( "id" );
		
			//Setting the current animation should be done manually
			m_CurrentFrame = 0;
			if( nID > 0 )
			{
				m_CurrentFrame = nID;
			}
		}
		else if("joint_pose"==Tag)
		{
			eg_uint nFrame = m_CurrentFrame;
		
			//If the frame is out of range, exit.
			//#pragma __WARNING__( "No way to know if this frame is in range." )
			//if(!EG_IsBetween<eg_uint>(nFrame, 1, m_XmlH.nFramesCount))
			//{
			//	assert( false );
			//	return;
			//}
			
			eg_transform Trans;
			zero(&Trans);
			eg_uint nJoint=Getter.GetUInt( "joint" );
			
			eg_vec3 ReadTrans;
			Getter.GetVec( "position" , reinterpret_cast<eg_real*>(&ReadTrans) , 3 , m_bUseBase64Floats );
			Trans.SetTranslation( ReadTrans );
			Trans.SetScale( 1.0f );
			eg_vec3 v3RotT;
			Getter.GetVec( "rotation" , reinterpret_cast<eg_real*>(&v3RotT) , 3 , m_bUseBase64Floats );
			Trans.SetRotation( EulerToQuat(v3RotT) );
		
			if( EG_IsBetween<eg_size_t>(nJoint, 1, m_BaseSkelJointsArray.Len()))
			{
				eg_size_t FrameIndex = (nFrame-1)*m_BaseSkelJointsArray.Len() + (nJoint-1);
				m_FramesArray.ExtendToAtLeast( FrameIndex+1 );
				m_FramesArray[FrameIndex] = Trans;
			}
		}
		else
		{
			assert( false );
			EGLogf( eg_log_t::Error, ("%s.OnTag: An invalid tag was specified."), XMLObjName() );
		}
	}

	virtual eg_cpstr XMLObjName()const
	{
		return ("\"Emergence Skeleton\"");
	}

	void DeallocateMemory()
	{
		if( m_pMem )
		{
			EG_SafeDeleteArray( m_pMem );
			m_pMem = nullptr;
		}
	}

	eg_uint AddString(eg_string_big& s)
	{
		for( eg_size_t i=0; i<s.Len(); i++ )
		{
			m_StringsArray.Append( s[i] );
		}
		m_StringsArray.Append( '\0' );

		eg_size_t nOfs = m_NextStrOfs;
		m_NextStrOfs = m_StringsArray.Len();
		return static_cast<eg_uint>(nOfs);
	}

	static eg_quat EulerToQuat(eg_vec3& v3Euler)
	{
		eg_quat qtX(EGMath_sin(EG_Rad(-v3Euler.x/2)), 0, 0, EGMath_cos(EG_Rad(-v3Euler.x/2)));
		eg_quat qtY(0, EGMath_sin(EG_Rad(-v3Euler.y/2)), 0, EGMath_cos(EG_Rad(-v3Euler.y/2)));
		eg_quat qtZ(0, 0, EGMath_sin(EG_Rad(-v3Euler.z/2)), EGMath_cos(EG_Rad(-v3Euler.z/2)));
		eg_quat qtFinal = qtX*qtY*qtZ;

		assert(EG_Abs(qtFinal.LenSq()-1) < 0.0001);
		return qtFinal;
	}

	eg_bool CalcExData()
	{
		//In this function we basically want to get the skeleton ready for real-time
		//transformations, so we generate all data that does not change on a per
		//frame basic. Namely the base skeleton position (B1, B2, ... in the notes).
		//As well as the final inverse matrix given by (B1*B2*...*Bn)^{-1}.

		//Firstly we must convert Euler angles for the base skeleton and
		//key frames to matrices.
		//Read the base skeleton.
		for(eg_size_t i=0; i<m_BaseSkelJointsArray.Len(); i++)
		{
			//Now create the rotation matrices (in the final format the rotation
			//matrices will probably be stored instead of the Euler angles.
			m_BaseSkelJointsArray[i].matLocal = m_BaseSkelJointsArray[i].Trans;
			m_BaseSkelJointsArray[i].nJointRef=static_cast<eg_uint>(i);
		}

		//Calculate the final matrices for the base skeleton.
		//This is simply a matter of multiplying each joint by
		//all of it's parent's matrices.
		for(eg_size_t i=0; i<m_BaseSkelJointsArray.Len(); i++)
		{
			EGSkelBase::egJoint* pTemp=&m_BaseSkelJointsArray[i];
			m_BaseSkelJointsArray[i].matFinal=pTemp->matLocal;
			while(pTemp->nParent)
			{
				pTemp=&m_BaseSkelJointsArray[pTemp->nParent-1];
				m_BaseSkelJointsArray[i].matFinal *= m_BaseSkelJointsArray[pTemp->nJointRef].matLocal;
			}
		
			//Also want the inverse of the final base skeleton joint.

			//Base skeleton should only have rotation and translation so the
			//inverse could potentially be fast, just transpose the rotation and
			//make the translation negative.

			m_BaseSkelJointsArray[i].matFinalInv = m_BaseSkelJointsArray[i].matFinal.GetInverse();
		}

		//One thing we want to do is make sure that each bone has a higher reference
		//than it's parent, this will insure the fastest possible real time animation.
		//If we ever hit the assert here we should probably write an algorithm that
		//re-orders the bones. This algorithm will need to re-order both the base
		//skeleton and the frames. If we do sort joints it will be absolutely required
		//that two skeletons with the same bone structure get sorted in the exact same
		//way.
		for(eg_uint i=0; i<m_BaseSkelJointsArray.Len(); i++)
		{
			if(!(m_BaseSkelJointsArray[i].nParent == 0 || (m_BaseSkelJointsArray[i].nJointRef == i && i > (m_BaseSkelJointsArray[i].nParent-1)) ))
			{
				assert(false);
				EGLogf(eg_log_t::Warning, ("egresource.skel Warning: This skeleton's bones are not properly sorted."));
			}
		}

		return true;
	}

	void BuildData()
	{
		m_HArray.ExtendToAtLeast( 1 );
		m_HArray[0] = m_XmlH; // Make sure the header exists.

		// Set counts...
		#define SKEL_DATA_SECTION( _type_ , _var_ ) m_XmlH.n##_var_##Count = static_cast<eg_uint>(m_##_var_##Array.Len());
		#include "EGSkelDataSections.inc"
		#undef SKEL_DATA_SECTION

		// Set offsets...
		eg_size_t DataSize = 0;
		#define SKEL_DATA_SECTION( _type_ , _var_ ) m_XmlH.Ofs##_var_## = static_cast<eg_uint>(DataSize); DataSize += EG_AlignUp( m_XmlH.n##_var_##Count * sizeof( _type_ ) );
		#include "EGSkelDataSections.inc"
		#undef SKEL_DATA_SECTION

		// Copy data to a chunk...
		m_pMem = new eg_byte[ DataSize ];
		if( nullptr != m_pMem )
		{
			m_HArray[0] = m_XmlH; // Set the header again with the correct information.
			#define SKEL_DATA_SECTION( _type_ , _var_ ) if( m_XmlH.n##_var_##Count > 0 ){ EGMem_Copy( &m_pMem[m_XmlH.Ofs##_var_##] , &m_##_var_##Array[0] , m_XmlH.n##_var_##Count*sizeof(_type_)); }
			#include "EGSkelDataSections.inc"
			#undef SKEL_DATA_SECTION
			m_nMemSize = DataSize;
		}
	}

public:
	eg_byte* GetDataAndSize(eg_size_t* OutSize)
	{
		*OutSize = m_nMemSize;
		return m_pMem;
	}
};

eg_bool EGMakeRes_eskel()
{
	EGSkelCompiler Skel;

	if(!Skel.LoadFromXML(EGMake_GetInputPath()))
	{
		return false;
	}

	eg_size_t nSize = 0;
	const eg_byte* pData = Skel.GetDataAndSize(&nSize);

	eg_bool bSuc = ::EGMake_WriteOutputFile( ::EGMake_GetOutputPath(), pData, nSize);

	return bSuc;
}