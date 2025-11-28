// (c) 2017 Beem  Media

#include "EGWndPropControlTypes.h"
#include "EGWndHelpers.h"

///////////////////////////////////////////////////////////////////////////////
// EGWndPropControl_Header
///////////////////////////////////////////////////////////////////////////////

EGWndPropControl_Header::EGWndPropControl_Header( IWndPropControlOwner* Parent, const egPropControlInfo& Info )
: Super( Parent , Info )
, m_HeaderText( Info.DisplayName )
{
	m_Height = FONT_SIZE*2;
}

void EGWndPropControl_Header::OnPaint( HDC hdc )
{
	RECT rc = GetViewRect();

	SetTextColor( hdc , RGB(255,0,0) );
	SetBkMode( hdc , TRANSPARENT );
	DrawTextA( hdc , m_HeaderText , -1 , &rc , DT_CENTER|DT_BOTTOM|DT_SINGLELINE );
}

///////////////////////////////////////////////////////////////////////////////
// EGWndPropControl_Button
///////////////////////////////////////////////////////////////////////////////

EGWndPropControl_Button::EGWndPropControl_Button( IWndPropControlOwner* Parent , const egPropControlInfo& Info )
: Super( Parent , Info )
{
	m_bDrawDirtyControls = true;

	m_Height = 40;

	m_WndButton = CreateButtonControl( 2 , 2 , 200 , m_Height , EGString_ToWide(Info.DisplayName) , m_Font );

	RefreshFromSourceData();
}

void EGWndPropControl_Button::OnWmControlCmd( HWND WndControl, eg_int NotifyId, eg_int ControlId )
{
	unused( NotifyId , ControlId );

	if( m_WndButton == WndControl )
	{
		if( m_ControlInfo.Editor )
		{
			m_Parent->OnPropControlButtonPressed( m_ControlInfo.Editor );
		}
	}
}

void EGWndPropControl_Button::RefreshFromSourceData()
{
	eg_string_big TextString;
	if( m_ControlInfo.Editor )
	{
		TextString = *m_ControlInfo.Editor->ToString();
	}
	SetWindowTextA( m_WndButton , TextString );
}

///////////////////////////////////////////////////////////////////////////////
// EGWndPropControl_Button
///////////////////////////////////////////////////////////////////////////////

EGWndPropControl_Unk::EGWndPropControl_Unk( IWndPropControlOwner* Parent, const egPropControlInfo& Info )
: Super( Parent , Info )
{
	m_Height = FONT_SIZE + 2;
	m_Pen = CreatePen( PS_SOLID , 1 , EGWnd_GetColor( egw_color_t::GARBAGE ) );
}

EGWndPropControl_Unk::~EGWndPropControl_Unk()
{
	DeleteObject( m_Pen );
}

void EGWndPropControl_Unk::OnPaint( HDC hdc )
{
	const RECT rcView = GetViewRect();

	SetBkMode( hdc , OPAQUE );
	SetBrush( hdc , EGWnd_GetBrush( egw_color_t::BG_EDIT ) );
	SetFont( hdc , m_Font );
	SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_STATIC ) );
	SetPen( hdc , m_Pen );

	DrawRect( hdc , rcView );

	SetBkMode( hdc , TRANSPARENT );

	RECT rcText = rcView;
	rcText.left += 2;
	rcText.top += 1;


	eg_d_string LeftText = *m_ControlInfo.DisplayName;// EGString_Format( "%s Width %d" , *m_ControlInfo.DisplayName , m_EditorWidth );
	if( m_ControlInfo.Editor && m_ControlInfo.Editor->IsPrimitive() )
	{
		LeftText.Append( ": " );
		LeftText.Append( *m_ControlInfo.Editor->ToString() );
	}
	DrawTextA( hdc , *LeftText , -1 , &rcText , DT_SINGLELINE );

	// eg_string RightText = EGString_Format( "Right %d" , m_EditorWidth );
	// DrawTextA( hdc , RightText , -1 , &rcText , DT_SINGLELINE|DT_RIGHT );
}
