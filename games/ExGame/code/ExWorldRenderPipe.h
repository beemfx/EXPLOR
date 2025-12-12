// (c) 2018 Beem Media

#pragma once

#include "EGWorldRenderPipe.h"
#include "EGRendererTypes.h"
#include "EGWeakPtr.h"
#include "EGRandom.h"
#include "EGCircleArray.h"
#include "EGTween.h"

class ExGame;

class ExWorldRenderPipe : public EGWorldRenderPipe
{
	EG_CLASS_BODY( ExWorldRenderPipe , EGWorldRenderPipe )

protected:

	egv_rtarget       m_RtBg = egv_rtarget::Null;
	EGLight           m_PlayerLight;
	eg_real           m_PlayerLightRange = 10.f;
	eg_real           m_PlayerLightFalloffRange = 7.f;
	eg_bool           m_bHasPlayerLight;
	eg_real           m_TestCameraRotationTime = 0.f;
	eg_vec4           m_SunLightDir;
	eg_real           m_SunTime;
	eg_transform      m_SunPose;
	eg_transform      m_LastPlayerPose = CT_Default;
	eg_vec4           m_LightPerc_Sun_PointLight_Ambient;
	eg_color          m_AmbientLight;
	eg_color          m_TorchColor;
	eg_color          m_SkyColor = eg_color( eg_color32( 121, 176, 204 ) );
	eg_real           m_ViewDistance = 6.f*11;
	eg_real           m_FogStartOffset = 8.f;
	eg_real           m_FogEndOffset = 4.f;
	eg_vec3           m_DofRange = eg_vec3(0.f,0.f,0.f);
	eg_real           m_DofBlurFactor = .0009f;
	eg_real           m_CombatPostBlurFactor = .00045f;
	eg_bool           m_bCombatPostActive = false;
	eg_bool           m_bWantCombatPostActive = false;
	EGTween<eg_real>  m_CombatPostBrightness;
	EGTween<eg_real>  m_CombatPostShiftPower;

	egv_material      m_Dof1PostProcMtrl = EGV_MATERIAL_NULL;
	egv_material      m_Dof2PostProcMtrl =  EGV_MATERIAL_NULL;
	egv_material      m_CombatPostProcMtrl = EGV_MATERIAL_NULL;
	egv_material      m_ColorPostProcMtrl = EGV_MATERIAL_NULL;

	// Torch flicker data:
	eg_vec3 m_PlCurPos = eg_vec3(0.f,0.f,2.f);
	eg_real m_PlTimeElapsed = 0.f;
	eg_bool m_bPlMovingBack = false;
	EGCircleArray<eg_vec3,4> m_PlPoses;
	mutable EGRandom m_Rng = CT_Default;

public:

	virtual void OnConstruct() override;
	virtual void OnDestruct() override;
	void UpdateFromGame( eg_real DeltaTime , ExGame* GameData );
	void SetCombatPostProcActive( eg_bool bNewValue );

protected:

	void UpdatePlayerLightBasePos( eg_real DeltaTime );
	eg_vec4 GetPlayerLightBasePos() const;

	virtual void OnDrawEvent( const egDrawThingInfo& DrawThingInfo , class EGDisplayList* DisplayList ) override final;
	virtual void DrawPrimaryScene( const EGCamera2& Camera ) override final;
	virtual void DrawDebugItems( class EGDisplayList* DisplayList ) override;

	void SetupGamePalette();
};
