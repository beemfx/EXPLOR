// (c) 2016 Beem Media

#pragma once

#include "EGWnd.h"
#include "EGWndChildBase.h"

enum class eg_panel_fill // How children are added to th is panel
{
	 None,
	 Horizontal,
	 Vertical,
	 Tabs,
};

enum class eg_panel_size_t // How this panel fills itself into it's parent
{
	Fixed,
	Auto,
	Percent,
};

struct egWndPanelCustomLayout
{
	const eg_bool bUseCustomDraw = true;

	egWndPanelCustomLayout() = default;
};

struct egWndPanelAppContainer
{
	EGWndBase* BaseWindowParent = nullptr;

	explicit egWndPanelAppContainer( EGWndBase* InBaseWindowParent ): BaseWindowParent( InBaseWindowParent ){ }
};

class EGWndPanel : public EGWndChildBase
{
private: typedef EGWndChildBase Super;

private:

	static const eg_int PANE3_SIDE_WIDTH = 250;
	static const eg_int PANE3_MID_MIN_WIDTH = 300; // Doesn't account for borders
	static const eg_int MAIN_WND_MIN_HEIGHT = 400;
	static const eg_int PREVIEW_PADDING = 10;
	static const eg_int HEADER_SIZE = 22;
	static const eg_int TAB_SIZE = 22;

public:

	static eg_int GetPane3SideWidth();
	static eg_int GetPane3MidMinWidth();
	static eg_int GetMainWndMinHeight();
	static eg_int GetPreviewPadding();
	static eg_int GetHeaderSize();
	static eg_int GetTabSize();

public:

	EGWndPanel( egWndPanelAppContainer& Parent );
	EGWndPanel( EGWndPanel* Parent , egWndPanelCustomLayout CustomLayout );
	EGWndPanel( EGWndPanel* Parent , eg_panel_fill FillMode );
	EGWndPanel( EGWndPanel* Parent , eg_panel_size_t SizeType , eg_int Size );
	EGWndPanel( EGWndPanel* Parent , eg_panel_fill FillMode , eg_panel_size_t SizeType , eg_int Size );
	virtual ~EGWndPanel() override;

	void SetFillSize( eg_panel_size_t InType , eg_int InSize );
	eg_int GetFillSize() const { return m_FillSize; }
	eg_panel_size_t GetFillSizeType() const { return m_FillSizeType; }
	void SetFillMode( eg_panel_fill NewFillMode );
	RECT GetParentClientRect() const;
	eg_panel_fill GetFillMode() const { return m_FillMode; }

	void HandleResize();

	EGWndPanel* GetParentPanel(){ return m_OwnerPanel;	}
	const EGWndPanel* GetParentPanel() const { return m_OwnerPanel; }

	RECT GetChildFill( const EGWndPanel* ChildPanel ) const;

	template<typename PanelType,typename ... CArgs>
	PanelType* AddChild( CArgs ... args )
	{
		PanelType* NewPanel = new PanelType( this , args... );
		m_Children.Append( NewPanel );
		HandleResize();
		return NewPanel;
	}

	EGWndPanel* AddContainer( egWndPanelCustomLayout CustomLayout )
	{
		EGWndPanel* NewPanel = new EGWndPanel( this , CustomLayout );
		m_Children.Append( NewPanel );
		HandleResize();
		return NewPanel;
	}

	EGWndPanel* AddContainer( eg_panel_fill FillMode )
	{
		EGWndPanel* NewPanel = new EGWndPanel( this , FillMode );
		m_Children.Append( NewPanel );
		HandleResize();
		return NewPanel;
	}

	eg_bool IsMouseOverPanel( POINT* ClientPosOut = nullptr ) const;

	// Only for parent windows to call
	eg_int GetParentOwnedSize() const { return m_ParentOwnedSize; }
	void SetParentOwnedSize( eg_int NewParentOwnedSize ){ m_ParentOwnedSize = NewParentOwnedSize; }

	virtual void SetVisible( eg_bool bVisible ) override;

private:

	void ReevaluateChildSizes();
	virtual RECT GetCustomWindowRect() const;

protected:

	eg_panel_fill        m_FillMode = eg_panel_fill::None;
	eg_panel_size_t      m_FillSizeType = eg_panel_size_t::Auto;
	eg_int               m_FillSize = 0;
	eg_bool              m_bUseCustomWindowRect = false;
	EGWndPanel*const     m_OwnerPanel = nullptr;
	EGArray<EGWndPanel*> m_Children;

private:
	
	eg_int m_ParentOwnedSize = 200; // Variable used solely so the parent can compute the layout
};

class EGWndPanelHeader: public EGWndPanel
{
private: typedef EGWndPanel Super;

private:
	
	COLORREF m_TextColor  = RGB(255,0,0);
	egw_color_t m_BgColor = egw_color_t::BG_EDIT;

public:

	EGWndPanelHeader( EGWndPanel* Parent , eg_cpstr Text )
	: EGWndPanel( Parent , eg_panel_size_t::Fixed , GetHeaderSize() )
	{
		SetHeaderText( Text );
	}

	void SetHeaderText( eg_cpstr Str )
	{ 
		m_HeaderText = Str;
		FullRedraw();
	}

	void SetColors( COLORREF TextColor , egw_color_t BgColor )
	{
		m_TextColor = TextColor;
		m_BgColor = BgColor;
		FullRedraw();
	}

	virtual void OnPaint( HDC hdc ) override final
	{
		RECT rc = GetViewRect();

		SetTextColor( hdc , m_TextColor );
		SetBkMode( hdc , TRANSPARENT );
		SetFont( hdc , EGWnd_GetFont( egw_font_t::DEFAULT_BOLD ) );
		DrawTextA( hdc , m_HeaderText , -1 , &rc , DT_CENTER|DT_SINGLELINE|DT_VCENTER );
	}

	virtual void OnDrawBg( HDC hdc ) override final
	{
		FillRect( hdc , &GetViewRect() , EGWnd_GetBrush( m_BgColor ) );
	}

private:

	eg_string m_HeaderText;
};

class EGWndBgPanel: public EGWndPanel
{
private: typedef EGWndPanel Super;

private:

	HBRUSH m_BgBrush = nullptr;

public:

	EGWndBgPanel( EGWndPanel* Parent , COLORREF BgColor )
	: EGWndPanel( Parent , eg_panel_size_t::Auto , 0 )
	{
		m_BgBrush = CreateSolidBrush( BgColor );

		FullRedraw();
	}

	virtual ~EGWndBgPanel() override
	{
		DeleteObject( m_BgBrush );
		m_BgBrush = nullptr;
	}

	virtual void OnPaint( HDC hdc ) override final
	{
		RECT rc = GetViewRect();

		SetTextColor( hdc , RGB( 255 , 0 , 0 ) );
		SetBkMode( hdc , TRANSPARENT );
		SetFont( hdc , EGWnd_GetFont( egw_font_t::DEFAULT_BOLD ) );
		DrawTextA( hdc , m_HeaderText , -1 , &rc , DT_CENTER|DT_SINGLELINE|DT_VCENTER );
	}

	virtual void OnDrawBg( HDC hdc ) override final
	{
		FillRect( hdc , &GetViewRect() , m_BgBrush );
	}

private:

	eg_string m_HeaderText;
};