// (c) 2011 Beem Media

#include "EGSkelBase.h"

/******************************************************************************
Notes On Animation
------------------

To explain the calculation, let L1, L2, ..., Ln be the frame
matrices for a joint for each parent with L1 being the local transform
for the current joing, L2 being the local transform for the parent,
L3 for the grandparent, and so forth.

Let B1, B2, ..., Bn be the local base position matrices for each
successive parent. With B1 being the local position for the selected
joint. The local position is relative to the joint's parent.

The final transformation is given by
(B1*B2*...*Bn)^{-1}*(L1*B1*L2*B2*...*Ln*Bn)

The idea behind that, is that the vertex starts relative to the position of
the base joint in world space, and we need it in the joint's local space,
so we multiply by the inverse of the collection of transformations 
that lead us to joint's base position. (Essentially
we are subtracting the actual position of the bone from the vertex.) This
constitutes the (B1*B2*...Bn)^{-1} (which is calculated at the start of this
method.) This places the vertex in the joint's local space.

Then we perform the joint's local transform, L1, this will move and rotate
the vertex according to the local transform. Then we transform the vertex
according to the base skeleton's local transform for that joint. The base
skeleton local transform describes the position of this joint relative to
it's parent, so essentially what we are doing is transforming the vertex
into the local space of it's parent. These two steps are the L1*B1
transformation.

At this point the vertex is now in the bone's parent's local space, so we
can now treat it as if it belongs to the parent's bone and thus we multiply
by L2*B2. When we reach the topmost bone (which has no parent), we are done.
******************************************************************************/

EGSkelBase::EGSkelBase()
{
	
}

EGSkelBase::~EGSkelBase()
{
	assert( nullptr == m_pMem );
}

eg_uint EGSkelBase::GetNumAnims()const
{
	return m_H.nAnimsCount;
}

eg_uint EGSkelBase::GetNumJoints()const
{
	return m_H.nBaseSkelJointsCount;
}
eg_uint EGSkelBase::GetNumKeyFrames()const
{
	return m_H.nFramesCount;
}
eg_bool EGSkelBase::IsLoaded()const
{
	return m_H.nID == SKEL_ID && m_H.nVersion == SKEL_VERSION;
}
eg_uint EGSkelBase::GetParentBoneRef(eg_uint nBone)const
{
	return m_pBaseSkelJoints[nBone].nParent;
}

const EGSkelBase::egAnim* EGSkelBase::GetAnimByCrc( eg_string_crc Crc )const
{
	for( eg_uint i=0; i<m_H.nAnimsCount; i++ )
	{
		if( Crc == m_pAnims[i].NameCrc )
		{
			return &m_pAnims[i];
		}
	}
#if defined( __DEBUG__ )
	static eg_bool ASSERTED = false;
	if( !ASSERTED )
	{
		ASSERTED=true;
		assert( false ); //Anim not found.
	}
#endif
	return &m_pAnims[0];
}

eg_uint EGSkelBase::GetFrameFromTime(eg_string_crc AnimationId, eg_real fTime, eg_real* pFrameTime, eg_uint* pFrame2)const
{
	const EGSkelBase::egAnim* Anim = GetAnimByCrc( AnimationId );
	switch( Anim->LoopMode )
	{
	case egAnim::LOOP_ONCE     : return GetFrameFromTime_Once( AnimationId , fTime , pFrameTime , pFrame2 );
	case egAnim::LOOP_LOOPING  : return GetFrameFromTime_Looping( AnimationId , fTime , pFrameTime , pFrame2 );
	case egAnim::LOOP_LOOP_BACK: return GetFrameFromTime_LoopBack( AnimationId , fTime , pFrameTime , pFrame2 );
	}

	assert( false ); //Invalid anim looping mode.
	return GetFrameFromTime_Looping( AnimationId , fTime , pFrameTime , pFrame2 );
}

