// (c) 2018 Beem Media

#include "ExWorldRenderPipe.h"
#include "EGDisplayList.h"
#include "EGRenderer.h"
#include "EGEntDef.h"
#include "EGEnt.h"
#include "ExDayNightCycles.h"
#include "ExGame.h"
#include "ExMapInfos.h"
#include "EGGlobalConfig.h"
#include "EGDebugText.h"
#include "EGDebugShapes.h"
#include "EGClientView.h"
#include "ExGlobalData.h"

EG_CLASS_DECL( ExWorldRenderPipe )

static EGSettingsBool ExWorldRenderPipe_DebugDrawPlayerLight( "ExWorldRenderPipe.DebugDrawPlayerLight" , eg_loc("SettingsDebugDebugDrawPlayerLight","DEBUG: Draw Player Light") , false , EGS_F_USER_SAVED|EGS_F_DEBUG_EDITABLE );
static EGSettingsBool ExWorldRenderPipe_EnableCombatPostProcessing( "ExWorldRenderPipe.EnableCombatPostProcessing" , eg_loc("SettingsCombatPostProcessingEnabled","Combat Post Processing") , true , EGS_F_SYS_SAVED|EGS_F_EDITABLE );
static EGSettingsBool ExWorldRenderPipe_EnableDepthOfField( "ExWorldRenderPipe.EnableDepthOfField" , eg_loc("SettingsDepthOfFieldPostProcessingEnabled","Depth of Field") , true , EGS_F_SYS_SAVED|EGS_F_EDITABLE );
static EGSettingsClampedInt ExWorldRenderPipe_Gamma( "ExWorldRenderPipe.Gamma", eg_loc("SettingsGamma","Gamma") , 10 , EGS_F_SYS_SAVED|EGS_F_EDITABLE , 1 , 30 , 1 );

void ExWorldRenderPipe::OnConstruct()
{
	Super::OnConstruct();

	m_RtBg = EGRenderer::Get().CreateRenderTarget( 1024 , 1024 );

	auto CreatePostProcMaterial = []( egv_material& Mat , eg_cpstr Path , eg_cpstr Id ) -> void
	{
		EGMaterialDef CombatPostProcMatDef( CT_Default );
		CombatPostProcMatDef.SetShader( Path );
		Mat = EGRenderer::Get().CreateMaterial( &CombatPostProcMatDef , Id );
	};

	CreatePostProcMaterial( m_CombatPostProcMtrl , "/game/shaders/ex_combat_post" , "CombatPostProcMtrlDef" );
	CreatePostProcMaterial( m_Dof1PostProcMtrl , "/game/shaders/ex_dof1_post" , "Dof1MtrlDef" );
	CreatePostProcMaterial( m_Dof2PostProcMtrl , "/game/shaders/ex_dof2_post" , "Dof2MtrlDef" );
	CreatePostProcMaterial( m_ColorPostProcMtrl , "/game/shaders/ex_color_post" , "ColorPostProcMtrlDef" );

	m_AmbientLight = eg_color::White;
	m_TorchColor = eg_color::White;
	m_LightPerc_Sun_PointLight_Ambient = eg_vec4(1.f,1.f,1.f,1.f);

	for( eg_int i=0; i<4; i++ )
	{
		m_PlPoses.InsertLast( eg_vec3(0.f,0.f,0.f) );
	}
}

void ExWorldRenderPipe::OnDestruct()
{
	EGRenderer::Get().DestroyRenderTarget( m_RtBg );
	m_RtBg = egv_rtarget::Null;

	EGRenderer::Get().DestroyMaterial( m_CombatPostProcMtrl );
	m_CombatPostProcMtrl = EGV_MATERIAL_NULL;

	EGRenderer::Get().DestroyMaterial( m_Dof1PostProcMtrl );
	m_Dof1PostProcMtrl = EGV_MATERIAL_NULL;

	EGRenderer::Get().DestroyMaterial( m_Dof2PostProcMtrl );
	m_Dof2PostProcMtrl = EGV_MATERIAL_NULL;

	EGRenderer::Get().DestroyMaterial( m_ColorPostProcMtrl );
	m_ColorPostProcMtrl = EGV_MATERIAL_NULL;

	Super::OnDestruct();
}

