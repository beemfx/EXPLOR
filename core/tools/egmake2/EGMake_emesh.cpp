// (c) 2016 Beem Media

#include "EGMake.h"
#include "EGXMLBase.h"
#include "EGCrcDb.h"
#include "EGMeshBase.h"

class EGMeshCompiler
: private IXmlBase
{
private:
	eg_byte*  m_pMem;
	eg_size_t m_nMemSize;

	eg_size_t     m_NextStrOfs;
	eg_uint       m_XmlLastTextNode;
	eg_string_crc m_XmlPositionType;
	eg_bool       m_bUseBase64Floats:1;
	eg_bool       m_bFoundTangent:1;

	EGMeshBase::egHeader m_XmlH; //For loading with XML.
	#define MESH_DATA_SECTION( _type_ , _var_ ) EGArray<_type_> m_##_var_##Array;
	#include "EGMeshDataSections.inc"
	#undef MESH_DATA_SECTION

public:
	EGMeshCompiler()
	: m_NextStrOfs(0)
	, m_bFoundTangent(false)
	, m_XmlLastTextNode(0)
	, m_XmlPositionType(eg_crc(""))
	, m_bUseBase64Floats(false)
	{

	}
	
	~EGMeshCompiler()
	{
		Unload();
	}

	eg_bool LoadFromXML(eg_cpstr strFile)
	{
		Unload();
		zero(&m_XmlH);
		XMLLoad(strFile);
		eg_bool bSuccess=(m_XmlH.nVer==EGMeshBase::MESH_VERSION);
		if(bSuccess)
		{
			GenerateTangents();

			// Check some things...
			const eg_uint NumMtrls = static_cast<eg_uint>(m_MtrlsArray.Len());
			for( eg_size_t i=0; i<m_SubMeshesArray.Len(); i++ )
			{
				m_SubMeshesArray[i].nMtrIndex = EG_Clamp<eg_int>(m_SubMeshesArray[i].nMtrIndex, -1, NumMtrls-1);
			}

			BuildData();
		}
		return bSuccess;
	}

	void Unload()
	{
		DeallocateMemory();
		zero(&m_XmlH);
	}
private:
	void GenerateTangents()
	{
		if( !m_bFoundTangent && m_VertexesArray.Len() > 0 )
		{
			EGLogf( eg_log_t::Warning , "%s\" Warning: No tangents found, generating them..." , GetXmlFilename() );
			EGVertex_ComputeTangents(m_VertexesArray.GetArray(), m_IndexesArray.GetArray(), static_cast<eg_uint>(m_VertexesArray.Len()) , static_cast<eg_uint>(m_IndexesArray.Len()/3));
		}
	}
/*****************************
*** XML Loading functions: ***
*****************************/
private:
	void OnTag_textnode( const EGXmlAttrGetter& Getter )
	{
		egTextNodeTagInfo TagInfo = EGTextNodeTag_FromXmlTag( Getter );

		m_TextNodesArray.ExtendToAtLeast( m_XmlLastTextNode + 1 );
		EGMeshBase::egTextNode* TextNode = &m_TextNodesArray[m_XmlLastTextNode];
		m_XmlLastTextNode++;
		EGCrcDb::AddAndSaveIfInTool(TagInfo.Id);
		TextNode->Id = eg_string_crc(TagInfo.Id);
		TextNode->Bone = TagInfo.Bone;
		TextNode->Text = eg_string_crc(TagInfo.LocText);
		TextNode->Font = eg_string_crc(TagInfo.Font);
		TextNode->Width = TagInfo.NodeWidth;
		TextNode->Height = TagInfo.NodeHeight;
		TextNode->LineHeight = TagInfo.LineHeight;
		TextNode->Color = eg_color32(TagInfo.Color);
		TextNode->Justify = TagInfo.Justify;
		
		//Get the position (may be overridden by position tag)
		{
			eg_string_big StrPos = Getter.GetString( "position" , "0 0 0" );
			eg_real Pos[3];
			EGString_GetFloatList( StrPos , StrPos.Len() , &Pos , countof(Pos) );
			TextNode->Pose = eg_transform::BuildTranslation( eg_vec3(Pos[0] , Pos[1] , Pos[2]) );
		}
	}

	void OnTag_position(  const EGXmlAttrGetter& Getter  )
	{
		//This will be gotten in data.
		m_XmlPositionType = eg_crc("MATRIX");
		eg_string_big TypeStr = Getter.GetString( "type" , "MATRIX" );
		TypeStr.ConvertToUpper();
		m_XmlPositionType = eg_string_crc(TypeStr); 
	}

	virtual void OnData( eg_cpstr Data , eg_uint Len )
	{
		const eg_string_big Tag = GetXmlTagUp(0);
		const eg_string_big ParentTag = (GetXmlTagLevels() <= 1) ? "" : GetXmlTagUp( 1 );

		if( "position" == Tag )
		{
			eg_transform* pTrans = nullptr;

			if( "textnode" == ParentTag )
			{
				if( m_XmlLastTextNode > m_TextNodesArray.Len() || m_XmlLastTextNode < 1 )
				{ 
					EGLogf( eg_log_t::Error , __FUNCTION__" Error: Text somehow not added?" );
					return; 
				} 
				EGMeshBase::egTextNode* TextNode = &m_TextNodesArray[m_XmlLastTextNode-1];
				pTrans = &TextNode->Pose;
			}

			assert_pointer( pTrans ); //Should not be possible to have gotten here without this.

			if( eg_crc("MATRIX") == m_XmlPositionType )
			{
				assert( false ); // Not supported
			}
			else if( eg_crc("QUAT_TRANS") == m_XmlPositionType )
			{
				EGString_GetFloatList( Data , Len , pTrans , 7 , false );
				pTrans->SetScale( 1.f );
			}
			else if( eg_crc("ROTXYZ_TRANSXYZ") == m_XmlPositionType )
			{
				struct egTransform
				{
					eg_vec3 Rot;
					eg_vec3 Trans;
				} Trans;

				assert( sizeof(egTransform) == sizeof( eg_real)*6 );
				EGString_GetFloatList( Data , Len , &Trans , 6 );
				*pTrans = eg_transform::BuildIdentity();
				pTrans->RotateXThis( EG_Rad(Trans.Rot.x) );
				pTrans->RotateYThis( EG_Rad(Trans.Rot.y) );
				pTrans->RotateZThis( EG_Rad(Trans.Rot.z) );
				pTrans->TranslateThis( Trans.Trans.x , Trans.Trans.y , Trans.Trans.z );
			}
			else
			{
				EGLogf( eg_log_t::Warning , __FUNCTION__" Warning: Unknown transform type." );
			}
		}
	}

	virtual void OnTag(const eg_string_base& Tag, const EGXmlAttrGetter& Getter) override final
	{
		if(m_XmlH.nVer!=EGMeshBase::MESH_VERSION && "emesh"!=Tag)
			return;
		
		if("emesh"==Tag)
		{
			zero( &m_XmlH );
			m_XmlH.nID  = EGMeshBase::MESH_ID;
			m_bFoundTangent = false;
			m_NextStrOfs=0;

			#define MESH_DATA_SECTION( _type_ , _var_ ) m_##_var_##Array.Clear();
			#include "EGMeshDataSections.inc"
			#undef MESH_DATA_SECTION
			m_HArray.Resize(1);
		
			eg_string_big strFormat     = Getter.GetString( "format" );
			m_XmlH.nVer             = Getter.GetUInt( "version" );

			if(m_XmlH.nVer != EGMeshBase::MESH_VERSION)
			{
				EGLogf( eg_log_t::Warning , __FUNCTION__ " Warning: It may have been a while since this mesh was exported, not all data may be present." );
				m_XmlH.nVer = EGMeshBase::MESH_VERSION;
			}

			if(EGMeshBase::MESH_VERSION == m_XmlH.nVer)
			{
				// It's good...
			}
				
			m_bUseBase64Floats = strFormat.EqualsI("base64");
		}
		else if("mesh"==Tag)
		{
			EGMeshBase::egSubMesh mesh;
			eg_uint nID=Getter.GetUInt( "id" );
			eg_string_big strName = Getter.GetString( "name" );
			EGCrcDb::AddAndSaveIfInTool( strName );
		
			mesh.nFirstIndex = Getter.GetUInt( "first_triangle" );
			mesh.nNumTri     = Getter.GetUInt( "triangles" );
			mesh.nMtrIndex   = Getter.GetUInt( "material" );
		
			//Adjust the indexes for the internal format:
			//Material index -1, and the indexes must be
			//adjusted from triangle space to index space.
			mesh.nMtrIndex--;
			mesh.nFirstIndex=(mesh.nFirstIndex-1)*3;
		
			if( nID > 0 )
			{
				mesh.nStrOfsName = AddString(strName);
				m_SubMeshesArray.ExtendToAtLeast( nID );
				m_SubMeshesArray[nID-1]=mesh;
			}
		}
		else if("material"==Tag)
		{
			EGMaterialDef MtrlDef;
			MtrlDef.SetFromTag(Getter);
			//We don't set relative pathnames because we don't want them exported.

			eg_uint nID = Getter.GetUInt( "id" );
		
			if( nID > 0 )
			{
				m_MtrlsArray.ExtendToAtLeast( nID );
				m_MtrlsArray[nID-1].Def = MtrlDef;
			}
		}
		else if("bone"==Tag)
		{
			eg_uint nID=Getter.GetUInt( "id" );
			eg_string_big strName=Getter.GetString( "name" );
			EGCrcDb::AddAndSaveIfInTool( strName );
		
			if( nID > 0 )
			{
				m_BonesArray.ExtendToAtLeast( nID );
				m_BonesArray[nID-1].BoneNameOfs = AddString(strName);
				m_BonesArray[nID-1].SkelRef = 0;
			}
		}
		else if("triangle"==Tag)
		{
			eg_uint nID=0;
			eg_uint nV[3]={0, 0, 0};
		
			nID   = Getter.GetUInt( "id" );
			nV[0] = Getter.GetUInt( "v1" );
			nV[1] = Getter.GetUInt( "v2" );
			nV[2] = Getter.GetUInt( "v3" );
		
			if( nID > 0 )
			{
				//Have to subtract index by 1 for internal structure.
				m_IndexesArray.ExtendToAtLeast( (nID-1)*3+2 + 1 );
				m_IndexesArray[(nID-1)*3+0]=static_cast<egv_index>(nV[0]-1);
				m_IndexesArray[(nID-1)*3+1]=static_cast<egv_index>(nV[1]-1);
				m_IndexesArray[(nID-1)*3+2]=static_cast<egv_index>(nV[2]-1);
			}
		}
		else if("vertex"==Tag)
		{
			egv_vert_mesh v;
			zero(&v);

			eg_bool FoundTangent=false;
			eg_uint nID = ::EGVertex_FromXmlTag(&v, m_bUseBase64Floats, Getter, &FoundTangent);
			m_bFoundTangent = FoundTangent;

			if( nID > 0 )
			{
				m_VertexesArray.ExtendToAtLeast( nID );
				m_VertexesArray[nID-1]=v;
			}
		}
		else if("bounds"==Tag)
		{
			Getter.GetVec( "min" , reinterpret_cast<eg_real*>(&m_XmlH.AABB.Min) , 3 , m_bUseBase64Floats );
			m_XmlH.AABB.Min.w = 1.0f;
			Getter.GetVec( "max" , reinterpret_cast<eg_real*>(&m_XmlH.AABB.Max) , 3 , m_bUseBase64Floats );
			m_XmlH.AABB.Max.w = 1.0f;
		}
		else if( "textnode" == Tag )
		{
			OnTag_textnode( Getter );
		}
		else if( "position" == Tag )
		{
			OnTag_position( Getter );
		}
		else
		{
			assert( false );
			EGLogf( eg_log_t::Error , "%s.OnTag: An invalid tag was specified." ,XMLObjName() );
		}
	}
	virtual eg_cpstr XMLObjName()const
	{
		return ("\"Emergence Mesh\"");
	}

	void DeallocateMemory()
	{
		if( m_pMem )
		{
			EG_SafeDeleteArray( m_pMem );
			m_pMem = nullptr;
		}
	}

	eg_uint AddString( const eg_string_base& s )
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

	void BuildData()
	{
		m_HArray.ExtendToAtLeast( 1 );
		m_HArray[0] = m_XmlH; // Make sure the header exists.

		// Set counts...
		#define MESH_DATA_SECTION( _type_ , _var_ ) m_XmlH.n##_var_##Count = static_cast<eg_uint>(m_##_var_##Array.Len());
		#include "EGMeshDataSections.inc"
		#undef MESH_DATA_SECTION

		// Set offsets...
		eg_size_t DataSize = 0;
		#define MESH_DATA_SECTION( _type_ , _var_ ) m_XmlH.Ofs##_var_## = static_cast<eg_uint>(DataSize); DataSize += EG_AlignUp( m_XmlH.n##_var_##Count * sizeof( _type_ ) );
		#include "EGMeshDataSections.inc"
		#undef MESH_DATA_SECTION

		// Copy data to a chunk...
		m_pMem = new eg_byte[DataSize];
		if( nullptr != m_pMem )
		{
			m_HArray[0] = m_XmlH; // Set the header again with the correct information.
			#define MESH_DATA_SECTION( _type_ , _var_ ) if( m_XmlH.n##_var_##Count > 0 ){ EGMem_Copy( &m_pMem[m_XmlH.Ofs##_var_##] , &m_##_var_##Array[0] , m_XmlH.n##_var_##Count*sizeof(_type_)); }
			#include "EGMeshDataSections.inc"
			#undef MESH_DATA_SECTION
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

eg_bool EGMakeRes_emesh()
{
	EGMeshCompiler Mesh;

	if(!Mesh.LoadFromXML(EGMake_GetInputPath()))
	{
		return false;
	}

	eg_size_t nSize = 0;
	const eg_byte* pData = Mesh.GetDataAndSize(&nSize);

	eg_bool bSuc = ::EGMake_WriteOutputFile( ::EGMake_GetOutputPath(), pData, nSize);

	return bSuc;
}