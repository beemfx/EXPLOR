// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

enum class ex_transform_spline_t
{
	Linear ,
	CatmullRom ,
};

struct egTransformSplineInitData
{
	eg_transform          InitialPose   = CT_Default;
	eg_bool               bLoop         = true;
	ex_transform_spline_t SplineType    = ex_transform_spline_t::CatmullRom;
	eg_real               Acceleration  = 1.f;
	eg_real               StartingSpeed = 0.f;
	eg_real               MaxSpeed      = 10.f;
};

class EGTransformSpline
{
protected:
	
	struct egNodeData
	{
		eg_transform Pose = CT_Default;
		eg_bool bIsSet    = false;
	};

protected:
	
	EGArray<egNodeData>   m_SplineNodes;
	eg_string_crc         m_Id            = CT_Clear;
	eg_transform          m_CurrentPose   = CT_Default;
	eg_transform          m_SegStartPoseC = CT_Default;
	eg_transform          m_SegStartPose  = CT_Default;
	eg_transform          m_SegEndPose    = CT_Default;
	eg_transform          m_SegEndPoseC   = CT_Default;
	eg_int                m_TargetPoseIdx = 0;
	eg_bool               m_bLoop         = true;
	ex_transform_spline_t m_SplineType    = ex_transform_spline_t::CatmullRom;
	eg_int                m_EndIndex      = 0; // Only relevant if not blooping
	eg_bool               m_bIsPlaying    = false;
	eg_real               m_Acceleration  = 1.f;
	eg_real               m_Speed         = 0.f;
	eg_real               m_MaxSpeed      = 10.f;
	eg_real               m_CurrentSegLen = 0.f;
	eg_real               m_CurrentSegPos = 0.f;

public:
	
	void SetId( eg_string_crc InId ) { m_Id = InId; }
	const eg_string_crc& GetId() const { return m_Id; }
	void SetSplineNodes( const EGArray<eg_transform>& In );
	void InsertSplineNode( eg_int InIndex , const eg_transform& InPose );
	void RemoveSplineNode( eg_int InIndex );
	void BeginSpline( const egTransformSplineInitData& InitData );
	void StopSpline();
	void ResumeSpline();
	void Update( eg_real DeltaTime );
	eg_transform GetFirstPose() const;
	const eg_transform& GetCurrentPose() const { return m_CurrentPose; }

	void SetAcceleration( eg_real NewValue ) { m_Acceleration = NewValue; }
	void SetSpeed( eg_real NewValue ) { m_Speed = NewValue; }
	void SetMaxSpeed( eg_real NewValue ) { m_MaxSpeed = NewValue; }
	void SetSplineType( ex_transform_spline_t NewValue ) { m_SplineType = NewValue; }
	void SetLooping( eg_bool bNewValue ) { m_bLoop = bNewValue; }

	void Reset();

private:

private:

	void BeginSplineInternal( const eg_transform& InitialPose );
	void BeginSeg( eg_int SegIdx );
	eg_int FindValidNodeStartingAtIndex( eg_int InIndex ) const;
};
