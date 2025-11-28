// (c) 2017 Beem Media

#pragma once

#include "EGWndScrollingPanel.h"
#include "EGWndPropControl.h"
#include "EGReflection.h"

struct egRflEditor;

class EGWndPanelPropEditor : public EGWndScrollingPanel , public IWndPropControlOwner
{
	EG_DECL_SUPER( EGWndScrollingPanel )

private:

	static const eg_int CTL_Y_PADDING = 5;
	static const eg_int CTL_X_PADDING = 2;
	static const eg_int DEPTH_PADDING = 16;

public:

	struct egProp
	{
		egEditProperty          Info;
		eg_int                  Depth = 0;
		eg_int                  Offset = 0;
		class EGWndPropControl* Control = nullptr;
	};

protected:

	egRflEditor*      m_CurEditor = nullptr;
	EGEditPropArray   m_CurProps;
	EGArray<egProp>   m_Props;
	eg_int            m_TotalHeight = 0;
	eg_string         m_NoPropsMessage = CT_Clear;
	HPEN              m_RelationPen = nullptr;
	EGArray<eg_ivec2> m_DepthRelations;
	eg_bool           m_bRebuildQueued = false;

public:

	EGWndPanelPropEditor( EGWndPanel* Parent );
	~EGWndPanelPropEditor();

	// BEGIN IWndPropControlOwner
	virtual EGWndBase* GetOwningWnd() override final { return this; }
	virtual void OnPropChangedValue( const egRflEditor* Editor ) override final;
	virtual void OnPropControlButtonPressed( const egRflEditor* Property ) override final;
	virtual void PropEditorRebuild() override final;
	// END IWndPropControlOwner

	void SetEditObject( egRflEditor* RflEditor );
	void RefreshAll();
	void SetNoPropsMessage( eg_cpstr MessageIn ) { m_NoPropsMessage = MessageIn; }

	// BEGIN EGWndScrollingPanel
	virtual void OnPaint( HDC hdc ) override final;
	virtual void OnDrawBg( HDC hdc ) override final;
	virtual void OnScrollChanged( eg_int NewScrollPos , eg_int OldScrollPos ) override final;
	virtual eg_size_t GetNumItems() const override final;

	virtual LRESULT WndProc( UINT Msg , WPARAM wParam , LPARAM lParam ) override final;
	virtual void OnWmSize( const eg_ivec2& NewClientSize ) override final;
	// END EGWndScrollingPanel

	void DrawRelations( HDC hdc );

	static eg_string GetNoPropsMessage( eg_bool bNoObject , eg_bool bLocked );

protected:

	void ClearControls();

	void RebuildControls();
	void RefreshControlSizes( eg_int ClientWidth );

	EGWndPropControl* CreatePropControl( const egEditProperty& Info );

	virtual void HandlePropChanged( const egRflEditor* Editor );
	virtual void OnButtonPressed( const egRflEditor* Property ){ unused( Property ); }
};
