// (c) 2018 Beem Media

#include "EGWorldEdPreviewPanel.h"
#include "EGWorldEd.h"
#include "EGRenderer.h"
#include "EGDisplayList.h"
#include "EGRendererInterface.h"
#include "EGRenderThread.h"
#include "EGDebugText.h"
#include "EGRenderThread.h"
#include "EGEngineForTools.h"
#include "EGComponent.h"
#include "EGCameraController.h"

EGWorldEdPreviewPanel::EGWorldEdPreviewPanel( EGWndPanel* Parent )
: EGWndPanel( Parent , egWndPanelCustomLayout() )
{
	EngineForTools_InitRenderer( reinterpret_cast<eg_uintptr_t>(m_hwnd) );

	// EGDefEdPreview::Get().Init();

	EnableDoubleBuffering( false );
}

EGWorldEdPreviewPanel::~EGWorldEdPreviewPanel()
{
	// EGDefEdPreview::Get().Deinit();
}

void EGWorldEdPreviewPanel::OnPaint( HDC hdc )
{
	unused( hdc );
}

void EGWorldEdPreviewPanel::OnDrawBg( HDC hdc )
{
	unused( hdc );
}

void EGWorldEdPreviewPanel::OnWmLButtonDown( const eg_ivec2& MousePos )
{
	Super::OnWmLButtonDown( MousePos );
}

void EGWorldEdPreviewPanel::OnWmLButtonUp( const eg_ivec2& MousePos )
{
	Super::OnWmLButtonUp( MousePos );
}

void EGWorldEdPreviewPanel::OnWmRButtonDown( const eg_ivec2& MousePos )
{
	Super::OnWmRButtonDown( MousePos );

	if( !IsCapturing() )
	{
		/*
		m_CaptureObj = GetCaptureObj( MousePos );
		if( m_CaptureObj )
		{
			m_CaptureObjIntialPos = m_CaptureObj->GetBasePose();	
		}
		*/
		ShowCursor( FALSE );
		m_CaptureReason = eg_capture_reason::MoveCamera;
		BeginCapture( MousePos );
	}
}

void EGWorldEdPreviewPanel::OnWmRButtonUp( const eg_ivec2& MousePos )
{
	Super::OnWmRButtonUp( MousePos );

	if( m_CaptureReason == eg_capture_reason::MoveCamera )
	{
		EndCapture( MousePos );
	}
}

void EGWorldEdPreviewPanel::OnWmMouseMove( const eg_ivec2& _MousePos )
{
	Super::OnWmMouseMove( _MousePos );

	if( m_CaptureReason == eg_capture_reason::MoveCamera )
	{
		// Don't let the mouse cursor actually move.
		eg_ivec2 MoveAmount(0,0);

		{
			eg_ivec2 StartPos = GetBeginCapturePos();
			MoveAmount = _MousePos - StartPos;
			POINT Cursor = { StartPos.x , StartPos.y };
			ClientToScreen( GetWnd() , &Cursor );
			SetCursorPos( Cursor.x , Cursor.y );
		}

		const eg_real RotationScale = .003f;

		// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "Force {0} " , EGVec3Formatter(MoveAmount) ) );

		m_LastMouseInput.x += MoveAmount.x*RotationScale;
		m_LastMouseInput.y += MoveAmount.y*RotationScale;
	}
}

void EGWorldEdPreviewPanel::OnWmSize( const eg_ivec2& NewClientSize )
{
	Super::OnWmSize( NewClientSize );

	EngineForTools_OnToolWindowResized( NewClientSize.x , NewClientSize.y );
}

void EGWorldEdPreviewPanel::OnCaptureLost()
{
	Super::OnCaptureLost();
	OnEndCapture( eg_ivec2(0,0) );
}

void EGWorldEdPreviewPanel::OnEndCapture( const eg_ivec2& MousePos )
{
	Super::OnEndCapture( MousePos );

	if( m_CaptureReason == eg_capture_reason::MoveCamera )
	{
		ShowCursor( TRUE );
	}
	// m_CaptureObj = nullptr;
	m_CaptureReason = eg_capture_reason::None;
}

