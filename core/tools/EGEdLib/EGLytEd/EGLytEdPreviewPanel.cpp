// (c) 2016 Beem Media

#include "EGLytEdPreviewPanel.h"
#include "EGRenderer.h"
#include "EGDisplayList.h"
#include "EGRendererInterface.h"
#include "EGRenderThread.h"
#include "EGDebugText.h"
#include "EGRenderThread.h"
#include "EGEngineForTools.h"
#include "EGLytEdFile.h"
#include "EGLytEd.h"
#include "EGLytEdPropPanel.h"
#include "EGLytEdLayoutPanel.h"
#include "EGUiLayout.h"
#include "EGUiWidgetInfo.h"

EGLytEdPreviewPanel* EGLytEdPreviewPanel::GlobalPreviewPanel = nullptr;

EGLytEdPreviewPanel::EGLytEdPreviewPanel( EGWndPanel* Parent )
: EGWndPanel( Parent , egWndPanelCustomLayout() )
, m_bCaptureIsRMB( false )
, m_bCaptureIsMMB( false )
{
	assert( nullptr == GlobalPreviewPanel ); // Only one preview panel should exist.
	GlobalPreviewPanel = this;
	EngineForTools_InitRenderer( reinterpret_cast<eg_uintptr_t>(m_hwnd) );
	
	EnableDoubleBuffering( false );
}

EGLytEdPreviewPanel::~EGLytEdPreviewPanel()
{
	assert( this == GlobalPreviewPanel ); // Global preview panel should have been set.
	GlobalPreviewPanel = nullptr;
}

LRESULT EGLytEdPreviewPanel::WndProc( UINT Msg, WPARAM wParam, LPARAM lParam )
{
	switch( Msg )
	{
	case WM_MOUSEMOVE:
	{
		eg_int xPos = GET_X_LPARAM(lParam); 
		eg_int yPos = GET_Y_LPARAM(lParam);

		eg_vec2 MousePos = ClientSpaceToMenuSpace( xPos , yPos , GetViewRect() );

		if( m_MouseCapture.IsCapturing() )
		{
			POINT BeginDrag = m_MouseCapture.GetBeginCaptureMousePos();
			eg_vec2 BeginDragPos = ClientSpaceToMenuSpace( BeginDrag.x , BeginDrag.y , GetViewRect() );
			eg_vec2 MoveAmount = MousePos - BeginDragPos;

			eg_bool bCanDrag = m_CaptureObj && (m_CaptureObj->Type == egUiWidgetInfo::eg_obj_t::IMAGE || m_CaptureObj->Type == egUiWidgetInfo::eg_obj_t::MESH || m_CaptureObj->Type == egUiWidgetInfo::eg_obj_t::TEXT_NODE || m_CaptureObj->Type == egUiWidgetInfo::eg_obj_t::LIGHT);

			if( m_CaptureObj && m_CaptureObj->bIsLocked )
			{
				bCanDrag = false;
			}

			if( bCanDrag )
			{
				if( m_bCaptureIsRMB || m_bCaptureIsMMB )
				{
					eg_transform MouseRotTransform;
					MouseRotTransform = CT_Default;
					if( m_bCaptureIsMMB )
					{
						MouseRotTransform.RotateZThis( EG_Rad(-MoveAmount.x/2.f) );
						MouseRotTransform.RotateZThis( EG_Rad(-MoveAmount.y/2.f) );
					}
					else
					{
						MouseRotTransform.RotateYThis( EG_Rad(MoveAmount.x/2.f) );
						MouseRotTransform.RotateXThis( EG_Rad(MoveAmount.y/2.f) );
					}
					m_CaptureObj->BasePose.SetRotation( m_CaptureObjIntialPos.GetRotation() * MouseRotTransform.GetRotation() );
					m_CaptureObj->BasePose.NormalizeRotationOfThis();
					EGLytEdPropPanel::GetPanel()->CommitChangesFromPreviewPanel();
					EGLytEd_SetDirty();
				}
				else
				{
					MoveAmount.x *= MENU_ORTHO_SIZE;
					MoveAmount.y *= MENU_ORTHO_SIZE;
					//DebugText_MsgOverlay_AddText(DEBUG_TEXT_OVERLAY_CLIENT , EGString_Format( "Mouse %g,%g" , MoveAmount.x , MoveAmount.y ) );

					eg_vec3 NewTranslation = m_CaptureObj->BasePose.GetTranslation();
					NewTranslation.x = m_CaptureObjIntialPos.GetTranslation().x + MoveAmount.x;
					NewTranslation.y = m_CaptureObjIntialPos.GetTranslation().y + MoveAmount.y;
					m_CaptureObj->BasePose.SetTranslation( NewTranslation );
					EGLytEdPropPanel::GetPanel()->CommitChangesFromPreviewPanel();
					EGLytEd_SetDirty();
				}
			}
		}
		else
		{
			//DebugText_MsgOverlay_AddText(DEBUG_TEXT_OVERLAY_CLIENT , EGString_Format( "Mouse %g,%g" , MousePos.x , MousePos.y ) );
			EGLytEdFile::Get().SetMousePos( MousePos.x , MousePos.y );
		}
	} break;
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	{
		if( !m_MouseCapture.IsCapturing() )
		{
			eg_int xPos = GET_X_LPARAM(lParam); 
			eg_int yPos = GET_Y_LPARAM(lParam);

			eg_vec2 MousePos = ClientSpaceToMenuSpace( xPos , yPos , GetViewRect() );

			m_CaptureObj = EGLytEdFile::Get().GetObjByMousePos( MousePos.x , MousePos.y );

			if( m_CaptureObj )
			{
				EGLytEdLayoutPanel::GetPanel()->SetSelectedObjByObjInfo( m_CaptureObj );
			}
			else
			{
				m_CaptureObj = EGLytEdLayoutPanel::GetPanel()->GetEditObject();
			}

			m_MouseCapture.OnBeginCapture( GetWnd() , GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) );
			if( m_CaptureObj )
			{
				m_CaptureObjIntialPos = m_CaptureObj->BasePose;	
			}

			m_bCaptureIsRMB = (WM_RBUTTONDOWN == Msg);
			m_bCaptureIsMMB = (WM_MBUTTONDOWN == Msg);
		}
	} break;
	case WM_RBUTTONUP:
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	{
		m_MouseCapture.OnEndCapture();
		m_CaptureObj = nullptr;
		m_bCaptureIsRMB = false;
		m_bCaptureIsMMB = false;
	} break;
	case WM_CANCELMODE:
	{
		m_MouseCapture.OnCancelMode();
		m_CaptureObj = nullptr;
		m_bCaptureIsRMB = false;
		m_bCaptureIsMMB = false;
	} break;
	case WM_SIZE:
	{
		eg_uint Width = LOWORD(lParam);
		eg_uint Height = HIWORD(lParam);
		EngineForTools_OnToolWindowResized( Width , Height );
	} break;
	}
	return Super::WndProc( Msg , wParam , lParam );
}

