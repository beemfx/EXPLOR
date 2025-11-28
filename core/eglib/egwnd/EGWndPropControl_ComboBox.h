// (c) 2017 Beem Media

#pragma once

#include "EGWndPropControl.h"

class EGWndPropControl_ComboBox : public EGWndPropControl
{
	EG_DECL_SUPER( EGWndPropControl )

public:

	EGWndPropControl_ComboBox( IWndPropControlOwner* Parent , const egPropControlInfo& Info );

	virtual LRESULT WndProc( UINT message , WPARAM wParam , LPARAM lParam ) override;
	virtual void OnWmControlCmd( HWND WndControl , eg_int NotifyId , eg_int ControlId ) override;
	virtual void OnWmSize( const eg_ivec2& NewClientSize ) override;
	virtual void CommitChangesToSourceData( HWND CommitControl ) override;
	virtual void RefreshFromSourceData() override;

protected:

	HWND    m_WndComboBox = nullptr;
	HWND    m_WndComboBoxEdit = nullptr;
	EGArray<eg_string_big> m_Choices;
};