void ExWorldRenderPipe::UpdateFromGame( eg_real DeltaTime , ExGame* GameData )
{
	UpdatePlayerLightBasePos( DeltaTime );
	
	auto ComputeAmbientColorByNormalizedTimeOfDay = [this,&GameData]( eg_real NormalizedTime ) -> void
	{
		const EGArray<exDayNightCycleState>& CurCycle = GameData->GetCurrentDayNightCycle();

		exDayNightCycleState FirstColor;
		exDayNightCycleState SecondColor;

		if( CurCycle.Len() == 0 )
		{
			SecondColor.NormalizedTime = 1.f;
		}
		else if( CurCycle.Len() == 1 )
		{
			FirstColor = CurCycle[0];
			SecondColor = CurCycle[0];
			SecondColor.NormalizedTime = 1.f;
		}
		else if( CurCycle.Len() >= 2 )
		{
			FirstColor = CurCycle[0];
			SecondColor = CurCycle[1];

			for( eg_size_t i=0; i<CurCycle.Len(); i++ )
			{
				const exDayNightCycleState& Cur = CurCycle[i];
				if( NormalizedTime >= Cur.NormalizedTime )
				{
					FirstColor = Cur;
					if( (i+1) < CurCycle.Len() )
					{
						SecondColor = CurCycle[i+1];
					}
					else
					{
						SecondColor = CurCycle[0];
						SecondColor.NormalizedTime = 1.f;
					}
				}
			}
		}

		// DebugText_MsgOverlay_AddText( DEBUG_TEXT_OVERLAY_CLIENT , EGString_Format( "Normalized Time %g" , NormalizedTime ) );

		eg_vec4 FirstLightPerc = eg_vec4( FirstColor.SunIntensity , FirstColor.TorchIntensity , FirstColor.AmbientIntensity , 0.f );
		eg_vec4 SecondLightPerc = eg_vec4( SecondColor.SunIntensity , SecondColor.TorchIntensity , SecondColor.AmbientIntensity , 0.f );

		eg_real Blend = EGMath_GetMappedRangeValue( NormalizedTime , eg_vec2(FirstColor.NormalizedTime,SecondColor.NormalizedTime) , eg_vec2(0.f,1.f) );

		m_AmbientLight = eg_color(FirstColor.SunColor.ToVec4()*(1.f-Blend) + SecondColor.SunColor.ToVec4()*(Blend));
		m_TorchColor = eg_color(FirstColor.TorchColor.ToVec4()*(1.f-Blend) + SecondColor.TorchColor.ToVec4()*(Blend));
		m_LightPerc_Sun_PointLight_Ambient = FirstLightPerc*(1.f-Blend) + SecondLightPerc*(Blend);
	};

	m_TestCameraRotationTime += DeltaTime*( EG_PI*2.f ) / 5.f;

	m_CombatPostBlurFactor = ExGlobalData::Get().GetCombatPostBlurFactor();

	// Compute the player light:
	if( GameData )
	{
		m_PlayerLight = EGLight( CT_Clear );
		m_bHasPlayerLight = false;
		const exMapInfoFogData& FogData = GameData->GetCurrentMapInfo().FogData;
		m_ViewDistance = FogData.ViewDistance * FogData.ViewDistanceUnitSize;
		m_FogStartOffset = EG_Min( FogData.FogStartOffset , m_ViewDistance );
		m_FogEndOffset = EG_Min( FogData.FogEndOffset , m_FogStartOffset );
		m_DofRange = FogData.DepthOfFieldRange;
		m_DofBlurFactor = FogData.DepthOfFieldBlurFactor;
		// We want a light where the player entity is:
		EGEnt* PlayerEnt = GameData->SDK_GetEnt( GameData->GetPrimaryPlayer() );
		if( PlayerEnt )
		{
			const eg_vec4 PlayerLightBasePos = GetPlayerLightBasePos();
			m_LastPlayerPose = PlayerEnt->GetPose();
			m_PlayerLight.Pos = PlayerLightBasePos * m_LastPlayerPose;
			// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "Player Light {0} (Base {1})" , EGVec4Formatter(m_PlayerLight.Pos) , EGVec4Formatter(PlayerLightBasePos) ) );
			m_PlayerLight.Color = m_TorchColor;
			m_PlayerLight.SetRangeSq( m_PlayerLightRange*m_PlayerLightRange , m_PlayerLightFalloffRange*m_PlayerLightFalloffRange );
			m_PlayerLight.SetIntensity( 1.f );
			m_bHasPlayerLight = true;

			const eg_real NormalizedTime = GameData->GetNormalizedTimeOfDay();
			m_SunTime = EGMath_GetMappedRangeValue( NormalizedTime , eg_vec2(0.f,1.f) , eg_vec2(-EG_PI,EG_PI) );
			m_SunPose = eg_transform::BuildTranslation( eg_vec3(0.f , 4.f , 0.f) );
			m_SunPose.RotateZThis( EG_Rad(m_SunTime) );
			m_SunLightDir = -m_SunPose.GetPosition();
			m_SunLightDir.w = 0.f;
			m_SunLightDir.NormalizeThisAsVec3();
			// We want it above the player.
			eg_vec4 PlayerPos = m_LastPlayerPose.GetPosition();
			PlayerPos.w = 0.f;
			m_SunPose.TranslateThis( PlayerPos.ToVec3() );

			ComputeAmbientColorByNormalizedTimeOfDay( NormalizedTime );
		}
	}
	else
	{
		m_ViewDistance = 6.f*11;
		m_FogStartOffset = 8.f;
		m_FogEndOffset = 4.f;

		m_DofRange = ExMapInfo_DefaultDof;
		m_DofBlurFactor = .0009f;
	}

	if( m_bCombatPostActive )
	{
		m_CombatPostBrightness.Update( DeltaTime );
		m_CombatPostShiftPower.Update( DeltaTime );

		if( m_bWantCombatPostActive )
		{
			if( !m_CombatPostShiftPower.IsAnimating() )
			{
				const eg_bool bPos = m_CombatPostShiftPower.GetCurrentValue() >= 0.f;

				static const eg_real POST_SHIFT_SPEED = 3.f;
				m_CombatPostShiftPower.MoveTo( bPos ? -1.f : 1.f , POST_SHIFT_SPEED , eg_tween_animation_t::Cubic );
			}
		}
		else
		{
			if( !m_CombatPostBrightness.IsAnimating() && !m_CombatPostShiftPower.IsAnimating() )
			{
				m_bCombatPostActive = false;
			}
		}
	}
}

