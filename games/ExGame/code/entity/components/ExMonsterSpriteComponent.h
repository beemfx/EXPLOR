// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "EGMeshComponent.h"
#include "ExMonsterSpriteComponent.reflection.h"

egreflect class ExMonsterSpriteComponent : public egprop EGMeshComponent
{
	EG_CLASS_BODY( ExMonsterSpriteComponent , EGMeshComponent )
	EG_FRIEND_RFL( ExMonsterSpriteComponent )

public:

	enum class ex_anim_t
	{
		LoopForward,
		LoopBack,
		LoopForwardBack,
		OnceForward,
		OnceBack,
	};

protected:

	static const eg_int NUM_FRAMES = 16; // Hard coded for now since it optimizes the shader a little, but it doesn't have to be.

	eg_int m_CurrentFrame = 0;
	eg_real m_TimeSinceLastFrame = 0.f;

	eg_int m_FirstFrame = 0;
	eg_int m_LastFrame = 0;
	eg_real m_Duration = 0.f;
	eg_real m_FrameTime = 0.f;
	eg_int m_BaseFrame = 0;
	eg_int m_NumFrames = 1;
	ex_anim_t m_AnimType = ex_anim_t::LoopForward;

public:

	void SetAnimation( eg_int FirstFrame , eg_int LastFrame , eg_real Duration , ex_anim_t AnimType );

protected:

	virtual void OnConstruct() override;
	virtual void RelevantUpdate( eg_real DeltaTime ) override;
	virtual void Draw( const eg_transform& ParentPose ) const override;
	virtual void ScriptExec( const struct egTimelineAction& Action ) override;

	void UpdateFrame( eg_real DeltaTime );
};
