// (c) 2020 Beem Media. All Rights Reserved.

#include "ExLookAtComponent.h"
#include "ExPlayerController.h"

EG_CLASS_DECL( ExLookAtComponent )

void ExLookAtComponent::ApplyLookAtToPlayer( ExPlayerController* PlayerEnt ) const
{
	const EGEnt* OwnerEnt = GetOwner<EGEnt>();

	if( PlayerEnt && OwnerEnt )
	{
		switch( m_Type )
		{
			case ex_look_at_component_t::LookWith:
			{
				const eg_transform LookWithPose = GetWorldPose( OwnerEnt->GetPose() );
				PlayerEnt->OnEncounterSetCamera( LookWithPose );
			} break;
			
			case ex_look_at_component_t::LookAt:
			{
				const eg_transform PlayerCamera = PlayerEnt->GetBaseCameraPose();
				const eg_vec3 PlayerPos = PlayerCamera.GetTranslation();
				const eg_transform LookAtPose = GetWorldPose( OwnerEnt->GetPose() );
				const eg_vec3 LookAtPos = LookAtPose.GetTranslation();

				const eg_vec3 ToLookAtVec = LookAtPos - PlayerPos;
				const eg_vec3 RelativeToLookAtVec = (eg_vec4(ToLookAtVec,0.f) * PlayerCamera.GetInverse()).XYZ_ToVec3();

				// We'll only actually deal with pitch and yaw.

				const eg_vec2 PitchVec( RelativeToLookAtVec.z , RelativeToLookAtVec.y );
				const eg_angle Pitch = EGMath_atan2( PitchVec.y , PitchVec.x );

				const eg_vec2 YawVec( RelativeToLookAtVec.z , RelativeToLookAtVec.x );
				const eg_angle Yaw = EGMath_atan2( YawVec.y , YawVec.x );

				eg_transform Camera = CT_Default;
				Camera.RotateXThis( -Pitch );
				Camera.RotateYThis( Yaw );
				Camera *= PlayerCamera;

				const eg_vec4 AdditionalTranslateVector = eg_vec4(0.f,0.f,m_AdditionalLookAtTranslation,0.f) * Camera;
				Camera.TranslateThis( AdditionalTranslateVector.XYZ_ToVec3() );

				PlayerEnt->OnEncounterSetCamera( Camera );
			} break;
		}
	}
}

void ExLookAtComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );

	const ExLookAtComponent* DefAsLookAt = EGCast<ExLookAtComponent>( InitData.Def );

	m_Type = DefAsLookAt->m_Type;
	m_AdditionalLookAtTranslation = DefAsLookAt->m_AdditionalLookAtTranslation;
}

void ExLookAtComponent::OnEnterWorld()
{
	Super::OnEnterWorld();

	if( IsClient() && GetEntRole() != eg_ent_role::EditorPreview )
	{
		SetHidden( true );
	}
}
