// (c) 2017 Beem Media

#pragma once

#include "EGWndPropControl.h"
#include "EGWndTextNode.h"

class EGWndPropControl_Header : public EGWndPropControl
{
private: typedef EGWndPropControl Super;

public:

	EGWndPropControl_Header( IWndPropControlOwner* Parent , const egPropControlInfo& Info );

	virtual void OnPaint( HDC hdc ) override;

protected:

	eg_string m_HeaderText;
};

///////////////////////////////////////////////////////////////////////////////

class EGWndPropControl_Button : public EGWndPropControl
{
private: typedef EGWndPropControl Super;

public:

	EGWndPropControl_Button( IWndPropControlOwner* Parent , const egPropControlInfo& Info );

	virtual void OnWmControlCmd( HWND WndControl , eg_int NotifyId , eg_int ControlId ) override;
	virtual void OnPaint( HDC hdc ) override { unused( hdc ); }
	virtual void RefreshFromSourceData() override;

protected:

	HWND m_WndButton = nullptr;
};

class EGWndPropControl_Unk : public EGWndPropControl
{
	EG_DECL_SUPER( EGWndPropControl )

public:

	EGWndPropControl_Unk( IWndPropControlOwner* Parent , const egPropControlInfo& Info );
	~EGWndPropControl_Unk();

	virtual void OnPaint( HDC hdc ) override;

protected:

	HPEN m_Pen = nullptr;
};