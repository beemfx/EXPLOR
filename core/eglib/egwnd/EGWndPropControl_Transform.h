// (c) 2017 Beem Media

#pragma once

#include "EGWndPropControl.h"
#include "EGWndTextNode.h"

class EGWndPropControl_Transform : public EGWndPropControl
{
private: typedef EGWndPropControl Super;

public:

	EGWndPropControl_Transform( IWndPropControlOwner* Parent , const egPropControlInfo& Info );

	virtual LRESULT WndProc( UINT message , WPARAM wParam , LPARAM lParam ) override;
	virtual void CommitChangesToSourceData( HWND CommitControl ) override;
	virtual void RefreshFromSourceData() override;

protected:

	EGWndTextNode m_WndEditRotX;
	EGWndTextNode m_WndEditRotY;
	EGWndTextNode m_WndEditRotZ;

	HWND m_WndCommitRotX = nullptr;
	HWND m_WndCommitRotY = nullptr;
	HWND m_WndCommitRotZ = nullptr;
	HWND m_WndCommitRotReset = nullptr;

	HWND m_WndRotDisplay = nullptr;

	EGWndTextNode m_WndEditPosX;
	EGWndTextNode m_WndEditPosY;
	EGWndTextNode m_WndEditPosZ;

	eg_transform m_Transform;
};
