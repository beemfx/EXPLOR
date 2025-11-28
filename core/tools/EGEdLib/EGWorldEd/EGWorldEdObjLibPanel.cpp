// (c) 2018 Beem Media

#include "EGWorldEdObjLibPanel.h"
#include "EGWorldEd.h"
#include "EGWorldEdPreviewPanel.h"
#include "EGWorldEdWorldObjsPanel.h"
#include "EGWorldObject.h"
#include "EGAlwaysLoaded.h"
#include "EGResourceLib.h"

EGWorldEdObjLibPanel::egLibItem::egLibItem( EGClass* InClass ) 
: Class( InClass )
, DisplayName( CT_Clear )
{
	DisplayName = *EGWorldObject::GetShortNameFromClass( Class );
}

EGWorldEdObjLibPanel::EGWorldEdObjLibPanel( EGWndPanel* Parent ) 
: EGWndScrollingPanel( Parent , EGWnd_GetAppScaledSize( 16 ) )
, m_bCapturing( false )
, m_DragCursor( nullptr )
, m_DragCantDropCursor( nullptr )
, m_RestoreCursor( nullptr )
{
	EGArray<EGClass*> ObjectClasses;
	EGClass::FindAllClassesOfType( &EGWorldObject::GetStaticClass() , ObjectClasses );

	for( EGClass* ObjectClass : ObjectClasses )
	{
		m_Items.Append( ObjectClass );
	}

	m_Items.Sort( []( const egLibItem& lhs , const egLibItem& rhs )->eg_bool { return EGString_CompareI( lhs.DisplayName , rhs.DisplayName ) < 0; } );

	m_DragCursor = LoadCursorW( EGResourceLib_GetLibrary() , L"EGDEFED_DRAG_CURSOR" );
	m_DragCantDropCursor = LoadCursorW( EGResourceLib_GetLibrary() , L"EGDEFED_DRAG_CURSOR_X" );

	UpdateScroll();
	//RedrawWindow( m_hwnd , nullptr , nullptr , RDW_INVALIDATE );
}

EGWorldEdObjLibPanel::~EGWorldEdObjLibPanel()
{

}

LRESULT EGWorldEdObjLibPanel::WndProc( UINT Msg, WPARAM wParam, LPARAM lParam )
{
	switch( Msg )
	{
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

			if( m_Items.IsValidIndex( m_HoveredItem ) )
			{
				const eg_bool bIsMouseOverPreviewPanel = EGWorldEd_GetPreviewPanel().IsMouseOverPanel();
				const eg_bool bIsMouseOverWorldObjsPanel = EGWorldEd_GetWorldObjsPanel().IsMouseOverPanel();

				if( bIsMouseOverWorldObjsPanel || bIsMouseOverPreviewPanel )
				{
					EGWorldEd_InsertNewWorldObject( m_Items[m_HoveredItem].Class );
				}
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
			const eg_bool bIsMouseOverPreviewPanel = EGWorldEd_GetPreviewPanel().IsMouseOverPanel();
			const eg_bool bIsMouseOverWorldObjsPanel = EGWorldEd_GetWorldObjsPanel().IsMouseOverPanel();

			SetCursor( (bIsMouseOverWorldObjsPanel || bIsMouseOverPreviewPanel) ? m_DragCursor : m_DragCantDropCursor );
		}
	} break;
	}
	return Super::WndProc( Msg , wParam , lParam );
}

void EGWorldEdObjLibPanel::OnPaint( HDC hdc )
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
		DrawTextAndAdvanceY( m_Items[i].DisplayName , 0 , i==m_HoveredItem );
	}
}

void EGWorldEdObjLibPanel::OnDrawBg( HDC hdc )
{
	FillRect( hdc , &GetViewRect() , EGWnd_GetBrush( egw_color_t::BG_STATIC ) );
}

void EGWorldEdObjLibPanel::OnWmMouseLeave()
{
	Super::OnWmMouseLeave();

	SetHoveredItem( -1 );
}


