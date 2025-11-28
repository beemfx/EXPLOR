// (c) 2016 Beem Media

#pragma once

#include "EGWndPanel.h"

class EGLytEdPreviewPanel: public EGWndPanel
{
private:

	typedef EGWndPanel Super;
	static const eg_uint TIMER_ID = 24;

public:

	EGLytEdPreviewPanel( EGWndPanel* Parent );
	~EGLytEdPreviewPanel();

	virtual LRESULT WndProc( UINT Msg, WPARAM wParam, LPARAM lParam) override final;
	virtual void OnPaint( HDC hdc ) override final;
	virtual void OnDrawBg( HDC hdc ) override final;

	void SetPreviewPanelAspectRatio( eg_real AspectRatio );
	void DropLibraryItem( eg_cpstr StrDefId );

	static eg_vec2 ClientSpaceToMenuSpace( eg_int x , eg_int y , const RECT& rcClient );
	static EGLytEdPreviewPanel* GetPanel(){ return GlobalPreviewPanel; }

private:
	virtual RECT GetCustomWindowRect() const override final;

private:

	EGWndMouseCapture      m_MouseCapture;
	struct egUiWidgetInfo* m_CaptureObj;
	eg_transform           m_CaptureObjIntialPos;
	eg_real                m_PreviewPanelAspectRatio = 16.f/9.f;
	eg_bool                m_bCaptureIsRMB:1;
	eg_bool                m_bCaptureIsMMB:1;

private:

	static EGLytEdPreviewPanel* GlobalPreviewPanel;
};
