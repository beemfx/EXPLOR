// (c) 2017 Beem Media

#pragma once

#include "EGWndPropControl.h"
#include "EGWndTextNode.h"

class EGWndPropControl_Rot : public EGWndPropControl
{
private: typedef EGWndPropControl Super;

public:

	EGWndPropControl_Rot( IWndPropControlOwner* Parent , const egPropControlInfo& Info );

	virtual LRESULT WndProc( UINT message , WPARAM wParam , LPARAM lParam ) override;
	virtual void CommitChangesToSourceData( HWND CommitControl ) override;
	virtual void RefreshFromSourceData() override;

protected:

	EGWndTextNode m_WndEditX;
	EGWndTextNode m_WndEditY;
	EGWndTextNode m_WndEditZ;

	HWND m_WndCommitX = nullptr;
	HWND m_WndCommitY = nullptr;
	HWND m_WndCommitZ = nullptr;
	HWND m_WndCommitReset = nullptr;

	HWND m_WndQuatDisplay = nullptr;

	eg_quat m_Quat;
};
