// (c) 2017 Beem Media

#pragma once

#include "EGWndTextNode.h"
#include "EGWndPropControl.h"

enum class egwnd_prop_control_vector_t
{
	Real,
	Int,
};

class EGWndPropControl_Vector : public EGWndPropControl
{
EG_DECL_SUPER( EGWndPropControl )

public:

	EGWndPropControl_Vector( IWndPropControlOwner* Parent , const egPropControlInfo& Info , eg_int Size , egwnd_prop_control_vector_t Type );

	virtual LRESULT WndProc( UINT message , WPARAM wParam , LPARAM lParam ) override;
	virtual void OnWmSize( const eg_ivec2& NewClientSize ) override;
	virtual void OnPaint( HDC hdc ) override;
	virtual void CommitChangesToSourceData( HWND CommitControl ) override;
	virtual void RefreshFromSourceData() override;

protected:

	eg_int GetEditWidth() const;
	eg_int GetPadding() const;

protected:

	const eg_int m_Size;
	const egwnd_prop_control_vector_t m_Type;

	EGWndTextNode m_WndEditX = nullptr;
	EGWndTextNode m_WndEditY = nullptr;
	EGWndTextNode m_WndEditZ = nullptr;
	EGWndTextNode m_WndEditW = nullptr;
};
