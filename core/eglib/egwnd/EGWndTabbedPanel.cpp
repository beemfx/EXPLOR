// (c) 2017 Beem Media

#include "EGWndTabbedPanel.h"

EGWndTabbedPanel::EGWndTabbedPanel( EGWndPanel* InParent )
: EGWndPanel( InParent, eg_panel_fill::Tabs )
{

}

void EGWndTabbedPanel::SwitchToTab( eg_int TabIndex )
{
	m_SelectedTabIndex = TabIndex;
	for( eg_int i=0; i<m_Children.LenAs<eg_int>(); i++ )
	{
		m_Children[i]->SetVisible( m_SelectedTabIndex == i );
	}
}

void EGWndTabbedPanel::SetTabTextFor( EGWndPanel* Panel , eg_cpstr16 NewTabText )
{
	for( eg_int i=0; i<m_Tabs.LenAs<eg_int>(); i++ )
	{
		if( m_Tabs[i].Panel == Panel && m_Tabs[i].TabContainer )
		{
			m_Tabs[i].Name = NewTabText;
			m_Tabs[i].TabContainer->SetHeaderText( NewTabText );
		}
	}

	FullRedraw();
}

void EGWndTabbedPanel::OnWmSize( const eg_ivec2& NewClientSize )
{
	Super::OnWmSize( NewClientSize );

	RefreshButtonSizes();
}

void EGWndTabbedPanel::OnWmMouseLeave()
{
	Super::OnWmMouseLeave();
	
	if( !IsCapturing() )
	{
		eg_int PrevHoverIndex = m_HoveredTabIndex;
		m_HoveredTabIndex = -1;
		if( m_HoveredTabIndex != PrevHoverIndex )
		{
			FullRedraw();
		}
	}
}

void EGWndTabbedPanel::OnWmMouseMove( const eg_ivec2& MousePos )
{
	Super::OnWmMouseMove( MousePos );

	TrackMouseLeave();

	if( !IsCapturing() )
	{
		eg_int PrevHoverIndex = m_HoveredTabIndex;
		m_HoveredTabIndex = GetHitTab( MousePos );

		if( m_HoveredTabIndex != PrevHoverIndex )
		{
			FullRedraw();
		}
	}
}

void EGWndTabbedPanel::OnWmLButtonDown( const eg_ivec2& MousePos )
{
	Super::OnWmLButtonDown( MousePos );

	eg_int PrevHoverIndex = m_HoveredTabIndex;
	m_HoveredTabIndex = GetHitTab( MousePos );

	if( m_HoveredTabIndex != -1 )
	{
		BeginCapture( MousePos );
	}

	if( m_HoveredTabIndex != PrevHoverIndex )
	{
		FullRedraw();
	}
}

void EGWndTabbedPanel::OnWmLButtonUp( const eg_ivec2& MousePos )
{
	Super::OnWmLButtonUp( MousePos );

	eg_int PrevHoverIndex = m_HoveredTabIndex;
	m_HoveredTabIndex = GetHitTab( MousePos );

	if( IsCapturing() )
	{
		EndCapture( MousePos );

		if( m_HoveredTabIndex != -1 && m_HoveredTabIndex == PrevHoverIndex )
		{
			SwitchToTab( m_HoveredTabIndex );
			FullRedraw();
		}
	}

	if( m_HoveredTabIndex != PrevHoverIndex )
	{
		FullRedraw();
	}
}