eg_uint EGSkelBase::GetFrameFromTime_LoopBack(eg_string_crc AnimationId, eg_real fTime, eg_real* pFrameTime, eg_uint* pFrame2)const
{
	assert( 0 <= fTime && fTime <= 1.f );
	assert_pointer(pFrameTime);
	assert_pointer(pFrame2);

	const EGSkelBase::egAnim* Anim = GetAnimByCrc( AnimationId );

	eg_uint LastFrame = Anim->nFirstFrame+Anim->nNumFrames-1;
	eg_real Time;
	if( fTime <.5f )
	{
		Time = EGMath_GetMappedRangeValue( fTime , eg_vec2(0.f,.5f) , eg_vec2(static_cast<eg_real>(Anim->nFirstFrame),static_cast<eg_real>(LastFrame)) );
	}
	else
	{
		Time = EGMath_GetMappedRangeValue( 1.f - fTime , eg_vec2(0.f,.5f) , eg_vec2(static_cast<eg_real>(Anim->nFirstFrame),static_cast<eg_real>(LastFrame)) );
	}

	eg_uint Frame1 = EGMath_floor(Time);
	*pFrameTime = Time - Frame1;
	*pFrame2 = Frame1+1;
	if( *pFrame2 > LastFrame ) //If the 2nd frame is at the end of the animation we hit the end of it (this should only happen at fTime=1.f or if the animation is only 1 frame).
	{
		*pFrame2 = Frame1;
		*pFrameTime = 1.f;
	}
	//DebugText_MsgOverlay_AddText( DEBUG_TEXT_OVERLAY_CLIENT , EGString_Format( "Frame Time %u->%u %f (%u,%u)" , Frame1 , *pFrame2 , *pFrameTime , Anim->nFirstFrame , LastFrame ).String() );
	return Frame1;
}

eg_uint EGSkelBase::GetFrameFromTime_Looping(eg_string_crc AnimationId, float fTime, float* pFrameTime, eg_uint* pFrame2)const
{	
	assert( 0 <= fTime && fTime <= 1.f );
	assert_pointer(pFrameTime);
	assert_pointer(pFrame2);

	const EGSkelBase::egAnim* Anim = GetAnimByCrc( AnimationId );

	eg_uint LastFrame = Anim->nFirstFrame+Anim->nNumFrames-1;
	eg_real Time = EGMath_GetMappedRangeValue( fTime , eg_vec2(0.f,1.f) , eg_vec2(static_cast<eg_real>(Anim->nFirstFrame),static_cast<eg_real>(LastFrame+1)) ); //Add an extra frame since we treat the last frame loops back around to the first frame.

	eg_uint Frame1 = EGMath_floor(Time);
	*pFrameTime = Time - Frame1;
	*pFrame2 = Frame1+1;
	if( *pFrame2 > LastFrame ) //If the 2nd frame is at the end of the animation we hit the end of it, so loop to the first frame of the animation.
	{
		*pFrame2 = Anim->nFirstFrame;
	}
	//DebugText_MsgOverlay_AddText( DEBUG_TEXT_OVERLAY_CLIENT , EGString_Format( "Frame Time %u->%u %f (%u,%u)" , Frame1 , *pFrame2 , *pFrameTime , Anim->nFirstFrame , LastFrame ).String() );
	return Frame1;
}

eg_uint EGSkelBase::GetFrameFromTime_Once(eg_string_crc AnimationId, eg_real fTime, eg_real* pFrameTime, eg_uint* pFrame2)const
{
	assert( 0 <= fTime && fTime <= 1.f );
	assert_pointer(pFrameTime);
	assert_pointer(pFrame2);

	const EGSkelBase::egAnim* Anim = GetAnimByCrc( AnimationId );

	eg_uint LastFrame = Anim->nFirstFrame+Anim->nNumFrames-1;
	eg_real Time = EGMath_GetMappedRangeValue( fTime , eg_vec2(0.f,1.f) , eg_vec2(static_cast<eg_real>(Anim->nFirstFrame),static_cast<eg_real>(LastFrame)) );

	eg_uint Frame1 = EGMath_floor(Time);
	*pFrameTime = Time - Frame1;
	*pFrame2 = Frame1+1;
	if( *pFrame2 > LastFrame ) //If the 2nd frame is at the end of the animation we hit the end of it (this should only happen at fTime=1.f or if the animation is only 1 frame).
	{
		*pFrame2 = Frame1;
		*pFrameTime = 1.f;
	}
	//DebugText_MsgOverlay_AddText( DEBUG_TEXT_OVERLAY_CLIENT , EGString_Format( "Frame Time %u->%u %f (%u,%u)" , Frame1 , *pFrame2 , *pFrameTime , Anim->nFirstFrame , LastFrame ).String() );
	return Frame1;
}

