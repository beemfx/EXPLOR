// (c) 2018 Beem Media

#include "EGWorldRenderPipe.h"
#include "EGRenderer.h"
#include "EGClientView.h"
#include "EGWorldSceneGraph.h"
#include "EGGlobalConfig.h"

EG_CLASS_DECL( EGWorldRenderPipe )

void EGWorldRenderPipe::DrawSceneGraph( const EGWorldSceneGraph* SceneGraph , const EGCamera2& Camera , const egWorldRenderPipeDrawSpecs& DrawSpecs )
{
	m_WorldSceneGraph = SceneGraph;

	const egRendererSpecs RendererSpecs = EGRenderer::Get().GetSpecs();

	if( DrawSpecs.bIsSplitscreen || RendererSpecs.bDoSimpleOnePass )
	{
		MainDisplayList->BeginMainScene( DL_F_PROC_TOSCREEN );
		DrawScene( eg_draw_scene_reason::DRAW_MAIN_SCENE );
		MainDisplayList->EndMainScene( 0 );
	}
	else
	{
		if( RendererSpecs.bDrawZScene )
		{
			EGClientView::SetRenderTransforms( MainDisplayList , Camera );
			MainDisplayList->BeginMainScene( DL_F_SCENE_DRAWTOZ );
			DrawScene( eg_draw_scene_reason::DRAW_Z_BUFFER );
			MainDisplayList->EndMainScene( 0 );
		}

		if( SceneGraph && SceneGraph->GetEntForReflectiveDraw( 0 ) )
		{
			MainDisplayList->BeginMainScene( DL_F_SCENE_REFLECT );
			Draw_Scene_Reflection( Camera );
			MainDisplayList->EndMainScene( 0 );
		}

		EGClientView::SetRenderTransforms( MainDisplayList , Camera );
		DrawPrimaryScene( Camera );

		MainDisplayList->DisableAllLights();
		MainDisplayList->SetMaterial( EGV_MATERIAL_NULL );

		// Draw volume shadows if the renderer wants them.
		if( RendererSpecs.bDrawVolumeShadows )
		{
			MainDisplayList->BeginMainScene( DL_F_SCENE_SHADOWS );
			DrawShadows( Camera );
			MainDisplayList->EndMainScene( 0 );
		}
	}
	m_WorldSceneGraph = nullptr;
}

void EGWorldRenderPipe::OnDrawEvent( const struct egDrawThingInfo& DrawThingInfo , class EGDisplayList* DisplayList )
{
	unused( DrawThingInfo, DisplayList );
}

void EGWorldRenderPipe::DrawScene( eg_draw_scene_reason Reason )
{
	unused( Reason );

	//CalcVisibility should have been called prior to calling this and m_CurVisInfo should be set.
	DrawTerrain();
	DrawMap();
	DrawEnts( CT_Clear );
}

void EGWorldRenderPipe::DrawPrimaryScene( const EGCamera2& Camera )
{
	

	eg_flags PassFlags( CT_Clear );
	if( VideoConfig_PostFXAA.GetValueThreadSafe() )
	{
		PassFlags.SetState( DL_F_PROC_FXAA, true );
	}
	PassFlags.SetState( DL_F_PROC_TOSCREEN, true );

	MainDisplayList->BeginMainScene( 0 );

	DrawWorld( MainDisplayList , Camera );
	
	MainDisplayList->EndMainScene( PassFlags );
}


void EGWorldRenderPipe::DrawWorld( EGDisplayList* DisplayList , const EGCamera2& Camera )
{
	unused( Camera );

	DrawEnts( eg_string_crc( ENT_RENDER_FILTER_WORLD ) );
	DrawTerrain();
	DrawMap();
	DrawEnts( eg_string_crc( ENT_RENDER_FILTER_PRE_FX ) );
	if( VideoConfig_PostMotionBlur.GetValue() )
	{
		DisplayList->DoSceneProc( DL_F_PROC_STOREDEPTH|DL_F_PROC_MOTIONBLUR );
	}
	else
	{
		DisplayList->DoSceneProc( DL_F_PROC_STOREDEPTH );
	}
	DrawEnts( eg_string_crc(ENT_RENDER_FILTER_POST_FX) );
}

