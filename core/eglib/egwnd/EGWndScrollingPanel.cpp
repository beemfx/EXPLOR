// (c) 2016 Beem Media

#include "EGWndScrollingPanel.h"


EGWndScrollingPanel::EGWndScrollingPanel( EGWndPanel* Parent , eg_int ItemSize )
: EGWndPanel( Parent , eg_panel_size_t::Auto , 0 )
, m_ScrollPos( 0 )
, m_HoveredItem( -1 )
, m_ItemSize( ItemSize )
{
	m_ItemFont = EGWnd_GetFont( egw_font_t::DEFAULT );
	m_SelectedItemFont = EGWnd_GetFont( egw_font_t::DEFAULT_BOLD );

	FullRedraw();
}

EGWndScrollingPanel::~EGWndScrollingPanel()
{

}

void EGWndScrollingPanel::UpdateScroll()
{
	RECT rcClient = GetViewRect();
	const eg_int _ViewHeight = rcClient.bottom - rcClient.top;
	const eg_int _ViewWidth = rcClient.right - rcClient.left;
	const eg_int ViewSize = m_ScrollDir == eg_scroll_dir::Vertical ? _ViewHeight : _ViewWidth;

	eg_int OldScrollPos = m_ScrollPos;

	if( m_bFavorBottom && ViewSize != m_LastViewSize )
	{
		m_ScrollPos = -ViewSize + OldScrollPos + m_LastViewSize;
	}

	SCROLLINFO si;
	zero( &si );
	si.cbSize = sizeof( si );
	si.fMask = SIF_RANGE|SIF_PAGE|SIF_POS;
	si.nPage = ViewSize;
	si.nMin = 0;
	si.nMax = static_cast<eg_int>( m_ItemSize*GetNumItems() );
	si.nPos = EG_Max<eg_int>( 0, EG_Clamp<eg_int>( m_ScrollPos, 0, si.nMax - si.nPage ) );
	m_ScrollPos = si.nPos;

	if( m_bAlwaysShowScrollbar )
	{
		si.fMask |= SIF_DISABLENOSCROLL;
	}
	SetScrollInfo( m_hwnd , GetWndScrollBar() , &si , TRUE );

	m_LastViewSize = ViewSize;

	OnScrollChanged( m_ScrollPos , OldScrollPos );
	FullRedraw();
}

eg_int EGWndScrollingPanel::GetScroll() const
{
	return m_ScrollPos;
}

void EGWndScrollingPanel::SetScroll( eg_int Scroll )
{
	SCROLLINFO si;
	zero( &si );
	si.cbSize = sizeof( si );
	si.fMask = SIF_POS;
	si.nPos = Scroll;
	eg_int OldScrollPos = m_ScrollPos;
	m_ScrollPos = Scroll;
	if( m_bAlwaysShowScrollbar )
	{
		si.fMask |= SIF_DISABLENOSCROLL;
	}
	m_ScrollPos = SetScrollInfo( GetWnd() , GetWndScrollBar() , &si , TRUE );
	OnScrollChanged( m_ScrollPos , OldScrollPos );
	//Check to see if info has changed, if it has scroll the window.
	if( m_ScrollPos != OldScrollPos )
	{
		switch( m_ScrollDir )
		{
		case eg_scroll_dir::Vertical:
			ScrollContent( 0 , OldScrollPos - si.nPos );
			break;
		case eg_scroll_dir::Horizontal:
			ScrollContent( OldScrollPos - si.nPos , 0 );
			break;
		}	
	}
}

void EGWndScrollingPanel::ScrollProc( eg_int nScrollCode )
{
	SCROLLINFO si;
	si.cbSize = sizeof( si );
	si.fMask = SIF_ALL;
	GetScrollInfo( GetWnd() , GetWndScrollBar() , &si );
	if( si.nPage > (UINT)si.nMax )
	{
		return;
	}
	//Save previous info
	SCROLLINFO siPrevInfo = si;

	switch( nScrollCode ) {
	case SB_PAGEDOWN:
		si.nPos += si.nPage / 2; break;
	case SB_LINEDOWN:
		si.nPos += m_ItemSize*m_WheelScrollAmount; break;
	case SB_PAGEUP:
		si.nPos -= si.nPage / 2; break;
	case SB_LINEUP:
		si.nPos -= m_ItemSize*m_WheelScrollAmount; break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		si.nPos = si.nTrackPos; break;
	case SB_TOP:
		si.nPos = si.nMin; break;
	case SB_BOTTOM:
		si.nPos = si.nMax; break;
	}
	if( si.nPos < 0 )si.nPos = 0;
	if( si.nPos > ( int )( si.nMax - si.nPage + 1 ) )si.nPos = si.nMax - si.nPage + 1;
	if( m_bAlwaysShowScrollbar )
	{
		si.fMask |= SIF_DISABLENOSCROLL;
	}
	SetScrollInfo( m_hwnd , GetWndScrollBar() , &si , TRUE );

	m_ScrollPos = si.nPos;

	OnScrollChanged( m_ScrollPos , siPrevInfo.nPos );
	//Check to see if info has changed, if it has scroll the window.
	if( si.nPos != siPrevInfo.nPos )
	{
		switch( m_ScrollDir )
		{
		case eg_scroll_dir::Vertical:
			ScrollContent( 0 , siPrevInfo.nPos - si.nPos );
			break;
		case eg_scroll_dir::Horizontal:
			ScrollContent( siPrevInfo.nPos - si.nPos , 0 );
			break;
		}	
	}
}