const eg_transform* EGSkelBase::GetBaseTransform(eg_uint nJoint) const
{
	nJoint=EG_Clamp<eg_uint>(nJoint, 0, m_H.nBaseSkelJointsCount-1);

	return &m_pBaseSkelJoints[nJoint].matFinal;
}

eg_cpstr EGSkelBase::GetBoneName(eg_uint nBone)const
{
	assert(0 <= nBone && nBone < m_H.nBaseSkelJointsCount);

	return &m_pStrings[m_pBaseSkelJoints[nBone].nStrOfsName];
}

eg_uint EGSkelBase::GetNumFrames()const
{
	return m_H.nFramesCount;
}


eg_transform* EGSkelBase::GetLocalTransform(eg_uint nBone, eg_uint nFrame)
{
	return &m_pFrames[(nFrame-1)*m_H.nBaseSkelJointsCount + (nBone-1)];
}

void EGSkelBase::ComputeFrameBones( eg_uint nFrame1 , eg_uint nFrame2 , eg_real t , eg_transform* aTransBonesTarget )
{
	assert(1 <= nFrame1 && nFrame1 <= m_H.nFramesCount);
	assert(1 <= nFrame2 && nFrame2 <= m_H.nFramesCount);
	assert(0 <= t && t <= 1.0f);

	assert(m_H.nBaseSkelJointsCount < MAX_BONES );

	//Let us slerp all the local transforms:
	eg_transform aL[MAX_BONES];

	for(eg_uint i=0; i<m_H.nBaseSkelJointsCount; i++)
	{
		const eg_transform* ptr1 = GetLocalTransform(i+1, nFrame1);
		const eg_transform* ptr2 = GetLocalTransform(i+1, nFrame2);

		eg_transform trT = eg_transform::Lerp( *ptr1 , *ptr2 , t );

		//We now compute all the matrices necessary.
		aL[i] = trT;
	}

	ComputeFrameBones_Algorithm( aL , aTransBonesTarget );
}

void EGSkelBase::ComputeFrameBones( eg_uint nFrame1_1 , eg_uint nFrame2_1 , eg_real t1 , EGSkelBase* pSkel2 , eg_uint nFrame_2 , eg_real t2 , eg_transform* aTransBonesTarget )
{
	assert(1 <= nFrame1_1 && nFrame1_1 <= m_H.nFramesCount);
	assert(1 <= nFrame2_1 && nFrame2_1 <= m_H.nFramesCount);
	assert(0 <= t1 && t1 <= 1.0f);

	assert(1 <= nFrame_2 && nFrame_2 <= m_H.nFramesCount);
	assert(0 <= t2 && t2 <= 1.0f);

	assert(m_H.nBaseSkelJointsCount < MAX_BONES );

	//Let us slerp all the local transforms:
	eg_transform aL[MAX_BONES];

	for(eg_uint i=0; i<m_H.nBaseSkelJointsCount; i++)
	{
		//First compute the transform for the first animation:
		const eg_transform* ptr1 =  GetLocalTransform(i+1, nFrame1_1);
		const eg_transform* ptr2 =  GetLocalTransform(i+1, nFrame2_1);

		eg_transform trT1 = eg_transform::Lerp( *ptr1 , *ptr2 , t1 ); 

		//Now compute the transform for the second animation:
		ptr1 = pSkel2->GetLocalTransform(i+1, nFrame_2);
		//And lerp to it.
		trT1 = eg_transform::Lerp( trT1 , *ptr1 , t2 );

		//We now compute all the matrices necessary.
		aL[i] = trT1;
	}

	ComputeFrameBones_Algorithm( aL , aTransBonesTarget );
}

