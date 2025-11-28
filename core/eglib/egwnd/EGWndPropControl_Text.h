// (c) 2017 Beem Media

#pragma once

#include "EGWndPropControl.h"
#include "EGWndTextNode.h"

class EGWndPropControl_Text : public EGWndPropControl
{
	EG_DECL_SUPER( EGWndPropControl )

public:

	enum class eg_t
	{
		TEXT,
		INT,
		REAL,
		ANGLE,
		TEXT_BLOCK,
		STATIC_TEXT,
	};

public:

	EGWndPropControl_Text( IWndPropControlOwner* Parent , const egPropControlInfo& Info , eg_t Type );

	virtual LRESULT WndProc( UINT message , WPARAM wParam , LPARAM lParam ) override;
	virtual void OnWmSize( const eg_ivec2& NewClientSize ) override;
	virtual void CommitChangesToSourceData( HWND CommitControl ) override;
	virtual void RefreshFromSourceData() override;

protected:

	EGWndTextNode m_TextEdit;
	eg_t          m_Type = eg_t::TEXT;
};

///////////////////////////////////////////////////////////////////////////////

class EGWndPropControl_Filename : public EGWndPropControl_Text
{
	EG_DECL_SUPER( EGWndPropControl_Text )

public:

	enum class eg_t
	{
		Save,
		Open,
		BrowseDir,
	};

public:

	EGWndPropControl_Filename( IWndPropControlOwner* Parent , const egPropControlInfo& Info , eg_t Type , eg_cpstr Ext );

	virtual LRESULT WndProc( UINT message , WPARAM wParam , LPARAM lParam ) override;
	virtual void OnWmSize( const eg_ivec2& NewClientSize ) override;

protected:

	HWND      m_WndButton = nullptr;
	eg_t      m_Type = eg_t::Save;
	eg_string m_Ext;
};