// (c) 2017 Beem Media

#include "EGWndPropControl_Bool.h"

EGWndPropControl_Bool::EGWndPropControl_Bool( IWndPropControlOwner* Parent, const egPropControlInfo& Info )
: Super( Parent , Info )
{
	m_Height = FONT_SIZE;
	m_WndCheck = CreateCheckboxControl( 0 , 0 , 0 , FONT_SIZE , EGString_ToWide( Info.DisplayName ) , m_Font );

	RefreshFromSourceData();
}

void EGWndPropControl_Bool::OnWmControlCmd( HWND WndControl, eg_int NotifyId, eg_int ControlId )
{
	if( NotifyId == BN_CLICKED )
	{
		if( WndControl == m_WndCheck )
		{
			CommitChangesToSourceData( WndControl );
			return;
		}
	}
	Super::OnWmControlCmd( WndControl , NotifyId , ControlId );
}

void EGWndPropControl_Bool::OnWmSize( const eg_ivec2& NewClientSize )
{
	Super::OnWmSize( NewClientSize );

	SetWindowPos( m_WndCheck , nullptr , 2 , 0 , m_EditorWidth , FONT_SIZE , SWP_NOZORDER );
}

void EGWndPropControl_Bool::OnPaint( HDC hdc )
{
	unused( hdc );
}

void EGWndPropControl_Bool::CommitChangesToSourceData( HWND CommitControl )
{
	unused( CommitControl );

	eg_bool bChecked = BST_CHECKED == Button_GetCheck( m_WndCheck );
	// This event comes from the click so we just want to toggle it.
	if( m_ControlInfo.Editor )
	{
		m_ControlInfo.Editor->SetFromString( !bChecked ? "TRUE" : "FALSE" );
		m_Parent->OnPropChangedValue( m_ControlInfo.Editor );
	}
	RefreshFromSourceData();
}

void EGWndPropControl_Bool::RefreshFromSourceData()
{
	eg_bool bChecked = false;
	if( m_ControlInfo.Editor )
	{
		bChecked = EGString_ToBool( *m_ControlInfo.Editor->ToString() );
	}
	Button_SetCheck( m_WndCheck , bChecked ? BST_CHECKED : BST_UNCHECKED );
}