void EGWndScrollingPanel::SetHoveredItem( eg_int NewSelectedItem )
{
	if( NewSelectedItem != m_HoveredItem )
	{
		eg_int OldHoveredItem = m_HoveredItem;
		m_HoveredItem = NewSelectedItem;

		//EGLogf( eg_log_t::General , "Selected %u" , NewSelectedItem );

		// Invalidate the newly highlighted and previously highlighted items
		InvalidateByIndex( OldHoveredItem );
		InvalidateByIndex( m_HoveredItem );
		UpdateWindow( m_hwnd );
		//RedrawWindow( m_hwnd , nullptr , nullptr , RDW_ERASE|RDW_INVALIDATE );

		OnHoveredItemChanged( m_HoveredItem );
	}
}

void EGWndScrollingPanel::InvalidateByIndex( eg_int Index )
{
	RECT rcDirty = GetViewRect();

	if( m_ScrollDir == eg_scroll_dir::Vertical )
	{
		rcDirty.top = Index*m_ItemSize - m_ScrollPos;
		rcDirty.bottom = rcDirty.top + m_ItemSize;
	}
	else if( m_ScrollDir == eg_scroll_dir::Horizontal )
	{
		rcDirty.left = Index*m_ItemSize - m_ScrollPos;
		rcDirty.right = rcDirty.left + m_ItemSize;
	}

	InvalidateRect( m_hwnd, &rcDirty, TRUE );
}

eg_int EGWndScrollingPanel::ClientPosToItemIndex( const eg_ivec2& ClientPos )
{
	eg_int Index = -1;
	if( m_ScrollDir == eg_scroll_dir::Vertical )
	{
		Index = (ClientPos.y + m_ScrollPos)/m_ItemSize;
	}
	else if( m_ScrollDir == eg_scroll_dir::Horizontal )
	{
		Index = (ClientPos.x + m_ScrollPos)/m_ItemSize;
	}
	return Index;
}

eg_real EGWndScrollingPanel::ClientPosToPct( eg_int Index , const eg_ivec2& ClientPos ) const
{
	eg_real Out = 0.f;

	eg_real ActualPos = 0;

	if( m_ScrollDir == eg_scroll_dir::Vertical )
	{
		ActualPos = static_cast<eg_real>(ClientPos.y + m_ScrollPos);
	}
	else if( m_ScrollDir == eg_scroll_dir::Horizontal )
	{
		ActualPos = static_cast<eg_real>(ClientPos.x + m_ScrollPos);	
	}

	eg_real ItemStart = static_cast<eg_real>(Index*m_ItemSize);
	eg_real ItemEnd = static_cast<eg_real>((Index+1)*m_ItemSize);

	// EGLogf( eg_log_t::General , "Index %u %g-%g (%g)" , Index , ItemStart , ItemEnd , ActualPos );

	Out = EGMath_GetMappedRangeValue( ActualPos , eg_vec2( ItemStart , ItemEnd ) , eg_vec2(0.f , 1.f ) );

	return Out;
}

void EGWndScrollingPanel::SetAlwaysShowScrollbar( eg_bool bNewValue )
{
	m_bAlwaysShowScrollbar = bNewValue;
	UpdateScroll();
}

void EGWndScrollingPanel::OnWmSize( const eg_ivec2& NewClientSize )
{
	Super::OnWmSize( NewClientSize );

	UpdateScroll();
}

void EGWndScrollingPanel::OnWmVScroll( eg_int Request, eg_int ThumbPos )
{
	Super::OnWmVScroll( Request , ThumbPos );

	ScrollProc( Request );
}

void EGWndScrollingPanel::OnWmHScroll( eg_int Request, eg_int ThumbPos )
{
	Super::OnWmHScroll( Request , ThumbPos );

	ScrollProc( Request );
}

eg_bool EGWndScrollingPanel::OnWmMouseWheel( eg_int Delta )
{
	Super::OnWmMouseWheel( Delta );

	ScrollProc( Delta < 0 ? SB_LINEDOWN : SB_LINEUP );

	return true;
}

int EGWndScrollingPanel::GetWndScrollBar() const
{
	return m_ScrollDir == eg_scroll_dir::Vertical ? SB_VERT : SB_HORZ;
}

void EGWndScrollingPanel::SetItemSize( eg_int NewItemSize )
{
	assert( NewItemSize > 0 );
	m_ItemSize = EG_Max<eg_int>( NewItemSize , 1 );
	UpdateScroll();
	FullRedraw();
}

void EGWndScrollingPanel::SetScrollDir( eg_scroll_dir NewScrollDir )
{
	m_ScrollDir = NewScrollDir;

	SetWndStyle( WS_HSCROLL , NewScrollDir == eg_scroll_dir::Horizontal );
	SetWndStyle( WS_VSCROLL , NewScrollDir == eg_scroll_dir::Vertical );

	SCROLLINFO si;
	zero( &si );
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nPage = 0;
	si.nMin = 0;
	si.nMax = 0;
	si.nPos = 0;
	m_ScrollPos = 0;
	SetScrollInfo( GetWnd() , SB_VERT , &si , TRUE );
	SetScrollInfo( GetWnd() , SB_HORZ , &si , TRUE );

	UpdateScroll();
}
