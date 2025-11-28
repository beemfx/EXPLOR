// (c) 2017 Beem Media

#include "EGWndPropControl_Containers.h"

EGWndPropControl_Container::EGWndPropControl_Container( IWndPropControlOwner* Parent, const egPropControlInfo& Info )
: Super( Parent, Info )
{
	m_WndBtnExpand = CreateButtonControl( 0 , 2 , EXPAND_BTN_SIZE , EXPAND_BTN_SIZE , L"-" , nullptr );

	RefreshExpandBtnText();
}

void EGWndPropControl_Container::OnPaint( HDC hdc )
{
	RECT rc = GetViewRect();
	InflateRect( &rc , -(EXPAND_BTN_SIZE+2) , 0 );
	SetTextColor( hdc , m_HeaderColor );
	SetBkMode( hdc , TRANSPARENT );
	eg_string_small PreviewValue( CT_Clear );
	
	if( m_ControlInfo.Editor && !m_ControlInfo.Editor->IsExpanded() )
	{
		PreviewValue = *m_ControlInfo.Editor->GetPreviewValue();
	}
	eg_string_small DisplayName = PreviewValue.Len() > 0 ? EGString_Format( "%s (%s)" , m_ControlInfo.DisplayName.String() , PreviewValue.String() ) : m_ControlInfo.DisplayName;
	DrawTextA( hdc , DisplayName , DisplayName.Len() , &rc , DT_LEFT|DT_TOP|DT_SINGLELINE );
}

void EGWndPropControl_Container::OnWmControlCmd( HWND WndControl, eg_int NotifyId, eg_int ControlId )
{
	if( m_WndBtnExpand == WndControl )
	{
		if( m_ControlInfo.Editor )
		{
			m_ControlInfo.Editor->SetExpanded( !m_ControlInfo.Editor->IsExpanded() );
			m_Parent->OnPropChangedValue( m_ControlInfo.Editor );
		}
		m_Parent->PropEditorRebuild();
		RefreshExpandBtnText();
		return;
	}
	Super::OnWmControlCmd( WndControl , NotifyId , ControlId );
}

eg_bool EGWndPropControl_Container::IsExpanded() const
{
	eg_bool bIsExpanded = true;

	if( m_WndBtnExpand && m_Parent )
	{
		if( m_ControlInfo.Editor )
		{
			bIsExpanded = m_ControlInfo.Editor->IsExpanded();
		}
	}

	return bIsExpanded;
}

void EGWndPropControl_Container::RefreshExpandBtnText()
{
	SetWindowTextW( m_WndBtnExpand , IsExpanded() ? L"-" : L"+" );
}

///////////////////////////////////////////////////////////////////////////////

EGWndPropControl_Array::EGWndPropControl_Array( IWndPropControlOwner* Parent, const egPropControlInfo& Info )
: Super( Parent , Info )
{
	m_HeaderColor = RGB(0,255,0);
	m_Height = EGWnd_GetAppScaledSize( 40 );
	m_WndBtnAppend = CreateButtonControl( 5 , FONT_SIZE+4 , 20 , 20 , L"+" , nullptr );
	m_WndBtnRemove = CreateButtonControl( 5 + 25 , FONT_SIZE+4 , 20 , 20 ,  L"-" , nullptr );

	RefreshExpandBtnText();
}

void EGWndPropControl_Array::OnWmControlCmd( HWND WndControl, eg_int NotifyId, eg_int ControlId )
{
	unused( NotifyId , ControlId );

	if( m_WndBtnAppend == WndControl )
	{
		// EGLogf( eg_log_t::General , "Appending!" );
		if( m_ControlInfo.Editor )
		{
			m_ControlInfo.Editor->InsertArrayChildAt( m_ControlInfo.Editor->GetNumChildren() );
			m_Parent->OnPropChangedValue( m_ControlInfo.Editor );
		}
		m_Parent->PropEditorRebuild();
		return;
	}
	else if( m_WndBtnRemove == WndControl )
	{
		// EGLogf( eg_log_t::General , "Removing!" );
		if( m_ControlInfo.Editor )
		{
			if( m_ControlInfo.Editor->GetNumChildren() > 0 )
			{
				m_ControlInfo.Editor->DeleteArrayChildAt( m_ControlInfo.Editor->GetNumChildren() - 1 );
			}
			m_Parent->OnPropChangedValue( m_ControlInfo.Editor );
		}
		m_Parent->PropEditorRebuild();
		return;
	}

	Super::OnWmControlCmd( WndControl , NotifyId , ControlId );
}

void EGWndPropControl_Array::RefreshExpandBtnText()
{
	Super::RefreshExpandBtnText();

	eg_bool bIsExpanded = IsExpanded();

	ShowWindow( m_WndBtnAppend , bIsExpanded ? SW_SHOWNORMAL : SW_HIDE );
	ShowWindow( m_WndBtnRemove , SW_HIDE );// bIsExpanded ? SW_SHOWNORMAL : SW_HIDE );

	m_Height = EGWnd_GetAppScaledSize( bIsExpanded ? 40 : FONT_SIZE + 2 );
}

///////////////////////////////////////////////////////////////////////////////

EGWndPropControl_Struct::EGWndPropControl_Struct( IWndPropControlOwner* Parent, const egPropControlInfo& Info )
: Super( Parent , Info )
{
	m_HeaderColor = RGB(100,255,255);
	m_Height = FONT_SIZE + 2;
}