void ExWorldRenderPipe::SetCombatPostProcActive( eg_bool bNewValue )
{
	static const eg_real TIME_TO_FADE_BRIGHTNESS = .5f;

	m_bWantCombatPostActive = bNewValue;

	if( m_bWantCombatPostActive )
	{
		if( !m_bCombatPostActive && ExWorldRenderPipe_EnableCombatPostProcessing.GetValue() )
		{
			m_bCombatPostActive = true;
			m_CombatPostShiftPower.SetValue( 0.f );
			m_CombatPostBrightness.SetValue( 0.f );
			m_CombatPostBrightness.MoveTo( 1.f , TIME_TO_FADE_BRIGHTNESS , eg_tween_animation_t::Cubic );
		}
	}
	else
	{
		if( m_bCombatPostActive )
		{
			m_CombatPostShiftPower.MoveTo( 0.f , TIME_TO_FADE_BRIGHTNESS , eg_tween_animation_t::Cubic );
			m_CombatPostBrightness.MoveTo( 0.f , TIME_TO_FADE_BRIGHTNESS , eg_tween_animation_t::Cubic );
		}
	}
}

void ExWorldRenderPipe::UpdatePlayerLightBasePos( eg_real DeltaTime )
{
	static const eg_real MOVE_DURATION = .2f;

	m_PlTimeElapsed += DeltaTime;
	if( m_PlTimeElapsed >= MOVE_DURATION )
	{
		static const eg_vec2 XRange(-.25f,.25f);
		static const eg_vec2 YRange(.5f,1.f);
		static const eg_vec2 ZRange(.5f,.5f);//(1.f,1.1f);

		eg_vec3 NewPos;
		NewPos.x = m_Rng.GetRandomRangeF(XRange.x,XRange.y);
		NewPos.y = m_Rng.GetRandomRangeF(YRange.x,YRange.y);
		NewPos.z = m_Rng.GetRandomRangeF(ZRange.x,ZRange.y);
		m_PlPoses.RemoveFirst();
		m_PlPoses.InsertLast( NewPos );

		m_PlTimeElapsed = 0.f;
		m_PlCurPos = m_PlPoses[1];
	}
	else
	{
		const eg_real t = EGMath_GetMappedRangeValue( m_PlTimeElapsed , eg_vec2(0.f,MOVE_DURATION) , eg_vec2(0.f,1.f) );
		const eg_vec4 Vecs[4] =
		{
			eg_vec4(m_PlPoses[0],1.f),
			eg_vec4(m_PlPoses[1],1.f),
			eg_vec4(m_PlPoses[2],1.f),
			eg_vec4(m_PlPoses[3],1.f),
		};
		m_PlCurPos = EGMath_CatmullRomInterpolation( Vecs , t ).ToVec3();
	}
}

