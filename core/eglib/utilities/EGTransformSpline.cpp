// (c) 2020 Beem Media. All Rights Reserved.

#include "EGTransformSpline.h"

void EGTransformSpline::SetSplineNodes( const EGArray<eg_transform>& In )
{
	m_SplineNodes.Resize( In.Len() );

	if( m_SplineNodes.Len() == In.Len() )
	{
		const eg_size_t SplineLen = m_SplineNodes.Len();
		for( eg_size_t i=0; i<SplineLen; i++ )
		{
			m_SplineNodes[i].Pose = In[i];
			m_SplineNodes[i].bIsSet = true;
		}
	}
}

void EGTransformSpline::InsertSplineNode( eg_int InIndex , const eg_transform& InPose )
{
	m_SplineNodes.ExtendToAtLeast( InIndex + 1 );
	if( m_SplineNodes.IsValidIndex( InIndex ) )
	{
		m_SplineNodes[InIndex].Pose = InPose;
		m_SplineNodes[InIndex].bIsSet = true;
	}
}

void EGTransformSpline::RemoveSplineNode( eg_int InIndex )
{
	if( m_SplineNodes.IsValidIndex( InIndex ) )
	{
		m_SplineNodes[InIndex].bIsSet = false;
	}
}

void EGTransformSpline::BeginSplineInternal( const eg_transform& InitialPose )
{
	m_SegStartPoseC = InitialPose;
	m_SegStartPose = InitialPose;
	m_CurrentPose = InitialPose;
	m_TargetPoseIdx = 0;
	m_bIsPlaying = true;

	eg_int StartingIndex = FindValidNodeStartingAtIndex( 0 );
	if( m_SplineType == ex_transform_spline_t::CatmullRom )
	{
		// If doing catmull rom we may need to adjust the starting node to the next one if
		// we are jumping to it so that the first bit of animation is consistent.
		if( m_SplineNodes.IsValidIndex( StartingIndex ) )
		{
			assert( m_SplineNodes[StartingIndex].bIsSet );
			const eg_transform StartingPose = m_SplineNodes[StartingIndex].Pose;
			if( (StartingPose.GetPosition() - m_CurrentPose.GetPosition()).LenSqAsVec3() < EG_SMALL_NUMBER )
			{
				// Starting at starting pose (probably a jump) so  start at next node.
				StartingIndex = FindValidNodeStartingAtIndex( StartingIndex + 1 );
			}
		}
	}

	if( !m_bLoop )
	{
		const eg_int SplineLen = m_SplineNodes.LenAs<eg_int>();
		if( m_SplineType == ex_transform_spline_t::CatmullRom && SplineLen < 4 )
		{
			EGLogf( eg_log_t::Warning , "A non-looping Catmull-Rom spline must have at least 4 nodes." );
		}

		eg_int LastKnownSecondToLast = StartingIndex;
		eg_int CurrentKnownLast = LastKnownSecondToLast;
		for( eg_int i=LastKnownSecondToLast+1; i<SplineLen; i++ )
		{
			if( m_SplineNodes[i].bIsSet )
			{
				LastKnownSecondToLast = CurrentKnownLast;
				CurrentKnownLast = i;
			}
		}

		if( m_SplineType == ex_transform_spline_t::CatmullRom )
		{
			// If we are doing catmull rom and not looping, we want to end the animation
			// with one control node left so we'll find the next to last index to end on.
			m_EndIndex = LastKnownSecondToLast;
		}
		else
		{
			m_EndIndex = CurrentKnownLast;
		}

		if( !m_SplineNodes.IsValidIndex( m_EndIndex ) )
		{
			EGLogf( eg_log_t::Warning , "A spline with no valid ending was trying to loop." );
		}
	}

	BeginSeg( StartingIndex );
}

void EGTransformSpline::BeginSpline( const egTransformSplineInitData& InitData )
{
	SetAcceleration( InitData.Acceleration );
	SetSpeed( InitData.StartingSpeed );
	SetMaxSpeed( InitData.MaxSpeed );
	SetSplineType( InitData.SplineType );
	SetLooping( InitData.bLoop );
	BeginSplineInternal( InitData.InitialPose );
}

void EGTransformSpline::StopSpline()
{
	m_bIsPlaying = false;
}

void EGTransformSpline::ResumeSpline()
{
	m_bIsPlaying = true;
}

