// (c) 2017 Beem Media

#include "EGDefEdPreviewPanel.h"
#include "EGRenderer.h"
#include "EGDisplayList.h"
#include "EGRendererInterface.h"
#include "EGRenderThread.h"
#include "EGDebugText.h"
#include "EGRenderThread.h"
#include "EGEngineForTools.h"
#include "EGDefEdFile.h"
#include "EGDefEd.h"
#include "EGDefEdPropPanel.h"
#include "EGDefEdComponentsPanel.h"
#include "EGDefEdPreview.h"
#include "EGComponent.h"

EGDefEdPreviewPanel* EGDefEdPreviewPanel::GlobalPreviewPanel = nullptr;

EGDefEdPreviewPanel::EGDefEdPreviewPanel( EGWndPanel* Parent )
: EGWndPanel( Parent , egWndPanelCustomLayout() )
{
	assert( nullptr == GlobalPreviewPanel ); // Only one preview panel should exist.
	GlobalPreviewPanel = this;
	EngineForTools_InitRenderer( reinterpret_cast<eg_uintptr_t>(m_hwnd) );

	EGDefEdPreview::Get().Init();
	
	EnableDoubleBuffering( false );
}

EGDefEdPreviewPanel::~EGDefEdPreviewPanel()
{
	EGDefEdPreview::Get().Deinit();

	assert( this == GlobalPreviewPanel ); // Global preview panel should have been set.
	GlobalPreviewPanel = nullptr;
}

void EGDefEdPreviewPanel::OnPaint( HDC hdc )
{
	unused( hdc );

	EGDefEdPreview::Get().Draw();
}

void EGDefEdPreviewPanel::OnDrawBg( HDC hdc )
{
	unused( hdc );
}

void EGDefEdPreviewPanel::OnWmLButtonDown( const eg_ivec2& MousePos )
{
	Super::OnWmLButtonDown( MousePos );
}

void EGDefEdPreviewPanel::OnWmLButtonUp( const eg_ivec2& MousePos )
{
	Super::OnWmLButtonUp( MousePos );
}

void EGDefEdPreviewPanel::OnWmRButtonDown( const eg_ivec2& MousePos )
{
	Super::OnWmRButtonDown( MousePos );

	if( !IsCapturing() )
	{
		m_CaptureObj = GetCaptureObj( MousePos );
		if( m_CaptureObj )
		{
			m_CaptureObjIntialPos = m_CaptureObj->GetBasePose();	
		}
		ShowCursor( FALSE );
		m_CaptureReason = eg_capture_reason::MoveCamera;
		BeginCapture( MousePos );
	}
}

void EGDefEdPreviewPanel::OnWmRButtonUp( const eg_ivec2& MousePos )
{
	Super::OnWmRButtonUp( MousePos );

	if( m_CaptureReason == eg_capture_reason::MoveCamera )
	{
		EndCapture( MousePos );
	}
}

void EGDefEdPreviewPanel::OnWmMouseMove( const eg_ivec2& _MousePos )
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

void EGDefEdPreviewPanel::OnWmSize( const eg_ivec2& NewClientSize )
{
	Super::OnWmSize( NewClientSize );

	EngineForTools_OnToolWindowResized( NewClientSize.x , NewClientSize.y );
}

void EGDefEdPreviewPanel::OnCaptureLost()
{
	Super::OnCaptureLost();
	OnEndCapture( eg_ivec2(0,0) );
}

void EGDefEdPreviewPanel::OnEndCapture( const eg_ivec2& MousePos )
{
	Super::OnEndCapture( MousePos );

	if( m_CaptureReason == eg_capture_reason::MoveCamera )
	{
		ShowCursor( TRUE );
	}
	m_CaptureObj = nullptr;
	m_CaptureReason = eg_capture_reason::None;
}

void EGDefEdPreviewPanel::HandleAppUpdate( eg_real DeltaTime )
{
	m_MouseSmoother.SetMaxSamples( 4 );
	m_MouseSmoother.Update( m_LastMouseInput , DeltaTime );
	const eg_vec3 MouseMovement = m_MouseSmoother.GetSmoothValue();
	m_LastMouseInput = CT_Clear;

	m_MoveSmoother.SetMaxSamples( 8 );
	m_MoveSmoother.Update( m_LastMoveForce , DeltaTime );
	const eg_vec3 Movement = m_MoveSmoother.GetSmoothValue();
	m_LastMoveForce = CT_Clear;
	
	EGCameraController& CameraController = EGDefEdPreview::Get().GetCameraController();

	if( m_CaptureReason == eg_capture_reason::MoveCamera && EGDefEdPreview::Get().CanMoveCamera() )
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

		eg_real MoveSpeed = 1.f*EGDefEdPreview::Get().GetBigDim();

		MoveVec *= MoveSpeed;

		if( bShiftHeld )
		{
			MoveVec *= 2.f;
		}

		MoveVec *= DeltaTime;

		m_LastMoveForce = MoveVec;

		// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "Force {0} " , EGVec3Formatter(MoveVec) ) );

		CameraController.MoveLookDir( Movement.z );
		CameraController.MoveStrafe( Movement.x );
		CameraController.MoveUp( Movement.y );

		CameraController.MoveYaw( EG_Rad(MouseMovement.x) );
		CameraController.MovePitch( EG_Rad(MouseMovement.y) );
	}
}

RECT EGDefEdPreviewPanel::GetCustomWindowRect() const
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

EGComponent* EGDefEdPreviewPanel::GetCaptureObj( const eg_ivec2& MousePos )
{
	EGComponent* Out = EGDefEdFile::Get().GetComponentByMousePos( static_cast<eg_real>(MousePos.x) , static_cast<eg_real>(MousePos.y) );

	if( Out )
	{
		EGDefEdComponentsPanel::GetPanel()->SetSelectedComponentByDef( Out );
	}
	else
	{
		Out = EGDefEdComponentsPanel::GetPanel()->GetEditComponent();
	}

	return Out;
}

eg_real EGDefEdPreviewPanel::GetAspectRatio() const
{
	RECT rc = GetViewRect();

	return static_cast<eg_real>(rc.right - rc.left)/static_cast<eg_real>(rc.bottom-rc.top);
}
