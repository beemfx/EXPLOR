// (c) 2011 Beem Media

#include "EGMeshBase.h"
#include "EGMeshState.h"
#include "EGSkelBase.h"

EGMeshBase::EGMeshBase()
{

}

EGMeshBase::~EGMeshBase()
{
	assert( nullptr == m_pMem ); //Mesh was never unloaded in parent class?
}

eg_bool EGMeshBase::IsLoaded()const
{
	return m_H.nID == MESH_ID && m_H.nVer == MESH_VERSION;
}

void EGMeshBase::SetupFrame( class EGMeshState& NodeState ) const
{
	if(!NodeState.m_DefaultBonesSet)
	{
		NodeState.m_Bones.Resize(m_H.nBonesCount+1);
		NodeState.m_AttachBones.Resize(m_H.nBonesCount+1);
		NodeState.m_Bones[0] = eg_transform::BuildIdentity();

		for(eg_uint i=0; i<m_H.nBonesCount; i++)
		{
			NodeState.m_Bones[i+1] = eg_transform::BuildIdentity();
			NodeState.m_AttachBones[i] = m_pBones[i].SkelBasePose;
		}
	}
	NodeState.m_DefaultBonesSet = true;
}


/**********************************************************************
*** Technically the prepare frame functions should
*** get a compatible skeleton, not all skeleton's are compatible
*** with every model so in the future there will be a method to
*** check compatibility, but this should not be called every frame
*** as it would only use unnecessary processor power, instead
*** it is up to the the programmer to make sure a skeleton is
*** compatible before using it for rendering, as the function stands
*** right now the PrepareFrame functions simply try to prepare a
*** frame based off joint references
***********************************************************************/
void EGMeshBase::SetupFrame( class EGMeshState& NodeState , eg_uint dwFrame1, eg_uint dwFrame2, eg_real t, EGSkelBase* pSkel) const
{
	if(!pSkel || !pSkel->IsLoaded())
	{
		SetupFrame( NodeState );
		return;
	}

	NodeState.m_Bones.Resize( m_H.nBonesCount+1 );
	NodeState.m_AttachBones.Resize( m_H.nBonesCount+1 );
	NodeState.m_Bones[0] = eg_transform::BuildIdentity();
	eg_transform* pJointTrans=&NodeState.m_Bones[1];
	pSkel->ComputeFrameBones(dwFrame1, dwFrame2, t, pJointTrans);

	//Prepare each joints final transformation matrix.
	for(eg_uint i=0; i<m_H.nBonesCount; i++)
	{
		NodeState.m_AttachBones[i] = (*pSkel->GetBaseTransform(m_pBones[i].SkelRef)) * pJointTrans[i];
	}

	NodeState.m_DefaultBonesSet = false;
}

void EGMeshBase::SetupFrame( class EGMeshState& NodeState , eg_uint nFrame1_1, eg_uint nFrame2_1, eg_real fTime1, EGSkelBase* pSkel1, eg_uint nFrame_2, eg_real fTime2, EGSkelBase* pSkel2) const
{
	if(!pSkel1 || !pSkel1->IsLoaded() || !pSkel2 || !pSkel2->IsLoaded())
	{
		SetupFrame( NodeState );
		return;
	}

	NodeState.m_Bones.Resize( m_H.nBonesCount+1 );
	NodeState.m_AttachBones.Resize( m_H.nBonesCount+1 );
	NodeState.m_Bones[0] = eg_transform::BuildIdentity();
	eg_transform* pJointTrans=&NodeState.m_Bones[1];
	pSkel1->ComputeFrameBones(nFrame1_1, nFrame2_1, fTime1, pSkel2, nFrame_2, fTime2, pJointTrans);

	//Prepare each joints final transformation matrix.
	for(eg_uint i=0; i<m_H.nBonesCount; i++)
	{
		NodeState.m_AttachBones[i] = (*pSkel1->GetBaseTransform(m_pBones[i].SkelRef)) * pJointTrans[i];
	}

	NodeState.m_DefaultBonesSet = false;
}

void EGMeshBase::SetupFrame( class EGMeshState& NodeState , eg_string_crc nAnim1, eg_real fTime1, EGSkelBase* pSkel1, eg_string_crc nAnim2, eg_real fTime2, EGSkelBase* pSkel2) const
{
	if(!pSkel1 || !pSkel1->IsLoaded() || !pSkel2 || !pSkel2->IsLoaded())
	{
		SetupFrame( NodeState );
		return;
	}

	eg_uint nFrame1_1, nFrame2_1, nFrame_2;
	nFrame1_1=pSkel1->GetFrameFromTime(nAnim1, fTime1, &fTime1, &nFrame2_1);
	const EGSkelBase::egAnim* pAnim = pSkel2->GetAnimByCrc(nAnim2);
	if( nullptr != pAnim )
	{
		nFrame_2=pAnim->nFirstFrame;
	}
	//SetupFrame(nFrame1_1, nFrame2_1, fTime1, pSkel1);
	SetupFrame( NodeState , nFrame1_1, nFrame2_1, fTime1, pSkel1, nFrame_2, fTime2, pSkel2);
}