void EGLytEdPreviewPanel::OnPaint( HDC hdc )
{
	unused( hdc );

	EGLytEd_DrawPreview();
}

void EGLytEdPreviewPanel::OnDrawBg( HDC hdc )
{
	unused( hdc );
}

void EGLytEdPreviewPanel::SetPreviewPanelAspectRatio( eg_real AspectRatio )
{
	m_PreviewPanelAspectRatio = AspectRatio;
	HandleResize();
}

eg_vec2 EGLytEdPreviewPanel::ClientSpaceToMenuSpace( eg_int xPos , eg_int yPos , const RECT& rcClient )
{
	eg_vec2 Out(0.f,0.f);

	eg_real NormX = static_cast<eg_real>(xPos)/rcClient.right;
	eg_real NormY = static_cast<eg_real>(yPos)/rcClient.bottom;

	Out.x = NormX*2.f - 1.f;
	Out.y = (1.f - NormY)*2.f - 1.f;
	Out.x *= EGLytEdFile::Get().GetPreviewAspectRatio();

	return Out;
}

void EGLytEdPreviewPanel::DropLibraryItem( eg_cpstr StrDefId )
{
	POINT CursorPos;
	if( IsMouseOverPanel( &CursorPos ) )
	{
		eg_vec2 DropPos = ClientSpaceToMenuSpace( CursorPos.x , CursorPos.y , GetViewRect() );
		EGLytEdFile::Get().InsertObject( StrDefId , DropPos.x , DropPos.y );
	}
}

RECT EGLytEdPreviewPanel::GetCustomWindowRect() const
{
	// Do we favor width or height?
	eg_real Ratio = m_PreviewPanelAspectRatio;

	RECT rcParent;
	GetClientRect( GetParent(m_hwnd) , &rcParent );

	eg_real RealHeight = static_cast<eg_real>(rcParent.bottom - GetPreviewPadding()*2);
	eg_real RealWidth = RealHeight*Ratio;

	if( RealWidth > (rcParent.right-GetPreviewPadding()*2) )
	{
		RealWidth = static_cast<eg_real>(rcParent.right-GetPreviewPadding()*2);
		RealHeight = RealWidth/Ratio;
	}

	eg_int Width = static_cast<eg_int>(RealWidth);
	eg_int Height = static_cast<eg_int>(RealHeight);

	eg_int Left = (rcParent.right/2) - (Width/2);
	eg_int Top = (rcParent.bottom/2) - (Height/2);

	RECT Out;
	SetRect( &Out , Left , Top , Left + Width , Top + Height );
	return Out;
}
