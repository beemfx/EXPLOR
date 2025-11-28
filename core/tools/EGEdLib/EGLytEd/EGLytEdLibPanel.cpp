// (c) 2016 Beem Media

#include "EGLytEdLibPanel.h"
#include "EGEntDict.h"
#include "EGLytEdPreviewPanel.h"
#include "EGResourceLib.h"

///////////////////////////////////////////////////////////////////////////////
//
// EGLibraryPanel
//
///////////////////////////////////////////////////////////////////////////////

EGLytEdLibPanel::EGLytEdLibPanel( EGWndPanel* Parent ) 
: EGWndScrollingPanel( Parent , EGWnd_GetAppScaledSize( 16 ) )
, m_bCapturing( false )
, m_DragCursor( nullptr )
, m_DragCantDropCursor( nullptr )
, m_RestoreCursor( nullptr )
{
	EntDict_GetDefStrings( m_Items , false , true );

	m_Items.Sort( []( const eg_string& lhs , const eg_string& rhs )->eg_bool { return EGString_CompareI( lhs , rhs ) < 0; } );

	m_DragCursor = LoadCursorW( EGResourceLib_GetLibrary() , L"EGLAYOUT_DRAG_CURSOR" );
	m_DragCantDropCursor = LoadCursorW( EGResourceLib_GetLibrary() , L"EGLAYOUT_DRAG_CURSOR_X" );

	UpdateScroll();
	//RedrawWindow( m_hwnd , nullptr , nullptr , RDW_INVALIDATE );
}


EGLytEdLibPanel::~EGLytEdLibPanel()
{
	
}

LRESULT EGLytEdLibPanel::WndProc( UINT Msg, WPARAM wParam, LPARAM lParam )
{
	switch( Msg )
	{
	case WM_VSCROLL:
	{
		ScrollProc( LOWORD(wParam) );
	} break;
	case WM_SIZING:
	case WM_SIZE:
	{
		UpdateScroll();
	} break;
	case WM_LBUTTONDOWN:
	{
		if( EG_IsBetween<eg_size_t>( m_HoveredItem , 0 , m_Items.Len()-1 ) )
		{
			SetCapture( m_hwnd );
			m_RestoreCursor = SetCursor( m_DragCantDropCursor );
			m_bCapturing = true;
		}
	} break;
	case WM_LBUTTONUP:
	{
		if( m_bCapturing )
		{
			m_bCapturing = false;
			if( m_RestoreCursor )
			{
				SetCursor( m_RestoreCursor );
			}

			if( EGLytEdPreviewPanel::GetPanel() && m_Items.IsValidIndex(m_HoveredItem) )
			{
				EGLytEdPreviewPanel::GetPanel()->DropLibraryItem( m_Items[m_HoveredItem] );
			}

			ReleaseCapture();
		}
	} break;
	case WM_CANCELMODE:
	{
		if( m_bCapturing )
		{
			m_bCapturing = false;
			if( m_RestoreCursor )
			{
				SetCursor( m_RestoreCursor );
			}
			ReleaseCapture();
		} 
	} break;
	case WM_MOUSEMOVE:
	{
		//SetFocus( m_hwnd );

		TrackMouseLeave();

		if( !m_bCapturing )
		{
			eg_int xPos = GET_X_LPARAM(lParam); 
			eg_int yPos = GET_Y_LPARAM(lParam); 

			SetHoveredItem( ClientPosToItemIndex( eg_ivec2(xPos,yPos) ) );
		}

		if( m_bCapturing )
		{
			if( EGLytEdPreviewPanel::GetPanel() )
			{
				SetCursor( EGLytEdPreviewPanel::GetPanel()->IsMouseOverPanel() ? m_DragCursor : m_DragCantDropCursor );
			}
		}
	} break;
	case WM_MOUSEWHEEL:
	{
		eg_int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		ScrollProc( zDelta < 0 ? SB_LINEDOWN : SB_LINEUP );
	} break;
	}
	return Super::WndProc( Msg , wParam , lParam );
}

void EGLytEdLibPanel::OnPaint( HDC hdc )
{
	const eg_int ItemSize = GetItemSize();

	RECT rc = GetViewRect();
	rc.top -= m_ScrollPos;

	SetBkMode( hdc , TRANSPARENT );

	auto DrawTextAndAdvanceY = [&hdc,&rc,this,&ItemSize]( eg_cpstr Str , DWORD Format , eg_bool bSelected ) -> void
	{
		SetTextColor( hdc , EGWnd_GetColor( bSelected ? egw_color_t::FG_EDIT : egw_color_t::FG_STATIC ) );
		SetFont( hdc , bSelected ? m_SelectedItemFont : m_ItemFont );

		if( bSelected )
		{
			SelectObject( hdc , static_cast<HGDIOBJ>(EGWnd_GetBrush( egw_color_t::BG_EDIT ) ));
			Rectangle( hdc , rc.left , rc.top , rc.right , rc.top + ItemSize); 
		}
		
		RECT rcLineSize = rc;
		rcLineSize.left += 5;
		rcLineSize.right -= 5;
		DrawTextA( hdc , Str , -1 , &rcLineSize , Format|DT_SINGLELINE );
		rc.top += ItemSize;
	};

	SetFont( hdc , m_ItemFont );
	for( eg_size_t i=0; i<m_Items.Len(); i++ )
	{
		DrawTextAndAdvanceY( m_Items[i] , 0 , i==m_HoveredItem );
	}
}

void EGLytEdLibPanel::OnDrawBg( HDC hdc )
{
	FillRect( hdc , &GetViewRect() , EGWnd_GetBrush( egw_color_t::BG_STATIC ) );
}

void EGLytEdLibPanel::OnWmMouseLeave()
{
	Super::OnWmMouseLeave();

	SetHoveredItem( -1 );
}
