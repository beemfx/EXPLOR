// (c) 2017 Beem Media

#include "EGWndDragableLayoutEditor.h"

EGWndDragableLayoutEditor::EGWndDragableLayoutEditor( EGWndPanel* Parent ) 
: EGWndScrollingPanel( Parent , EGWnd_GetFontSize( egw_font_t::DEFAULT ) + EGWnd_GetAppScaledSize( 2 )  )
, m_SelectedIndex( -1 )
, m_RestoreCursor( nullptr )
, m_InsertIndex( -1 )
, m_ItemBeingMovedIndex( -1 )
{
	SetupDefaultCursors();
	m_InsertBrush = CreateSolidBrush( RGB(0,168,0) );
}


EGWndDragableLayoutEditor::~EGWndDragableLayoutEditor()
{
	DeleteObject( m_InsertBrush );
}

void EGWndDragableLayoutEditor::OnPreDrawItems( HDC hdc )
{
	SetBkMode( hdc , TRANSPARENT );
	SetFont( hdc , m_ItemFont );
}

void EGWndDragableLayoutEditor::OnDrawItem( HDC hdc , eg_int ItemIndex , const RECT& rc , eg_bool bIsBeingDroppedOn , eg_bool bIsHovered , eg_bool bIsSelected ) const
{
	unused( ItemIndex );

	SetTextColor( hdc , EGWnd_GetColor( bIsHovered || bIsSelected ? egw_color_t::FG_EDIT : egw_color_t::FG_STATIC ) );
	SetFont( hdc , bIsSelected ? m_SelectedItemFont : m_ItemFont );

	if( bIsBeingDroppedOn )
	{
		SetBrush( hdc , m_InsertBrush );
		Rectangle( hdc , rc.left , rc.top , rc.right , rc.bottom );
	}
	else if( bIsSelected )
	{
		SetBrush( hdc , EGWnd_GetBrush( egw_color_t::BG_EDIT ) );
		Rectangle( hdc , rc.left , rc.top , rc.right , rc.bottom );
	}
	else if( bIsHovered ) // Selected prioritized over highlighted
	{
		SetBrush( hdc , EGWnd_GetBrush( egw_color_t::BG_STATIC ) );
		Rectangle( hdc , rc.left , rc.top , rc.right , rc.bottom );
	}
	else
	{
		// SetBrush( hdc , EGWnd_GetBrush(egw_color_t::BG_STATIC) );
		// Rectangle( hdc , rc.left , rc.top , rc.right , rc.bottom );
	}
}

void EGWndDragableLayoutEditor::OnPostDrawItems( HDC hdc )
{
	unused( hdc );
}

void EGWndDragableLayoutEditor::OnPaint( HDC hdc )
{
	const eg_int ItemSize = GetItemSize();
	const eg_int NumItems = static_cast<eg_int>(GetNumItems());

	eg_uint LastDepth = 0;
	
	OnPreDrawItems( hdc );	

	for( eg_int i=0; i<NumItems; i++ )
	{
		RECT rcItem = GetDrawRectForItem( i );
		eg_bool bIsBeingDroppedOn = m_InsertMode == eg_insert_m::AsChild && m_InsertIndex == i && m_ItemBeingMovedIndex != m_InsertIndex;

		OnDrawItem( hdc , i , rcItem , bIsBeingDroppedOn , i == m_HoveredItem , i == m_SelectedIndex );
	}

	OnPostDrawItems( hdc );

	if( IsCapturing() )
	{
		eg_int InsertBeforeIndex = m_InsertIndex;
		if( m_InsertMode == eg_insert_m::After )
		{
			InsertBeforeIndex++;
		}

		if( 0 <= InsertBeforeIndex && InsertBeforeIndex <= NumItems && m_InsertMode != eg_insert_m::AsChild )
		{
			if( m_bHasMouseMovedSinceDragStart )
			{
				// if( InsertBeforeIndex != m_SelectedIndex && (m_SelectedIndex+1) != InsertBeforeIndex )
				{
					RECT rcInsertIndicator = GetInsertBetweenRectForIndex( InsertBeforeIndex );
					FillRect( hdc , &rcInsertIndicator , m_InsertBrush );
				}
			}
		}
	}
}