void EG_INLINE EGSkelBase::ComputeFrameBones_Algorithm( const eg_transform*const aL , eg_transform* aTransBonesTarget )
{
	//#define DONT_ASSUME_PARENTS_LOWER

	//For the dynamic programming algorithm we keep track of which joints we've
	//already computed. That way we don't do the same multiplications over and
	//over. Technically for best results the skeleton should be organized so
	//that all children have a greater reference than their parents.
#if defined(DONT_ASSUME_PARENTS_LOWER)
	eg_uint32 nBoneFound = 0;
	assert(MAX_BONES <= sizeof(nBoneFound)*8);
#endif

	//In the prefered algorithm, a bone's children always come after it, so all
	//the L2*B2*L3*B3*...*Ln*Bn will already be computed, so we merely need to
	//pre multiply L1*B1.

	for(eg_uint i=0; i<m_H.nBaseSkelJointsCount; i++)
	{
		//(B1*B2*...*Bn)^{-1}*(L1*B1*L2*B2*...*Ln*Bn)
		//Let BInv = (B1*B2*...*Bn)^{-1} (we already have this in m_pBaseSkelJoints[i].matFinalInv)
		//Let C = L1*B1*L2*B2*...*Ln*Bn

		//1. Obtain the base joint for that joint.
		const egJoint* pTemp=&m_pBaseSkelJoints[i];
		assert(pTemp->nJointRef == i);

		eg_transform C = aL[i] * m_pBaseSkelJoints[i].matLocal;

#if !defined(DONT_ASSUME_PARENTS_LOWER)
		if(pTemp->nParent)
		{
			assert((pTemp->nParent-1) < i);
			C *= aTransBonesTarget[pTemp->nParent-1];
		}
#else
		//We insure that we never get into an infinite loop, the max depth is
		//no more than the total number of bones
		for
			(
				eg_uint limit=0; 
				(pTemp->nParent != 0) && (limit < MAX_BONES) && (nBoneFound&(1<<i))==0; 
				limit++
				)
		{
			eg_uint nParentJoint = pTemp->nParent-1;
			if((nBoneFound&(1<<nParentJoint)) != 0)
			{
				C = C*aTransBonesTarget[nParentJoint];
				break;
			}
			else
			{
				pTemp=&m_pBaseSkelJoints[pTemp->nParent-1];
				C = C*aL[pTemp->nJointRef]*m_pBaseSkelJoints[pTemp->nJointRef].matLocal;
			}
		}

		nBoneFound |= (1<<i);
#endif
		aTransBonesTarget[i] = C;
	}

	//Once all joint transforms are computed, we must go ahead and pre-multiply
	//by the inverse transform:
	for(eg_uint i=0; i<m_H.nBaseSkelJointsCount; i++)
	{
		aTransBonesTarget[i] = m_pBaseSkelJoints[i].matFinalInv*aTransBonesTarget[i];
	}

#undef DONT_ASSUME_PARENTS_LOWER
}

void EGSkelBase::SetPointers(const egHeader& Header)
{
	assert( nullptr != m_pMem );
	//Create a copy just in case we are passing m_pH to this:
	egHeader H = Header;

	#define SKEL_DATA_SECTION( _type_ , _var_ ) m_p##_var_ = reinterpret_cast<_type_*>( &m_pMem[H.Ofs##_var_] );
	#include "EGSkelDataSections.inc"
	#undef SKEL_DATA_SECTION

	//Do some checking
	#define SKEL_DATA_SECTION( _type_ , _var_ ) assert_aligned( m_p##_var_ );
	#include "EGSkelDataSections.inc"
	#undef SKEL_DATA_SECTION

	m_H = H;
}