RECT EGWorldEdPreviewPanel::GetCustomWindowRect() const
{
	// Do we favor width or height?

	RECT rcParent;
	GetClientRect( GetParent(m_hwnd) , &rcParent );
	return rcParent;

	/*
	eg_real RealHeight = static_cast<eg_real>(rcParent.bottom - PREVIEW_PADDING*2);
	eg_real RealWidth = RealHeight*Ratio;

	if( RealWidth > (rcParent.right-PREVIEW_PADDING*2) )
	{
	RealWidth = static_cast<eg_real>(rcParent.right-PREVIEW_PADDING*2);
	RealHeight = RealWidth/Ratio;
	}

	eg_int Width = static_cast<eg_int>(RealWidth);
	eg_int Height = static_cast<eg_int>(RealHeight);

	eg_int Left = (rcParent.right/2) - (Width/2);
	eg_int Top = (rcParent.bottom/2) - (Height/2);

	RECT Out;
	SetRect( &Out , Left , Top , Left + Width , Top + Height );
	return Out;
	*/
}

eg_real EGWorldEdPreviewPanel::GetAspectRatio() const
{
	RECT rc = GetViewRect();

	return static_cast<eg_real>(rc.right - rc.left)/static_cast<eg_real>(rc.bottom-rc.top);
}

void EGWorldEdPreviewPanel::SetCameraPose( const eg_transform& NewCameraPose )
{
	m_CameraController.SetPose( NewCameraPose );
	EGWorldEd_SetCameraPose( m_CameraController.GetTransform() );
}

void EGWorldEdPreviewPanel::HandleAppUpdate( eg_real DeltaTime )
{
	m_MouseSmoother.SetMaxSamples( 4 );
	m_MouseSmoother.Update( m_LastMouseInput , DeltaTime );
	const eg_vec3 MouseMovement = m_MouseSmoother.GetSmoothValue();
	m_LastMouseInput = CT_Clear;

	m_MoveSmoother.SetMaxSamples( 8 );
	m_MoveSmoother.Update( m_LastMoveForce , DeltaTime );
	const eg_vec3 Movement = m_MoveSmoother.GetSmoothValue();
	m_LastMoveForce = CT_Clear;

	EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "Mouse Power {0} " , EGVec3Formatter(m_MouseSmoother.GetSmoothValue()) ) );
	
	if( m_CaptureReason == eg_capture_reason::MoveCamera )
	{
		const eg_bool bControlHeld = (GetAsyncKeyState( VK_LCONTROL )&0x8000) != 0;
		const eg_bool bShiftHeld = (GetAsyncKeyState( VK_LSHIFT )&0x8000) != 0;
		const eg_bool bLMBHeld = (GetAsyncKeyState( VK_LBUTTON )&0x8000) != 0;

		const eg_bool bForwardHeld = (GetAsyncKeyState( 'W' )&0x8000) != 0;
		const eg_bool bBackHeld = (GetAsyncKeyState( 'S' )&0x8000) != 0;
		const eg_bool bLeftHeld = (GetAsyncKeyState( 'A' )&0x8000) != 0;
		const eg_bool bRightHeld = (GetAsyncKeyState( 'D' )&0x8000) != 0;
		const eg_bool bSpaceHeld = (GetAsyncKeyState( ' ' )&0x8000) != 0;

		eg_vec3 MoveVec( CT_Clear );
		if( bForwardHeld )
		{
			MoveVec.z += 1.f;
		}
		if( bBackHeld )
		{
			MoveVec.z += -1.f;
		}
		if( bRightHeld )
		{
			MoveVec.x += 1.f;
		}
		if( bLeftHeld )
		{
			MoveVec.x += -1.f;
		}
		if( bSpaceHeld )
		{
			MoveVec.y += 1.f;
		}
		if( bControlHeld )
		{
			MoveVec.y += -1.f;
		}

		eg_real MoveSpeed = 10.f;

		MoveVec *= MoveSpeed;

		if( bShiftHeld )
		{
			MoveVec *= 2.f;
		}

		MoveVec *= DeltaTime;

		m_LastMoveForce = MoveVec;

		// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "Force {0} " , EGVec3Formatter(MoveVec) ) );

		m_CameraController.MoveLookDir( Movement.z );
		m_CameraController.MoveStrafe( Movement.x );
		m_CameraController.MoveUp( Movement.y );

		m_CameraController.MoveYaw( EG_Rad(MouseMovement.x) );
		m_CameraController.MovePitch( EG_Rad(MouseMovement.y) );

		EGWorldEd_SetCameraPose( m_CameraController.GetTransform() );
	}
}