void EGTransformSpline::Update( eg_real DeltaTime )
{
	if( m_bIsPlaying )
	{
		m_Speed += m_Acceleration * DeltaTime;
		m_Speed = EG_Clamp( m_Speed , 0.f , m_MaxSpeed );

		const eg_real DistThisFrame = m_Speed * DeltaTime;
		m_CurrentSegPos += DistThisFrame;
		if( m_CurrentSegPos < m_CurrentSegLen )
		{
			// eg_transform PrevPose = m_CurrentPose;

			const eg_real t = EGMath_GetMappedRangeValue( m_CurrentSegPos , eg_vec2(0.f,m_CurrentSegLen) , eg_vec2(0.f,1.f) );
			if( m_SplineType == ex_transform_spline_t::CatmullRom )
			{
				m_CurrentPose = eg_transform::CatmullRomLerp( m_SegStartPoseC , m_SegStartPose , m_SegEndPose , m_SegEndPoseC , t );
			}
			else
			{
				m_CurrentPose = eg_transform::Lerp( m_SegStartPose , m_SegEndPose , t );
			}

			// eg_real DistMoved = (m_CurrentPose.GetPos() - PrevPose.GetPos()).LenAsVec3();
			// eg_real CalcSpeed = DistMoved/DeltaTime;
			// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , EGString_Format("Speed: %.2g , C. Speed: %.2g , D. Moved: %.2g" , m_Speed , CalcSpeed , DistMoved ) );
		}
		else
		{
			BeginSeg( m_TargetPoseIdx + 1 );
		}
	}
}

eg_transform EGTransformSpline::GetFirstPose() const
{
	for( const egNodeData& Data : m_SplineNodes )
	{
		if( Data.bIsSet )
		{
			return Data.Pose;
		}
	}

	return CT_Default;
}

void EGTransformSpline::Reset()
{
	m_SplineNodes.Clear();
	m_Id = CT_Clear;
	m_CurrentPose = CT_Default;
	m_SegStartPoseC = CT_Default;
	m_SegStartPose = CT_Default;
	m_SegEndPose = CT_Default;
	m_SegEndPoseC = CT_Default;
	m_TargetPoseIdx = 0;
	m_bLoop = true;
	m_SplineType = ex_transform_spline_t::CatmullRom;
	m_bIsPlaying = false;
	m_Acceleration = 1.f;
	m_Speed = 0.f;
	m_MaxSpeed = 10.f;
	m_CurrentSegLen = 0.f;
	m_CurrentSegPos = 0.f;
}

void EGTransformSpline::BeginSeg( eg_int SegIdx )
{
	if( !m_bLoop && SegIdx > m_EndIndex )
	{
		m_bIsPlaying = false;
		return;
	}

	m_TargetPoseIdx = FindValidNodeStartingAtIndex( SegIdx );
	m_SegStartPoseC = m_SegStartPose;
	m_SegStartPose = m_CurrentPose;

	if( !m_SplineNodes.IsValidIndex( m_TargetPoseIdx ) )
	{
		// No valid nodes at all...
		m_bIsPlaying = false;
		return;
	}

	if( m_SplineNodes.IsValidIndex( m_TargetPoseIdx ) )
	{
		m_SegEndPose = m_SplineNodes[m_TargetPoseIdx].Pose;
	}
	else
	{
		m_SegEndPose = m_SegStartPose;
	}

	const eg_int EndControlPathIdx = FindValidNodeStartingAtIndex( m_TargetPoseIdx + 1 );
	m_SegEndPoseC = m_SplineNodes.IsValidIndex( EndControlPathIdx ) ? m_SplineNodes[EndControlPathIdx].Pose : m_SegEndPose;

	m_CurrentSegPos = 0.f;
	m_CurrentSegLen = (m_SegEndPose.GetTranslation() - m_SegStartPose.GetTranslation()).Len();
}

eg_int EGTransformSpline::FindValidNodeStartingAtIndex( eg_int InIndex ) const
{
	const eg_int SplineLen = m_SplineNodes.LenAs<eg_int>();
	for( eg_int i=0; i<SplineLen; i++ )
	{
		const eg_int NodeToCheck = (InIndex + i)%SplineLen;
		assert( m_SplineNodes.IsValidIndex( NodeToCheck ) ); // Bad logic?
		if( m_SplineNodes[NodeToCheck].bIsSet )
		{
			return NodeToCheck;
		}
	}

	return -1;
}
