// (c) 2016 Beem Media

#pragma once
#include "EGWndPanel.h"

class EGWndScrollingPanel : public EGWndPanel
{
	EG_DECL_SUPER( EGWndPanel )

public:

	enum class eg_scroll_dir
	{
		Vertical,
		Horizontal,
	};

protected:

	HFONT         m_ItemFont;
	HFONT         m_SelectedItemFont;
	eg_int        m_ScrollPos;
	eg_int        m_HoveredItem;
	eg_int        m_WheelScrollAmount = 1;
	eg_bool       m_bFavorBottom = false;
	eg_int        m_LastViewSize = 0;
	eg_scroll_dir m_ScrollDir = eg_scroll_dir::Vertical;
	eg_int        m_ItemSize = 1;
	eg_bool       m_bAlwaysShowScrollbar = false;

//
// Override Interface
//
public:

	virtual eg_size_t GetNumItems() const = 0;
	virtual void OnScrollChanged( eg_int NewScrollPos , eg_int OldScrollPos ){ unused( NewScrollPos , OldScrollPos ); }

public:

	EGWndScrollingPanel( EGWndPanel* Parent , eg_int ItemSize );
	~EGWndScrollingPanel();
	void UpdateScroll();
	eg_int GetScroll() const;
	void SetScroll( eg_int Scroll );
	void ScrollProc( eg_int nScrollCode );
	void SetHoveredItem( eg_int NewSelectedItem );
	void InvalidateByIndex( eg_int Index );
	eg_int ClientPosToItemIndex( const eg_ivec2& ClientPos );
	eg_real ClientPosToPct( eg_int Index , const eg_ivec2& ClientPos ) const;
	eg_int GetItemSize() const{ return m_ItemSize; }
	void SetWheelScrollAmount( eg_int NewScrollAmount ){ m_WheelScrollAmount = NewScrollAmount; }
	void SetAlwaysShowScrollbar( eg_bool bNewValue );

	// BEGIN EGWndBase
	virtual void OnWmSize( const eg_ivec2& NewClientSize ) override;
	virtual void OnWmVScroll( eg_int Request , eg_int ThumbPos ) override;
	virtual void OnWmHScroll( eg_int Request , eg_int ThumbPos ) override;
	virtual eg_bool OnWmMouseWheel( eg_int Delta ) override;
	// END EGWndBase

	virtual void OnHoveredItemChanged( eg_int NewHoveredIndex ){ unused( NewHoveredIndex ); }

protected:

	int GetWndScrollBar() const;
	void SetItemSize( eg_int NewItemSize );
	void SetScrollDir( eg_scroll_dir NewScrollDir );
};