void EGWorldRenderPipe::DrawMap()
{	
	MainDisplayList->PushDefaultShader( eg_defaultshader_t::MAP );
	MainDisplayList->DisableAllLights(); // Game callback may have set lights, we don't want them anymore.

	OnDrawEvent( eg_draw_thing::PRE_MAP, MainDisplayList );

	if( m_WorldSceneGraph )
	{
		m_WorldSceneGraph->DrawMapRegions();
	}

	MainDisplayList->DisableAllLights(); // Game callback may have set lights, we don't want them anymore.

	MainDisplayList->PopDefaultShader();
}

void EGWorldRenderPipe::DrawTerrain()
{
	if( m_WorldSceneGraph )
	{
		OnDrawEvent( eg_draw_thing::PRE_TERRAIN , MainDisplayList );
		m_WorldSceneGraph->DrawTerrains();
	}
}

void EGWorldRenderPipe::DrawEnts( eg_string_crc Filter )
{
	//Finally we render the entities. The entity priority is actually back to
	//front that way if there is some alpha blending going on we won't have
	//entities disappearing behind transparent entities.
	MainDisplayList->PushDefaultShader( eg_defaultshader_t::MESH_LIT );

	if( m_WorldSceneGraph )
	{
		auto PreDraw = [this]() -> void
		{
			OnDrawEvent( eg_draw_thing::PRE_ENT , MainDisplayList ); 
		};
		m_WorldSceneGraph->DrawComponents( Filter , PreDraw );
	}

	MainDisplayList->PopDefaultShader();
	MainDisplayList->DisableAllLights();
	MainDisplayList->SetMaterial(EGV_MATERIAL_NULL);
}

void EGWorldRenderPipe::DrawShadows( const EGCamera2& Camera )
{
	EGClientView::SetRenderTransforms( MainDisplayList , Camera );

	m_MaxShadowDistance = Camera.GetFar();

	OnDrawEvent( eg_draw_thing::PRE_SHADOW , MainDisplayList );

	eg_real FarPlaneDistSq = m_MaxShadowDistance;
	FarPlaneDistSq *= FarPlaneDistSq;

	if( m_WorldSceneGraph )
	{
		m_WorldSceneGraph->DrawShadows( FarPlaneDistSq );
	}
}

void EGWorldRenderPipe::Draw_Scene_Reflection( const EGCamera2& Camera )
{
	if( m_WorldSceneGraph && m_WorldSceneGraph->GetEntForReflectiveDraw( 0 ) )
	{
		const EGEnt* pEnt = m_WorldSceneGraph->GetEntForReflectiveDraw( 0 );

		//STEP 2: Next we draw the actual reflection.
		//-------------------------------------------
		//We setup the mirror plane, the mirror plane is setup according
		//to the position of the object, in local space an object reflects
		//across the XY plane, so it is up to the artist to setup the mesh
		//appropriately.
		eg_vec4 vPos(0,0,0,1);
		eg_vec4 vNorm(0,0,-1,0);
		vPos  *= pEnt->GetPose();
		vNorm *= pEnt->GetPose();
		EGClientView::SetReflectiveRenderTransforms( MainDisplayList , Camera , vPos , vNorm );
		
		DrawScene( eg_draw_scene_reason::DRAW_REFLECTED_SCENE );

		//STEP 3: Render the mirror normally.
		//-----------------------------------
		// The mirror object will be drawn later, and it is up to it to sample the reflected texture correctly.
	}
}

void EGWorldRenderPipe::Draw_DebugDrawMapOnly()
{
	DrawMap();
	DrawTerrain();
	DrawEnts( CT_Clear );
}
