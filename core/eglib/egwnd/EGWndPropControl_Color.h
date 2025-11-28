// (c) 2017 Beem Media

#pragma once

#include "EGWndPropControl.h"
#include "EGWndTextNode.h"

class EGWndPropControl_Color : public EGWndPropControl
{
private: typedef EGWndPropControl Super;

public:

	EGWndPropControl_Color( IWndPropControlOwner* Parent , const egPropControlInfo& Info );
	~EGWndPropControl_Color();

	virtual LRESULT WndProc( UINT message , WPARAM wParam , LPARAM lParam ) override;
	virtual void OnPaint( HDC hdc ) override;
	virtual void CommitChangesToSourceData( HWND CommitControl ) override;
	virtual void RefreshFromSourceData() override;

private:

	const RECT m_rcColorButton = { 2 , FONT_SIZE*2 , 78 , FONT_SIZE*3+8 };
	EGWndTextNode m_EditAlpha;
	HBRUSH   m_ColorBrush = nullptr;
	eg_color m_Color;
	COLORREF m_ColorChooseColors[16];
	EGWndMouseCapture m_MouseCapture;

private:

	void UpdateBrush( const eg_color& NewColor );
};
