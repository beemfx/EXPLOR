// (c) 2017 Beem Media

#pragma once

#include "EGWndPropControl.h"

class EGWndPropControl_Container : public EGWndPropControl
{
	EG_DECL_SUPER( EGWndPropControl )

private:

	static const eg_int EXPAND_BTN_SIZE = 12;

protected:

	COLORREF m_HeaderColor = RGB(255,255,255);
	HWND     m_WndBtnExpand = nullptr;

public:

	EGWndPropControl_Container( IWndPropControlOwner* Parent , const egPropControlInfo& Info );

	// BEGIN EGWndPropControl
	virtual void OnPaint( HDC hdc ) override;
	virtual void OnWmControlCmd( HWND WndControl , eg_int NotifyId , eg_int ControlId ) override;
	// END EGWndPropControl

protected:

	eg_bool IsExpanded() const;
	virtual void RefreshExpandBtnText();
};

class EGWndPropControl_Array : public EGWndPropControl_Container
{
	EG_DECL_SUPER( EGWndPropControl_Container )

private:

	HWND m_WndBtnExpand = nullptr;
	HWND m_WndBtnAppend = nullptr;
	HWND m_WndBtnRemove = nullptr;
	
public:

	EGWndPropControl_Array( IWndPropControlOwner* Parent , const egPropControlInfo& Info );

	// BEGIN EGWndPropControl
	virtual void OnWmControlCmd( HWND WndControl , eg_int NotifyId , eg_int ControlId ) override;
	virtual void RefreshExpandBtnText() override;
	// END EGWndPropControl
};

class EGWndPropControl_Struct : public EGWndPropControl_Container
{
	EG_DECL_SUPER( EGWndPropControl_Container )

public:

	EGWndPropControl_Struct( IWndPropControlOwner* Parent , const egPropControlInfo& Info );
};