void EGWndDragableLayoutEditor::OnDrawBg( HDC hdc )
{
	FillRect( hdc , &GetViewRect() , EGWnd_GetBrush( egw_color_t::BG_STATIC ) );
}

void EGWndDragableLayoutEditor::OnWmMouseMove( const eg_ivec2& MousePos )
{
	Super::OnWmMouseMove( MousePos );

	const eg_int NumItems = static_cast<eg_int>(GetNumItems());

	TrackMouseLeave();

	if( !IsCapturing() )
	{
		eg_int NewHoveredItem = ClientPosToItemIndex( MousePos );
		SetHoveredItem( NewHoveredItem );
	}

	if( IsCapturing() )
	{
		m_bHasMouseMovedSinceDragStart = true;

		if( IsMouseOverPanel() )
		{
			eg_int OverIndex = ClientPosToItemIndex( MousePos );
			eg_real Pct = ClientPosToPct( OverIndex , MousePos );
			eg_insert_m InsertMode = eg_insert_m::Before;
			
			if( m_DragType == eg_drag_t::BetweenAndOnto )
			{
				if( Pct > .25f )
				{
					InsertMode = eg_insert_m::AsChild;
				}
				if( Pct > .75f )
				{
					InsertMode = eg_insert_m::After;
				}
			}
			else if( m_DragType == eg_drag_t::Between )
			{
				if( Pct > .5f )
				{
					InsertMode = eg_insert_m::After;
				}
			}

			if( 0 <= OverIndex && OverIndex < NumItems )
			{
				SetInsertIndex( OverIndex , InsertMode );

				// We wont' change the cursor unless the y position of the mouse actually changed.
				if( m_MousePointAtStartOfDrag.y != MousePos.y )
				{
					// EGLogf( eg_log_t::General , "Index %u Percent %g" , OverIndex , Pct );
					if( InsertMode == eg_insert_m::AsChild && m_ItemBeingMovedIndex != OverIndex )
					{
						SetCursor( m_DragMakeChildCursor );
					}
					else
					{
						SetCursor( m_DragCursor );
					}
				}
			}
			else
			{
				SetInsertIndex( NumItems , eg_insert_m::Before );
				SetCursor( m_DragCursor );
			}
		}
		else
		{
			SetInsertIndex( -1 , eg_insert_m::Before );
			SetCursor( m_DragCantDropCursor );
		}
	}
}

void EGWndDragableLayoutEditor::OnWmMouseLeave()
{
	Super::OnWmMouseLeave();

	SetHoveredItem( -1 );
}

void EGWndDragableLayoutEditor::OnWmRButtonDown( const eg_ivec2& MousePos )
{
	Super::OnWmRButtonDown( MousePos );

	InvalidateByIndex( m_SelectedIndex );
	SetSelectedIndex( ClientPosToItemIndex( MousePos ) );
	InvalidateByIndex( m_SelectedIndex );
	SetHoveredItem( m_SelectedIndex );
}

void EGWndDragableLayoutEditor::OnWmRButtonUp( const eg_ivec2& MousePos )
{
	Super::OnWmRButtonUp( MousePos );

	OnItemRightClicked( m_SelectedIndex );
}

void EGWndDragableLayoutEditor::OnWmLButtonDown( const eg_ivec2& MousePos )
{
	Super::OnWmLButtonDown( MousePos );

	const eg_int NumItems = static_cast<eg_int>(GetNumItems());

	InvalidateByIndex( m_SelectedIndex );
	SetSelectedIndex( ClientPosToItemIndex( MousePos ) );
	InvalidateByIndex( m_SelectedIndex );
	SetHoveredItem( m_SelectedIndex );

	eg_bool bHoveredItemIsValid = 0 <= m_HoveredItem && m_HoveredItem < NumItems;

	if( m_DragType != eg_drag_t::None && bHoveredItemIsValid && CanDragItem( m_HoveredItem ) )
	{
		m_bHasMouseMovedSinceDragStart = false;
		m_MousePointAtStartOfDrag = MousePos;
		m_ItemBeingMovedIndex = m_HoveredItem;
		m_InsertMode = eg_insert_m::Before;
		m_InsertIndex = m_ItemBeingMovedIndex;
		m_RestoreCursor = GetCursor(); // Only get the cursor, we don't want it to change until we actually start dragging, this select could just be a click to set the item to edit.
		BeginCapture( MousePos );
	}
}

