/******************************************************************************
ExPlayerController - Main controller for interacting with the maps.

(c) 2015 Beem Media
******************************************************************************/

#include "ExEnt.h"
#include "ExServer.h"
#include "ExInput.h"
#include "ExGame.h"

class ExFreeMovePlayerController : public ExEnt
{
	EG_CLASS_BODY( ExFreeMovePlayerController , ExEnt )

private:
	eg_lockstep_id  m_LockstepId;
	eg_real       m_LookPitch;
	eg_real       m_LookYaw;
	eg_vec4       m_Vel;

public:

	virtual void OnCreate( const egEntCreateParms& Parms ) override final
	{
		Super::OnCreate( Parms );

		if( GetRole() == eg_ent_role::Authority )
		{
			m_LookPitch=0.0f;
			m_LookYaw=0.0f;
			m_Vel = eg_vec4(0,0,0,0);
			
			SetActive( true );
			SetKinematicEnabled( true );
			RunEvent( eg_crc("Init") );
		}
	}

	void OnInput( egLockstepCmds* pCmds , eg_real DeltaTime )
	{
		eg_transform mPos = GetPose();
		mPos.SetRotation( eg_quat::I ); // Don't want any rotation.

		eg_vec4 vT(0,0,0,0);

		if(pCmds->IsActive(CMDA_FREEMOVE_FORWARD))
		{
			vT.z+=1.0f;
		}
		if(pCmds->IsActive(CMDA_FREEMOVE_BACK))
		{
			vT.z-=1.0f;
		}
		if(pCmds->IsActive(CMDA_FREEMOVE_RIGHT))
		{
			vT.x+=1.0f;
		}
		if(pCmds->IsActive(CMDA_FREEMOVE_LEFT))
		{
			vT.x-=1.0f;
		}
		if(pCmds->IsActive(CMDA_FREEMOVE_ASCEND))
		{
			vT.y+=1.0f;
		}
		if(pCmds->IsActive(CMDA_FREEMOVE_DESCEND))
		{
			vT.y-=1.0f;
		}

		const eg_real Accel = 90.f*DeltaTime;
		const eg_real MAX_SPEED = 15.0f;

		auto ApplyVelocityCompenent = [&Accel,&MAX_SPEED]( eg_real& VelValue , const eg_real Adjustment ) -> void
		{
			VelValue += 2.f * Accel * Adjustment;
			if( VelValue > 0 )
			{
				VelValue -= Accel;
				if( VelValue < 0 )VelValue = 0;
				if( VelValue > MAX_SPEED)VelValue = MAX_SPEED;
			}
			else if( VelValue < 0 )
			{
				VelValue += Accel;
				if( VelValue > 0 )VelValue = 0;
				if( VelValue < -MAX_SPEED)VelValue = -MAX_SPEED;
			}
		};

		ApplyVelocityCompenent( m_Vel.x , vT.x );
		ApplyVelocityCompenent( m_Vel.y , vT.y );
		ApplyVelocityCompenent( m_Vel.z , vT.z );


		eg_transform VelTrans;
		VelTrans = eg_transform::BuildRotationX( EG_Rad(-m_LookPitch) );
		VelTrans.RotateYThis( EG_Rad(m_LookYaw) );

		eg_vec4 MoveToApply = m_Vel * VelTrans * DeltaTime;

		//DebugText_MsgOverlay_AddText( DEBUG_TEXT_OVERLAY_SERVER , EGString_Format( "Velocity (%g,%g,%g)" , m_Vel.x , m_Vel.y , m_Vel.z ) );
		//DebugText_MsgOverlay_AddText( DEBUG_TEXT_OVERLAY_SERVER , EGString_Format( "Move (%g,%g,%g)" , MoveToApply.x , MoveToApply.y , MoveToApply.z ) );

		mPos.TranslateThis( MoveToApply.ToVec3() );


		const eg_real ROT_ADJ = 1.f/(60.f*DeltaTime);

		//Determine where looking
		static const eg_real AXIS_ADJ = 0.01f;
		m_LookPitch += pCmds->m_Axis1.y*AXIS_ADJ;
		m_LookYaw += pCmds->m_Axis1.x*AXIS_ADJ;

		m_LookPitch = EG_Clamp( m_LookPitch , -EG_PI*0.5f , EG_PI*0.5f );

		eg_transform m;
		m = eg_transform::BuildRotationX( EG_Rad(-m_LookPitch) );
		m.TranslateThis( 0.f , 0.5f , 0.f );
		// SetLook( m );

		mPos.SetRotation( eg_quat::BuildRotationAxis( eg_vec3( 0.f, 1.f, 0.f) , EG_Rad(m_LookYaw) ) );
		SetPose( mPos );
	}

	virtual void OnUpdate( eg_real DeltaTime ) override
	{
		egLockstepCmds Cmds;
		if( m_LockstepId != INVALID_ID )
		{
			SDK_GetGame()->SDK_GetCommandsForPlayer( m_LockstepId , &Cmds );
			OnInput( &Cmds , DeltaTime );
		}
	}
};

EG_CLASS_DECL( ExFreeMovePlayerController )
