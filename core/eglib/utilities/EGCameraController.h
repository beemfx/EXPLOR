// (c) 2017 Beem Media

#pragma once

class EGCameraController
{
private:

	eg_angle m_Yaw = EG_Rad(0.f);
	eg_angle m_Pitch = EG_Rad(0.f);
	eg_vec3 m_Position = eg_vec3(0.f,0.f,0.f);

	mutable eg_bool      m_bFinalTransformDirty = true;
	mutable eg_transform m_FinalTransform;

public:

	void Set( eg_angle InYaw , eg_angle InPitch , const eg_vec3& InPos );
	void SetPose( const eg_transform& Pose );
	
	void MoveUp( eg_real UpAmount );
	void MoveForward( eg_real ForwardAmount );
	void MoveLookDir( eg_real ForwardAmount );
	void MoveStrafe( eg_real StrafeAmount );
	void MoveYaw( eg_angle YawAmount );
	void MovePitch( eg_angle PitchAmount );

	eg_angle GetYaw() const { return m_Yaw; }
	eg_angle GetPitch() const { return m_Pitch; }
	eg_vec3 GetPosition() const { return m_Position; }
	eg_transform GetTransform() const;
};