void EGMeshBase::SetupCustomBone( class EGMeshState& NodeState , eg_string_crc BoneId , const eg_transform& Transform ) const
{
	eg_uint BoneRef = GetJointRef( BoneId );
	if( INVALID_JOINT != BoneRef )
	{
		NodeState.m_Bones[BoneRef+1] *= Transform;
	}
	else
	{
		assert( false ); // Trying to set a custom bone on a joint that doesn't exist.
	}

	NodeState.m_DefaultBonesSet = false;
}

eg_uint EGMeshBase::GetJointRef(eg_string_crc CrcId)const
{
	for(eg_uint i=0; i<m_H.nBonesCount; i++)
	{
		if(CrcId == eg_string_crc(GetBoneName(i)))
		{
			return i;
		}
	}
	return INVALID_JOINT;
}

eg_uint EGMeshBase::GetJointRef(eg_cpstr szJoint)const
{
	for(eg_uint i=0; i<m_H.nBonesCount; i++)
	{
		if(!eg_string(GetBoneName(i)).CompareI(szJoint))
			return i;
	}
	return INVALID_JOINT;
}


const eg_transform* EGMeshBase::GetJointTransform( class EGMeshState& NodeState , eg_uint n) const
{
	assert(0 < n && n <= m_H.nBonesCount);
	return &NodeState.m_Bones[n];
}

const eg_transform* EGMeshBase::GetJointAttachTransform( class EGMeshState& NodeState , eg_uint n) const
{
	assert(0 <= n && n < m_H.nBonesCount);
	return &NodeState.m_AttachBones[n];
}

eg_cpstr EGMeshBase::GetSubMeshName(eg_uint n)const
{
	assert(0 <= n && n < m_H.nSubMeshesCount);
	return &m_pStrings[m_pSubMeshes[n].nStrOfsName];
}

const EGMaterialDef* EGMeshBase::GetSubMeshMaterialDef( eg_uint SubMeshIdx ) const
{
	eg_uint MtrlIndex = m_pSubMeshes[SubMeshIdx].nMtrIndex;
	return MtrlIndex != -1 ? &m_pMtrls[MtrlIndex].Def : nullptr;
}

eg_cpstr EGMeshBase::GetBoneName(eg_uint n)const
{
	assert(0 <= n && n < m_H.nBonesCount);
	return &m_pStrings[m_pBones[n].BoneNameOfs];
}

//This verison of prepare frames takes an animation and
//it wants a value from 0.0f to 1.0f.  0.0f would be
//the first frame of the animation, and 1.0f would also
//be the first frame of the animation.
void EGMeshBase::SetupFrame( class EGMeshState& NodeState , eg_string_crc nAnim, eg_real fTime, EGSkelBase* pSkel) const
{
	if(!pSkel || !pSkel->IsLoaded())
	{
		return;
	}
	eg_uint nFrame1, nFrame2;
	nFrame1=pSkel->GetFrameFromTime(nAnim, fTime, &fTime, &nFrame2);
	SetupFrame( NodeState , nFrame1, nFrame2, fTime, pSkel);
}

void EGMeshBase::MakeCompatibleWith( const EGSkelBase* pSkel )
{
	//If the skeleton isn't loaded yet, don't make compatible.
	if(!IsLoaded() || !pSkel || !pSkel->IsLoaded())
	{
		assert(false);
		return;
	}

	for(eg_uint i=0; i<m_H.nBonesCount; i++)
	{
		for(eg_uint j=0; j<pSkel->GetNumJoints(); j++)
		{
			if(eg_string(GetBoneName(i)).EqualsI(pSkel->GetBoneName(j)))
			{
				m_pBones[i].SkelRef=j;
				m_pBones[i].SkelBasePose = *pSkel->GetBaseTransform(j);
				j=pSkel->GetNumJoints()+1;
			}
		}
	}
}

void EGMeshBase::SetPointers(const egHeader& Header)
{
	assert( nullptr != m_pMem );
	//Create a copy just in case we are passing m_pH to this:
	egHeader H = Header;

	#define MESH_DATA_SECTION( _type_ , _var_ ) m_p##_var_ = reinterpret_cast<_type_*>( &m_pMem[H.Ofs##_var_] );
	#include "EGMeshDataSections.inc"
	#undef MESH_DATA_SECTION

	//Do some checking
	#define MESH_DATA_SECTION( _type_ , _var_ ) assert_aligned( m_p##_var_ );
	#include "EGMeshDataSections.inc"
	#undef MESH_DATA_SECTION

	m_H = H;
}

void EGMeshBase::InitPrivateData(eg_cpstr strFile)
{
	//Make material paths relative:
	for(eg_uint i=0; i<m_pH->nMtrlsCount; i++)
	{
		m_pMtrls[i].Def.MakePathsRelativeTo(strFile);
	}

	// Initialize default animation frame (no usage of skeleton).

	for(eg_uint i=0; i<m_pH->nBonesCount; i++)
	{
		m_pBones[i].SkelRef=i;
	}
}