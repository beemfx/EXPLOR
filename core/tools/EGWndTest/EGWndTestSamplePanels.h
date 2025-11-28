// (c) 2017 Beem Media

#pragma once

#include "EGWndPanel.h"
#include "EGWndDragableLayoutEditor.h"
#include "EGWndTextNodeWndControl.h"

class EGWndTestTodoPanel : public EGWndPanel
{
	EG_DECL_SUPER( EGWndPanel )

public:

	EGWndTestTodoPanel( EGWndPanel* InParent )
	: Super( InParent , eg_panel_size_t::Auto , 0 )
	{ 

	}

	// BEGIN EGWndPanel
	virtual void OnDrawBg( HDC hdc ) override
	{
		RECT rc = GetViewRect();
		FillRect( hdc , &rc , EGWnd_GetBrush( egw_color_t::BG_STATIC ) );
	}

	virtual void OnPaint( HDC hdc ) override
	{
		RECT rc = GetViewRect();
		SetBkMode( hdc , TRANSPARENT );
		SetFont( hdc , EGWnd_GetFont( egw_font_t::DEFAULT ) );
		SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_STATIC ) );
		DrawTextW( hdc , L"Sample Content Panel" , -1 , &rc , DT_CENTER|DT_SINGLELINE|DT_VCENTER );
	}
	// END EGWndPanel
};

class EGWndTestTextEditorPanel : public EGWndPanel
{
	EG_DECL_SUPER( EGWndPanel )

private:

	EGWndTextNodeWndControl m_TextNode;

public:

	EGWndTestTextEditorPanel( EGWndPanel* InParent )
	: Super( InParent , eg_panel_size_t::Auto , 0 )
	, m_TextNode( this )
	{ 
		m_TextNode.SetText( "A really long text with some new lines.\r\nYeah it has some new lines.\r\n\r\nIt really does." );
		m_TextNode.SetCommitAction( EGWndTextNodeWndControl::eg_commit_action::PressShiftEnter );
		m_TextNode.SetFont( EGWnd_GetFont( egw_font_t::CODE_EDIT ) );
		m_TextNode.SetMultiline( true );
		m_TextNode.SetWordWrap( false );
		m_TextNode.SetStatic( false );
		m_TextNode.SetDirty( false );
	}

	// BEGIN EGWndPanel
	virtual void OnDrawBg( HDC hdc ) override
	{
		unused( hdc );
	}

	virtual void OnPaint( HDC hdc ) override
	{
		unused( hdc );
	}

	virtual void OnWmSize( const eg_ivec2& NewClientSize ) override
	{
		Super::OnWmSize( NewClientSize );

		m_TextNode.SetWndPos( 0 , 0 , NewClientSize.x , NewClientSize.y );
	}

	virtual void OnWmEgCommit( HWND CommitCtrl ) override
	{
		if( CommitCtrl == m_TextNode.GetWnd() )
		{
			m_TextNode.SetDirty( false );
		} 
	}

	// END EGWndPanel
};

class EGWndTestSampleDragableEditor : public EGWndDragableLayoutEditor
{
	EG_DECL_SUPER( EGWndDragableLayoutEditor )

private:

	EGArray<eg_string_small> m_Items;

public:

	EGWndTestSampleDragableEditor( EGWndPanel* Parent )
	: Super( Parent )
	{
		// m_DragType = eg_drag_t::None;

		RefreshPanel();
	}

	void RefreshPanel()
	{
		m_Items.Clear();

		for( eg_int i = 0; i < 20; i++ )
		{
			m_Items.Append( EGString_Format( "Item %i" , i ) );
		}

		SetSelectedIndex( -1 );

		UpdateScroll();
	}

private:

	// BEGIN EGWndDragableLayoutEditor
	virtual eg_size_t GetNumItems() const override { return m_Items.Len(); }
	virtual void OnDrawItem( HDC hdc , eg_int ItemIndex , const RECT& rc , eg_bool bIsBeingDroppedOn , eg_bool bIsHovered , eg_bool bIsSelected ) const override;
	// END EGWndDragableLayoutEditor
};

class EGWndTestHorzScroller : public EGWndTestSampleDragableEditor
{
	EG_DECL_SUPER( EGWndTestSampleDragableEditor )

public:

	EGWndTestHorzScroller( EGWndPanel* Parent )
	: Super( Parent )
	{
		SetFillSize( eg_panel_size_t::Fixed , 100 );
		SetScrollDir( eg_scroll_dir::Horizontal );
		SetItemSize( 100 );

		RefreshPanel();
	}

	// virtual eg_size_t GetNumItems() const override { return 25; }
};