void EGWndDragableLayoutEditor::OnWmLButtonUp( const eg_ivec2& MousePos )
{
	Super::OnWmLButtonUp( MousePos );

	if( IsCapturing() )
	{
		EndCapture( MousePos );

		if( m_RestoreCursor )
		{
			SetCursor( m_RestoreCursor );
		}

		eg_int InsertBeforeIndex = m_InsertIndex;
		if( m_InsertMode == eg_insert_m::After )
		{
			InsertBeforeIndex++;
		}

		eg_int ObjToMoveIndex = m_ItemBeingMovedIndex;
		eg_int ObjToMoveBeforeIndex = InsertBeforeIndex;
		eg_int ObjToMoveOnIndex = -1;

		if( m_InsertMode == eg_insert_m::AsChild )
		{
			ObjToMoveOnIndex = ObjToMoveBeforeIndex;
			if( ObjToMoveOnIndex != ObjToMoveIndex && ObjToMoveOnIndex != -1 )
			{
				OnDraggedOnto( ObjToMoveIndex , ObjToMoveOnIndex );
			}
		}
		else
		{
			if
			( 
				(ObjToMoveBeforeIndex != -1) 
				&& (ObjToMoveIndex != ObjToMoveBeforeIndex)
			)
			{
				OnDragBefore( ObjToMoveIndex , ObjToMoveBeforeIndex );
			}
		}
		m_ItemBeingMovedIndex = -1;
		SetInsertIndex( -1 , eg_insert_m::Before );
	}
}

void EGWndDragableLayoutEditor::SetDragType( eg_drag_t NewDragType )
{
	assert( !IsCapturing() ); // Shouldn't change drag type while dragging
	m_DragType = NewDragType;
}

void EGWndDragableLayoutEditor::SetupDefaultCursors()
{
	m_DragCursor = LoadCursorW( nullptr , IDC_ARROW );
	m_DragCantDropCursor = LoadCursorW( nullptr , IDC_ARROW );
	m_DragMakeChildCursor = LoadCursorW( nullptr , IDC_ARROW );
}

RECT EGWndDragableLayoutEditor::GetDrawRectForItem( eg_int Index )
{
	RECT ViewRect = GetViewRect();
	RECT Out;
	
	if( m_ScrollDir == EGWndScrollingPanel::eg_scroll_dir::Vertical )
	{
		SetRect( &Out , 0 , Index*GetItemSize() , ViewRect.right , (Index+1)*GetItemSize() );
		OffsetRect( &Out , 0 , -m_ScrollPos );
	}
	else if( m_ScrollDir == EGWndScrollingPanel::eg_scroll_dir::Horizontal )
	{
		SetRect( &Out , Index*GetItemSize() , 0 , (Index+1)*GetItemSize() , ViewRect.bottom );
		OffsetRect( &Out , -m_ScrollPos , 0 );
	}

	return Out;
}

RECT EGWndDragableLayoutEditor::GetInsertBetweenRectForIndex( eg_int Index )
{
	RECT rc = GetViewRect();
	if( m_ScrollDir == EGWndScrollingPanel::eg_scroll_dir::Vertical )
	{
		rc.top = Index*GetItemSize() - m_ScrollPos - 1;
		rc.bottom = rc.top+3;
	}
	else if( m_ScrollDir == EGWndScrollingPanel::eg_scroll_dir::Horizontal )
	{
		rc.left = Index*GetItemSize() - m_ScrollPos - 1;
		rc.right = rc.left+3;
	}
	return rc;
}

void EGWndDragableLayoutEditor::SetInsertIndex( eg_int Index , eg_insert_m InsertMode )
{
	if( m_InsertIndex != Index || m_InsertMode != InsertMode )
	{
		m_InsertIndex = Index;
		m_InsertMode = InsertMode;

		FullRedraw();
	}
}

void EGWndDragableLayoutEditor::SetSelectedIndex( eg_int Index )
{
	if( m_SelectedIndex != Index )
	{
		m_SelectedIndex = Index;
		OnSelectedItemChanged( m_SelectedIndex );
		FullRedraw();
	}
}