eg_vec4 ExWorldRenderPipe::GetPlayerLightBasePos() const
{
	return eg_vec4( m_PlCurPos , 1.f );
}

void ExWorldRenderPipe::OnDrawEvent( const egDrawThingInfo& DrawThingInfo, class EGDisplayList* DisplayList )
{
	auto SetPlayerLight = [this,&DisplayList]( eg_uint LightIndex ) -> void
	{
		if( m_bHasPlayerLight )
		{
			DisplayList->SetLight( LightIndex , m_PlayerLight );
			DisplayList->EnableLight( LightIndex , true );
		}
	};

	auto SetSunLight = [this,&DisplayList]( eg_uint LightIndex ) -> void
	{
		EGLight Light( CT_Clear );
		Light.Color = eg_color( eg_color32( 255, 255, 255 ) );
		eg_bool bRotateLight = false;
		if( bRotateLight )
		{
			Light.Dir = eg_vec4(0.f,-1.f,0.f,0.f);// eg_vec4( 0.1f, -1.f, 0.1f, 0.f );
			eg_transform LightRotation;
			LightRotation = eg_transform::BuildRotationZ( EG_Rad(m_TestCameraRotationTime) );
			Light.Dir *= LightRotation;
			Light.Dir.NormalizeThisAsVec3();
		}
		else
		{
			Light.Dir = m_SunLightDir;
		}
		Light.SetIntensity( .2f );
		DisplayList->SetLight( LightIndex , Light );
		DisplayList->EnableLight( LightIndex , true );

		DisplayList->SetVec4( eg_rv4_t::AMBIENT_LIGHT , m_AmbientLight.ToVec4() );
		DisplayList->SetVec4( eg_rv4_t::GAME_PALETTE_2 , m_LightPerc_Sun_PointLight_Ambient );
	};


	switch( DrawThingInfo.DrawThing )
	{
	case eg_draw_thing::PRE_TERRAIN:
	{
		SetPlayerLight( 0 );
		SetSunLight( 3 );
	} break;
	case eg_draw_thing::PRE_SHADOW:
	{
		m_MaxShadowDistance = m_ViewDistance*.5f;
	} break;
	case eg_draw_thing::PRE_ENT:
	{
		SetPlayerLight( 2 );
		SetSunLight( 3 );
		DisplayList->SetVec4( eg_rv4_t::AMBIENT_LIGHT , m_AmbientLight.ToVec4() );

	} break;
	case eg_draw_thing::PRE_MAP:
	{
		//DisplayList->ClearRT(m_SkyColor);
		SetupGamePalette();
		SetPlayerLight( 0 );
		SetSunLight( 3 );

	} break;

	default: break;
	}
}

