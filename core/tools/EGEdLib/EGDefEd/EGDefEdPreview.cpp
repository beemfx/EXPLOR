// (c) 2017 Beem Media

#include "EGDefEdPreview.h"
#include "EGRenderer.h"
#include "EGDefEdPreviewPanel.h"
#include "EGEntDef.h"
#include "EGDefEdFile.h"
#include "EGEngineForTools.h"
#include "EGDebugShapes.h"
#include "EGDebugText.h"

EGDefEdPreview EGDefEdPreview::s_Instance;

void EGDefEdPreview::Init()
{
	m_DbLightObj = EGNewObject<EGDebugSphere>( eg_mem_pool::Default );
	ResetCamera();
}

void EGDefEdPreview::Deinit()
{
	EG_SafeRelease( m_DbLightObj );
}

void EGDefEdPreview::Draw()
{
	EGRenderer::Get().BeginFrame();

	if( EGDefEdPreviewPanel::GetPanel() )
	{
		egEntDefEdConfig* EditorConfig = EGDefEdFile::Get().GetConfig();
		if( EditorConfig && EditorConfig->bOrthoPreview )
		{
			EGCamera2 Cam( CT_Default );
			Cam.InitOrthographic( EGDefEdPreviewPanel::GetPanel()->GetAspectRatio() , EditorConfig->OrthoCamera.Height , EditorConfig->OrthoCamera.NearPlane , EditorConfig->OrthoCamera.FarPlane );
			MainDisplayList->SetViewTF( Cam.GetViewMat() );
			MainDisplayList->SetProjTF( Cam.GetProjMat() );
			MainDisplayList->SetVec4( eg_rv4_t::CAMERA_POS , eg_vec4(0.f,0.f,0.f,1.f) );
		}
		else
		{
			m_Camera.SetAspectRatio( EGDefEdPreviewPanel::GetPanel()->GetAspectRatio() );
			m_Camera.SetPose( m_CameraController.GetTransform() );
			MainDisplayList->SetViewTF( m_Camera.GetViewMat() );
			MainDisplayList->SetProjTF( m_Camera.GetProjMat() );
			MainDisplayList->SetVec4( eg_rv4_t::CAMERA_POS , m_Camera.GetPose().GetPosition() );
		}
		

		Draw_ApplyEditorConfig( EGDefEdFile::Get().GetBounds().GetCenter() );

		EGDefEdFile::Get().Draw();

		EGLogToScreen::Get().Draw( m_Camera.GetAspectRatio() );
	}

	EGRenderer::Get().EndFrame();


	EngineForTools_Draw();
}

void EGDefEdPreview::ResetCamera()
{
	eg_transform NewPose( CT_Default );

	eg_aabb Bounds = EGDefEdFile::Get().GetBounds();

	eg_vec4 LookAtPoint = Bounds.GetCenter();
	LookAtPoint.z = Bounds.Min.z;
	m_BigDim = EG_Max( Bounds.GetWidth() , Bounds.GetHeight() );

	if( m_BigDim < EG_REALLY_SMALL_NUMBER )
	{
		m_BigDim = 1.f;
	}

	// We want the object to take up approximately 1/3 of the center of the screen.
	// So we're computing the adjacent side of a triangle that represents half of
	// the view frustum triangle.
	eg_real DistanceAway = 1.5f * m_BigDim / EGMath_tan( EG_Deg(m_Camera.GetFovDeg()*.5f) );

	m_CameraController.Set( EG_Rad(0.f) , EG_Rad(0.f) , eg_vec3( 0.f , LookAtPoint.y , LookAtPoint.z - DistanceAway ) );
}

eg_bool EGDefEdPreview::CanMoveCamera() const
{
	egEntDefEdConfig* EditorConfig = EGDefEdFile::Get().GetConfig();
	return nullptr == EditorConfig || !EditorConfig->bOrthoPreview;
}

void EGDefEdPreview::Draw_Grid( const eg_real GridSpacing )
{
	const eg_int GridSize = 10;

	const eg_real Starting = -GridSpacing*GridSize*.5f;

	const eg_color LineColor(.5f,.5f,.5f,1.f);

	for( eg_int i=0; i<=GridSize; i++ )
	{	
		MainDisplayList->DrawLine( eg_vec4(Starting + i*GridSpacing , 0.f , Starting , 1.f ) , eg_vec4( Starting + i*GridSpacing , 0.f , Starting + GridSize*GridSpacing , 1.f ) , LineColor );
	}

	for( eg_int i=0; i<=GridSize; i++ )
	{	
		MainDisplayList->DrawLine( eg_vec4( Starting , 0.f , Starting + i*GridSpacing , 1.f ) , eg_vec4( Starting + GridSize*GridSpacing , 0.f , Starting + i*GridSpacing , 1.f ) , LineColor );
	}
}

void EGDefEdPreview::Draw_ApplyEditorConfig( const eg_vec4& ObjCenter )
{
	auto DrawLight = [this]( const EGLight& Light ) -> void
	{		
		eg_real LightRadius = EGMath_sqrt( Light.GetRangeSq() );
		eg_real ObjRadius = EG_Min( LightRadius*.5f , .25f );

		eg_transform Pose = eg_transform::BuildTranslation( Light.Pos.ToVec3() );
		eg_color Palette = Light.Color;

		m_DbLightObj->Update( 0.f );
		m_DbLightObj->DrawDebugShape( ObjRadius , Pose , Palette );

		Palette.a = .5f;
		MainDisplayList->PushRasterizerState( eg_rasterizer_s::CULL_NONE );
		MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF );
		m_DbLightObj->DrawDebugShape( LightRadius , Pose , Palette );
		MainDisplayList->PopDepthStencilState();
		MainDisplayList->PopRasterizerState();
	};

	egEntDefEdConfig* EditorConfig = EGDefEdFile::Get().GetConfig();

	if( EditorConfig )
	{
		MainDisplayList->ClearRT( eg_color(EditorConfig->BackgroundColor) );

		if( EditorConfig->bDrawGrid )
		{
			Draw_Grid( EditorConfig->GridSpacing );
		}

		EditorConfig->CameraPos = m_CameraController.GetPosition();
		EditorConfig->CameraYaw = m_CameraController.GetYaw();
		EditorConfig->CameraPitch = m_CameraController.GetPitch();
		EditorConfig->HasCameraPose = true;

		eg_int NumLights = EG_Min<eg_int>( RENDERER_MAX_LIGHTS , EditorConfig->Lights.LenAs<eg_int>() );

		for( eg_int i=0; i<NumLights; i++ )
		{
			EGLight Light = EditorConfig->Lights[i];
			Light.Dir = ObjCenter - Light.Pos;
			Light.Dir.w = 0.f;
			Light.Dir.NormalizeThisAsVec3();

			MainDisplayList->SetLight( i , Light );
			MainDisplayList->EnableLight( i , true );

			if( EditorConfig->bDrawLights )
			{
				DrawLight( Light );
			}
		}

		MainDisplayList->SetVec4( eg_rv4_t::AMBIENT_LIGHT , eg_color(EditorConfig->AmbientLight).ToVec4() );
		MainDisplayList->SetVec4( eg_rv4_t::GAME_PALETTE_0 , EditorConfig->GamePalette0 );
		MainDisplayList->SetVec4( eg_rv4_t::GAME_PALETTE_1 , EditorConfig->GamePalette1 );
		MainDisplayList->SetVec4( eg_rv4_t::GAME_PALETTE_2 , EditorConfig->GamePalette2 );
	}
}