void EGWndTabbedPanel::OnPaint( HDC hdc )
{
	RECT rc = GetViewRect();
	rc.bottom = GetTabSize();

	FillRect( hdc , &rc , EGWnd_GetBrush( egw_color_t::BG_STATIC ) );
	rc.top = GetTabSize() - EGWnd_GetAppScaledSize( TAB_SELECT_BAR_SIZE );
	FillRect( hdc , &rc , EGWnd_GetBrush( egw_color_t::BG_BTN_SELECTED ) );

	const eg_int NumTabs = m_Tabs.LenAs<eg_int>();

	if( NumTabs > 0 )
	{
		SetFont( hdc , EGWnd_GetFont( egw_font_t::SMALL_BOLD ) );
		SetTextColor( hdc , EGWnd_GetColor( egw_color_t::DEFAULT ) );
		SetBkMode( hdc , TRANSPARENT );
		SetBrush( hdc , EGWnd_GetBrush( egw_color_t::BG_EDIT_DIRTY ) );

		auto DrawTab = [&rc,&hdc,this]( eg_int Index , const egTabInfo& TabInfo ) -> void
		{
			RECT rcTab = TabInfo.rcButton;

			HBRUSH BgBrush = EGWnd_GetBrush( egw_color_t::BG_BTN );

			if( m_SelectedTabIndex == Index )
			{
				SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_BTN_SELECTED ) );
				BgBrush = EGWnd_GetBrush( egw_color_t::BG_BTN_SELECTED );
			}
			else if( m_HoveredTabIndex == Index )
			{
				SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_BTN_HOVER ) );
				BgBrush = EGWnd_GetBrush( egw_color_t::BG_BTN_HOVER );
			}
			else
			{
				SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_BTN ) );
			}

			// SetBrush( hdc , BgBrush );
			FillRect( hdc , &rcTab , BgBrush );
			InflateRect( &rcTab , -2 , -2 );
			DrawTextW( hdc , *eg_s_string_sml16(*TabInfo.Name) , -1 , &rcTab , DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );
		};

		for( eg_int i=0; i<NumTabs; i++ )
		{
			DrawTab( i , m_Tabs[i] );
		}
	}
}

void EGWndTabbedPanel::OnNewTabAdded()
{
	if( m_Children.Len() == 1 )
	{
		m_SelectedTabIndex = 0;
	}
	else if( m_Children.Len() > 1 )
	{
		m_Children[m_Children.Len()-1]->SetVisible( false );
	}
	RefreshButtonSizes();
}

void EGWndTabbedPanel::RefreshButtonSizes()
{
	for( eg_int i=0; i<m_Tabs.LenAs<eg_int>(); i++ )
	{
		m_Tabs[i].rcButton = GetTabRect( i );
	}
}

RECT EGWndTabbedPanel::GetTabRect( eg_int TabIndex ) const
{
	if( !m_Tabs.IsValidIndex( 0 ) )
	{
		RECT Out;
		SetRect( &Out , 0 , 0 , 0 , 0 );
		return Out;
	}
	RECT WndRect = GetViewRect();
	WndRect.bottom = GetTabSize() - EGWnd_GetAppScaledSize( TAB_SELECT_BAR_SIZE );

	const eg_int NumTabs = m_Tabs.LenAs<eg_int>();
	const eg_int TabSize = EG_Min<eg_int>( EGWnd_GetAppScaledSize(MAX_TAB_SIZE) , WndRect.right/NumTabs );

	RECT rcTab;
	SetRect( &rcTab , WndRect.left + TabSize*TabIndex , WndRect.top , WndRect.left + TabSize*(TabIndex+1), WndRect.bottom );

	return rcTab;
}

eg_int EGWndTabbedPanel::GetHitTab( const eg_ivec2& MousePos ) const
{
	const POINT HitPoint = { MousePos.x , MousePos.y };

	for( eg_int i=0; i<m_Tabs.LenAs<eg_int>(); i++ )
	{
		if( PtInRect( &m_Tabs[i].rcButton , HitPoint ) )
		{
			return i;
		}
	}
	return -1;
}

EGWndTabContainer::EGWndTabContainer( EGWndPanel* Parent , eg_cpstr Name )
: Super( Parent, eg_panel_fill::Vertical )
, m_Header( this, Name )
{
	m_Children.Append( &m_Header );
}

EGWndTabContainer::~EGWndTabContainer()
{
	m_Children.DeleteByItem( &m_Header );
}

void EGWndTabContainer::SetHeaderText( eg_cpstr16 NewHeaderText )
{
	m_Header.SetHeaderText( *eg_d_string8(NewHeaderText) );
}