void ExWorldRenderPipe::DrawPrimaryScene( const EGCamera2& Camera )
{
	eg_flags PassFlags( CT_Clear );
	if( VideoConfig_PostFXAA.GetValue() )
	{
		PassFlags.SetState( DL_F_PROC_FXAA, true );
	}
	PassFlags.SetState( DL_F_PROC_TOSCREEN, true );

	MainDisplayList->BeginMainScene( 0 );

	DrawEnts( eg_string_crc( ENT_RENDER_FILTER_WORLD ) );
	DrawTerrain();

	MainDisplayList->ResolveToRenderTarget( m_RtBg );
	MainDisplayList->SetGlobalSamplers( m_RtBg , egv_rtarget::Null , egv_rtarget::Null );

	DrawMap();
	DrawEnts( eg_string_crc(ENT_RENDER_FILTER_PRE_FX) );
	DrawEnts( eg_crc("PreFxTransparent") );

	if( VideoConfig_PostMotionBlur.GetValue() )
	{
		MainDisplayList->DoSceneProc( DL_F_PROC_STOREDEPTH | DL_F_PROC_MOTIONBLUR );
	}
	else
	{
		MainDisplayList->DoSceneProc( DL_F_PROC_STOREDEPTH );
	}

	EGClientView::SetRenderTransforms( MainDisplayList , Camera );
	DrawEnts( eg_crc("NpcPass") );
	DrawEnts( eg_crc("TextPass") );
	DrawEnts( eg_crc("PostNpcTransparent") );

	if( ExWorldRenderPipe_EnableDepthOfField.GetValue() )
	{
		MainDisplayList->DoSceneProc( DL_F_PROC_STOREDEPTH );

		MainDisplayList->SetWorldTF( eg_mat::I );
		MainDisplayList->SetViewTF( eg_mat::I );
		MainDisplayList->SetProjTF( eg_mat::I );
		MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF );
		MainDisplayList->PushBlendState( eg_blend_s::BLEND_NONE_COLOR_ALL );
		MainDisplayList->PushSamplerState( eg_sampler_s::TEXTURE_CLAMP_FILTER_POINT );


		auto ComputeDepthBufferValue = [&Camera]( eg_real InValue ) -> eg_real
		{
			eg_vec4 TempV( 0.f , 0.f , InValue , 1.f );
			TempV = TempV * Camera.GetProjMat();
			return TempV.z/TempV.w;
		};

		const eg_vec3 DepthBufferDofRange( ComputeDepthBufferValue( m_DofRange.x ) , ComputeDepthBufferValue( m_DofRange.y ) , ComputeDepthBufferValue( m_DofRange.z ) );

		// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "Dof range: {0} {1}" , DepthBufferDofRange.x , DepthBufferDofRange.y ) );

		MainDisplayList->SetVec4( eg_rv4_t::GAME_PALETTE_0, eg_vec4( DepthBufferDofRange.x , DepthBufferDofRange.y , DepthBufferDofRange.z , m_DofBlurFactor ) );
		MainDisplayList->RunFullScreenProc( m_Dof1PostProcMtrl );
		MainDisplayList->SetVec4( eg_rv4_t::GAME_PALETTE_0, eg_vec4( DepthBufferDofRange.x , DepthBufferDofRange.y , DepthBufferDofRange.z , m_DofBlurFactor*Camera.GetAspectRatio() ) );
		MainDisplayList->RunFullScreenProc( m_Dof2PostProcMtrl );

		MainDisplayList->PopDepthStencilState();
		MainDisplayList->PopBlendState();
		MainDisplayList->PopSamplerState();

		SetupGamePalette();
	}

	MainDisplayList->SetWorldTF( eg_mat::I );
	MainDisplayList->SetViewTF( eg_mat::I );
	MainDisplayList->SetProjTF( eg_mat::I );
	MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF );
	MainDisplayList->PushBlendState( eg_blend_s::BLEND_NONE_COLOR_ALL );
	MainDisplayList->PushSamplerState( eg_sampler_s::TEXTURE_CLAMP_FILTER_POINT );

	const eg_real Gamma = ExWorldRenderPipe_Gamma.GetValue()*.1f;
	if( !EG_IsEqualEps( Gamma , 1.f , .09f ) )
	{
		MainDisplayList->ResolveToRenderTarget( m_RtBg );
		MainDisplayList->SetGlobalSamplers( m_RtBg , egv_rtarget::Null , egv_rtarget::Null );
		MainDisplayList->SetVec4( eg_rv4_t::GAME_PALETTE_0 , eg_vec4( Gamma , 0.f , 0.f , 0.f ) );
		MainDisplayList->RunFullScreenProc( m_ColorPostProcMtrl );
	}

	if( m_bCombatPostActive && ExWorldRenderPipe_EnableCombatPostProcessing.GetValue() )
	{
		MainDisplayList->ResolveToRenderTarget( m_RtBg );
		MainDisplayList->SetGlobalSamplers( m_RtBg , egv_rtarget::Null , egv_rtarget::Null );
		// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "Combat Post Power: {0}" , m_CombatPostShiftPower.GetCurrentValue() ) );
		MainDisplayList->SetVec4( eg_rv4_t::GAME_PALETTE_0, eg_vec4( m_CombatPostBrightness.GetCurrentValue() , m_CombatPostShiftPower.GetCurrentValue() , m_CombatPostBlurFactor*Camera.GetAspectRatio() , 0.f ) ); // X: World Brightness
		MainDisplayList->RunFullScreenProc( m_CombatPostProcMtrl );
	}

	MainDisplayList->PopDepthStencilState();
	MainDisplayList->PopBlendState();
	MainDisplayList->PopSamplerState();

	SetupGamePalette();

	MainDisplayList->EndMainScene( PassFlags );

	MainDisplayList->SetGlobalSamplers( egv_rtarget::Null , egv_rtarget::Null , egv_rtarget::Null );
}

