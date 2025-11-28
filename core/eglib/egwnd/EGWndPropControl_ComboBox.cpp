// (c) 2017 Beem Media

#include "EGWndPropControl_ComboBox.h"

EGWndPropControl_ComboBox::EGWndPropControl_ComboBox( IWndPropControlOwner* Parent, const egPropControlInfo& Info )
: Super( Parent , Info )
{
	m_bDrawDirtyControls = true;

	m_Height = FONT_SIZE*3;

	m_Choices.Clear();
	eg_bool bManualEdit = true;
	if( m_ControlInfo.Editor )
	{
		EGArray<eg_d_string> EdChoices;
		m_ControlInfo.Editor->GetComboChoices( EdChoices , bManualEdit );
		for( const eg_d_string& Str : EdChoices )
		{
			m_Choices.Append( *Str );
		}
	}

	m_WndComboBox = CreateComboBoxControl( 0 , FONT_SIZE , 0 , 0 , bManualEdit , m_Font );
	m_WndComboBoxEdit = FindWindowExW( m_WndComboBox , NULL , WC_EDITW , NULL );
	ComboBox_ResetContent( m_WndComboBox );

	for( const eg_string_big& String : m_Choices )
	{
		ComboBox_AddString( m_WndComboBox , EGString_ToWide(String) );
	}

	RefreshFromSourceData();
}

LRESULT EGWndPropControl_ComboBox::WndProc( UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
	case WM_KEYDOWN:
	{
		assert( false );
	} break;
	case WM_EG_COMMIT_ON_FOCUS_LOST:
	case WM_EG_COMMIT:
	{
		CommitChangesToSourceData( reinterpret_cast<HWND>( lParam ) );
	} break;
	}

	return Super::WndProc( message , wParam , lParam );
}

void EGWndPropControl_ComboBox::OnWmControlCmd( HWND WndControl, eg_int NotifyId, eg_int ControlId )
{
	if( NotifyId == CBN_SELCHANGE )
	{
		PostMessageW( GetWnd() , WM_EG_COMMIT , 0 , reinterpret_cast<LPARAM>(WndControl) );
	}

	Super::OnWmControlCmd( WndControl , NotifyId , ControlId );
}

void EGWndPropControl_ComboBox::OnWmSize( const eg_ivec2& NewClientSize )
{
	Super::OnWmSize( NewClientSize );

	if( m_WndComboBox )
	{
		RECT rcDesktop;
		GetClientRect( GetDesktopWindow() , &rcDesktop );
		eg_int TextEditWidth = NewClientSize.x-4;
		SetWindowPos( m_WndComboBox , nullptr , 2 , FONT_SIZE , TextEditWidth , (rcDesktop.bottom-rcDesktop.top)/2 , SWP_NOZORDER);
		Edit_SetSel( m_WndComboBoxEdit , -1 , -1 );
	}
}

void EGWndPropControl_ComboBox::CommitChangesToSourceData( HWND CommitControl )
{
	eg_string_big OldText;
	if( m_ControlInfo.Editor )
	{
		OldText = *m_ControlInfo.Editor->ToString();
	}
	eg_string_big NewText = GetWndText( m_WndComboBox );

	if( OldText != NewText )
	{
		if( m_ControlInfo.Editor )
		{
			m_ControlInfo.Editor->SetFromString( NewText );
			m_Parent->OnPropChangedValue( m_ControlInfo.Editor );
		}

		RefreshFromSourceData();
		// Refreshing makes the cursor move to the beginning so we'll move it to the end
		if( CommitControl == m_WndComboBoxEdit )
		{
			Edit_SetSel( m_WndComboBoxEdit , 0 , -1 );
			Edit_SetSel( m_WndComboBoxEdit , -1 , -1 );
		}
		else
		{
			Edit_SetSel( m_WndComboBoxEdit , 0 , -1 );
		}
	}
}

void EGWndPropControl_ComboBox::RefreshFromSourceData()
{	
	auto ContainsString = [this]( eg_cpstr What , eg_uint* IndexOut ) -> eg_bool
	{
		for( eg_uint i=0; i<m_Choices.Len(); i++ )
		{
			const eg_string_big& String = m_Choices[i];
		
			if( What == String )
			{
				*IndexOut = i;
				return true;
			}
		}
		return false;
	};
	eg_string_big TextString;
	if( m_ControlInfo.Editor )
	{
		TextString = *m_ControlInfo.Editor->ToString();
	}

	eg_uint StringIndex = 0;
	if( ContainsString( TextString , &StringIndex ) )
	{
		ComboBox_SetCurSel( m_WndComboBox , StringIndex );
	}
	else
	{
		SetWindowTextW( m_WndComboBox , EGString_ToWide(TextString) );
	}

	m_bIsDirty = false;
}