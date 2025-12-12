// (c) 2015 Beem Media

#pragma once

#include "ExMapNavBase.h"

struct egLockstepCmds;

class ExPlayerController : public ExMapNavBase
{
	EG_CLASS_BODY( ExPlayerController , ExMapNavBase )

public:

	static const eg_uint MAX_CYCLES = 3;

private:

	//
	// Types
	//
	struct exRepData
	{
		eg_uint xLocation;
		eg_uint yLocation;
	};

	struct exFocusCamera
	{
		eg_transform CameraPose;
		eg_real      TimeSinceLook;
		eg_real      TransitionTime;
		eg_bool      bUsingFocusCamera:1;
		eg_bool      bIsRestoring:1;

		void SetFocus( const eg_transform& InCameraPose , eg_real InTransitionTime );
		void RestoreFocus();
		void Update( eg_real DeltaTime );
		eg_real GetTransitionPct() const;
	};

	typedef EGBoolList<MAX_CYCLES> ExCycleList;

	//
	// Data
	//
	egNetSmoothTransform m_LookTransform;
	exRepData            m_RepData;
	eg_real              m_StateProgress; // [0,1]
	eg_real              m_StateRate;     // Speed at which state progress happens.
	eg_transform         m_StateStartPose;
	eg_transform         m_StateEndPose;
	eg_real              m_WalkCycle;
	ExCycleList          m_CyclesPassed;
	EGNavGraph           m_NavGraph = CT_Preserve;
	eg_real              m_HeadSwayTime;
	eg_real              m_HeadBobTime;
	eg_real              m_HeadBobIntensity;
	exFocusCamera        m_FocusCamera;
	eg_ent_id            m_TargetedEnt;
	eg_bool              m_bWentIdleFromWalk:1;
	eg_bool              m_bWentIdleFromTurn:1;

public:

	ExPlayerController();

	void OnEncounterStarted();
	void OnEncounterSetCamera( const eg_transform& CameraPose );
	void RestoreCamera(){ m_FocusCamera.RestoreFocus(); }

	eg_transform GetCameraPose() const;
	eg_transform GetBaseCameraPose() const;
	ex_move_s GetMoveState() const { return m_MoveState; }

	virtual void TeleporTo( const eg_transform& NewPose ) override final;

private:

	virtual void OnCreate( const egEntCreateParms& Parms ) override final;
	virtual void OnEnterWorld() override;
	virtual void OnLeaveWorld() override;
	void OnWorldLoadComplete();
	virtual void AssignToPlayer( const eg_lockstep_id& InLockstepId ) override final;
	void OnStoppedMoving();
	virtual void OnUpdate( eg_real DeltaTime ) override final;
	void OnUpdate_HandleInput( const egLockstepCmds& Cmds , eg_real DeltaTime );
	eg_real UpdateHeadSway( eg_real DeltaTime , eg_bool bWalking );
	eg_vec4 UpdateHeadBob( eg_real DeltaTime , eg_bool bWalking );
	void CloseLastDoorIfOpen();
	void RefreshTargetedEnt();
};