void ExWorldRenderPipe::DrawDebugItems( class EGDisplayList* DisplayList )
{
	Super::DrawDebugItems( DisplayList );

	if( EX_CHEATS_ENABLED && ExWorldRenderPipe_DebugDrawPlayerLight.GetValue() )
	{
		EGDebugSphere* Sphere = EGDebugShapes::Get().GetSphere();
		if( DisplayList && Sphere )
		{
			const eg_vec4 FinalLightPos = GetPlayerLightBasePos() * m_LastPlayerPose;
			const eg_transform Pose = eg_transform::BuildTranslation( FinalLightPos.ToVec3() );
			Sphere->DrawDebugShape( .2f , Pose , m_TorchColor );
			DisplayList->PushRasterizerState( eg_rasterizer_s::CULL_NONE );
			// Sphere->DrawDebugShape( m_PlayerLightRange , Pose , eg_color(m_TorchColor.r,m_TorchColor.g,m_TorchColor.b,.5f) );
			DisplayList->PopRasterizerState();
		}
	}
}

void ExWorldRenderPipe::SetupGamePalette()
{
	const eg_real FogStart = m_ViewDistance - m_FogStartOffset;
	const eg_real FogEnd = m_ViewDistance - m_FogEndOffset;
	const eg_real FadeEnd = m_ViewDistance;
	MainDisplayList->SetVec4( eg_rv4_t::GAME_PALETTE_0, eg_vec4( FogStart, FogEnd, FadeEnd, 0.f ) ); // The Palette x,y is the fog start and end.
	MainDisplayList->SetVec4( eg_rv4_t::GAME_PALETTE_1, m_SkyColor.ToVec4() );
}

