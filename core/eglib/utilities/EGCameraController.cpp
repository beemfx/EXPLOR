// (c) 2017 Beem Media

#include "EGCameraController.h"

void EGCameraController::Set( eg_angle InYaw, eg_angle InPitch, const eg_vec3& InPos )
{
	m_Yaw = InYaw;
	m_Pitch = InPitch;
	m_Position = InPos;
	m_bFinalTransformDirty = true;
}

void EGCameraController::SetPose( const eg_transform& Pose )
{
	m_Position = Pose.GetTranslation();

	eg_vec4 NewForward(0.f,0.f,1.f,0.f);
	NewForward *= Pose;
	m_Yaw = EGMath_atan2( NewForward.x , NewForward.z );
	// TODO: Should also calculate pitch...
	m_bFinalTransformDirty = true;
}

void EGCameraController::MoveUp( eg_real UpAmount )
{
	m_Position.y += UpAmount;
	m_bFinalTransformDirty = true;
}

void EGCameraController::MoveForward( eg_real ForwardAmount )
{
	eg_vec4 MoveVec( 0.f , 0.f , ForwardAmount , 0.f );
	MoveVec *= eg_transform::BuildRotationY( m_Yaw );
	m_Position = (eg_vec4(m_Position,1.f) + MoveVec).ToVec3();
	m_bFinalTransformDirty = true;
}

void EGCameraController::MoveLookDir( eg_real ForwardAmount )
{
	eg_vec4 MoveVec( 0.f , 0.f , ForwardAmount , 0.f );
	MoveVec *= GetTransform();
	m_Position = (eg_vec4(m_Position,1.f) + MoveVec).ToVec3();
	m_bFinalTransformDirty = true;
}

void EGCameraController::MoveStrafe( eg_real StrafeAmount )
{
	eg_vec4 MoveVec( StrafeAmount , 0.f , 0.f , 0.f );
	MoveVec *= eg_transform::BuildRotationY( m_Yaw );
	m_Position = (eg_vec4(m_Position,1.f) + MoveVec).ToVec3();
	m_bFinalTransformDirty = true;
}

void EGCameraController::MoveYaw( eg_angle YawAmount )
{
	m_Yaw += YawAmount;
	m_Yaw.NormalizeThis();
	m_bFinalTransformDirty = true;
}

void EGCameraController::MovePitch( eg_angle PitchAmount )
{
	m_Pitch += PitchAmount;
	m_Pitch = EG_Rad(EG_Clamp<eg_real>( m_Pitch.ToRad() , -( EG_PI*.5f - EG_SMALL_NUMBER ) , (EG_PI*.5f-EG_SMALL_NUMBER) ));
	m_bFinalTransformDirty = true;
}

eg_transform EGCameraController::GetTransform() const
{
	if( m_bFinalTransformDirty )
	{
		m_bFinalTransformDirty = false;
		m_FinalTransform = eg_transform::BuildRotationX( m_Pitch ) * eg_transform::BuildRotationY( m_Yaw ) * eg_transform::BuildTranslation( m_Position );
	}
	return m_FinalTransform;
}
