// (c) 2017 Beem Media

#pragma once

#include "EGWndPanel.h"

class EGWndTabContainer : public EGWndPanel
{
private: typedef EGWndPanel Super;

private:

	EGWndPanelHeader m_Header;

public:

	EGWndTabContainer( EGWndPanel* Parent , eg_cpstr Name );
	virtual ~EGWndTabContainer() override;

	void SetHeaderText( eg_cpstr16 NewHeaderText );
};

class EGWndTabbedPanel : public EGWndPanel
{
	private: typedef EGWndPanel Super;

private:

	using EGWndPanel::AddChild;
	using EGWndPanel::AddContainer;

protected:

	struct egTabInfo
	{
		EGWndPanel* Panel = nullptr;
		EGWndTabContainer* TabContainer = nullptr;
		eg_s_string_sml8 Name = CT_Clear;
		RECT rcButton;

		egTabInfo() = default;
		egTabInfo( EGWndPanel* InPanel , eg_cpstr InName , EGWndTabContainer* InTabContainer ): Panel( InPanel ) , Name( InName ) , TabContainer(InTabContainer) { zero(&rcButton); }
	};

protected:

	EGArray<egTabInfo> m_Tabs;
	eg_int m_SelectedTabIndex = -1;
	eg_int m_HoveredTabIndex = -1;

	static const eg_int MAX_TAB_SIZE = 150;
	static const eg_int TAB_SELECT_BAR_SIZE = 2;

public:

	EGWndTabbedPanel( EGWndPanel* InParent );

	template<typename PanelType,typename ... CArgs>
	PanelType* AddTab( eg_cpstr Name , CArgs ... args )
	{
		EGWndTabContainer* TabContainer = AddChild<EGWndTabContainer>( Name );
		PanelType* Out = TabContainer->AddChild<PanelType>( args... );
		m_Tabs.Append( egTabInfo( Out , Name  , TabContainer) );
		OnNewTabAdded();
		return Out;
	}

	void SwitchToTab( eg_int TabIndex );

	void SetTabTextFor( EGWndPanel* Panel , eg_cpstr16 NewTabText );

	virtual void OnWmSize( const eg_ivec2& NewClientSize ) override;
	virtual void OnWmMouseLeave() override;
	virtual void OnWmMouseMove( const eg_ivec2& MousePos ) override;
	virtual void OnWmLButtonDown( const eg_ivec2& MousePos ) override;
	virtual void OnWmLButtonUp( const eg_ivec2& MousePos ) override;

	virtual void OnPaint( HDC hdc ) override;

private:

	void OnNewTabAdded();
	void RefreshButtonSizes();
	RECT GetTabRect( eg_int TabIndex ) const;
	eg_int GetHitTab( const eg_ivec2& HitPoint ) const;
};
