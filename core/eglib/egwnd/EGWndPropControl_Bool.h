// (c) 2017 Beem Media

#pragma once

#include "EGWndPropControl.h"

class EGWndPropControl_Bool : public EGWndPropControl
{
private: typedef EGWndPropControl Super;

public:

	EGWndPropControl_Bool( IWndPropControlOwner* Parent , const egPropControlInfo& Info );

	virtual void OnWmControlCmd( HWND WndControl , eg_int NotifyId , eg_int ControlId ) override;
	virtual void OnWmSize( const eg_ivec2& NewClientSize ) override;
	virtual void OnPaint( HDC hdc ) override;
	virtual void CommitChangesToSourceData( HWND CommitControl ) override;
	virtual void RefreshFromSourceData() override;

protected:

	HWND m_WndCheck = nullptr;
};
