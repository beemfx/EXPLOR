// (c) 2017 Beem Media

#include "EGWndPropControl_Text.h"
#include "EGWndHelpers.h"

EGWndPropControl_Text::EGWndPropControl_Text( IWndPropControlOwner* Parent, const egPropControlInfo& Info , eg_t Type )
: Super( Parent , Info )
, m_Type( Type )
, m_TextEdit( this )
{
	m_bDrawDirtyControls = true;

	eg_int TextBoxHeight = FONT_SIZE+8;
	eg_bool bMutiline = false;
	if( m_Type == eg_t::TEXT_BLOCK )
	{
		TextBoxHeight = FONT_SIZE*20;
		bMutiline = true;
	}
	m_Height = TextBoxHeight+FONT_SIZE+4;

	eg_int TextEditWidth = GetViewRect().right-4;

	if( m_Type == eg_t::INT || m_Type == eg_t::REAL )
	{
		TextEditWidth = 76;
	}

	RECT rcText = GetViewRect();
	rcText.left += 2;
	rcText.top += 1;

	m_TextEdit.SetStatic( Type == eg_t::STATIC_TEXT );
	m_TextEdit.SetMultiline( bMutiline );
	m_TextEdit.SetFont( m_Font );
	m_TextEdit.SetWndRect( eg_recti(2,FONT_SIZE+2,2+TextEditWidth,FONT_SIZE+2+TextBoxHeight) );
	m_TextEdit.SetText( Info.DisplayName );

	RefreshFromSourceData();
}

LRESULT EGWndPropControl_Text::WndProc( UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
	case WM_EG_CONTROL_CHANGED:
	{
		if( !m_bIsDirty )
		{
			eg_string_big OldText;
			if( m_ControlInfo.Editor )
			{
				OldText = *m_ControlInfo.Editor->ToString();
			}
			eg_string_big NewText = m_TextEdit.GetText();
			if( OldText != NewText )
			{
				m_bIsDirty = true;
				m_TextEdit.FullRedraw();
			}
		}
	} return 0;
	case WM_EG_COMMIT_ON_FOCUS_LOST:
	case WM_EG_COMMIT:
	{
		CommitChangesToSourceData( reinterpret_cast<HWND>(lParam) );
	} break;
	}

	return Super::WndProc( message , wParam , lParam );
}

void EGWndPropControl_Text::OnWmSize( const eg_ivec2& NewClientSize )
{
	Super::OnWmSize( NewClientSize );

	eg_int TextBoxHeight = FONT_SIZE+8;
	if( m_Type == eg_t::TEXT_BLOCK )
	{
		TextBoxHeight = FONT_SIZE*20;
	}
	m_Height = TextBoxHeight+FONT_SIZE+4;

	eg_int TextEditWidth = NewClientSize.x-4;

	if( m_Type == eg_t::INT || m_Type == eg_t::REAL )
	{
		TextEditWidth = 76;
	}

	RECT rcText = GetViewRect();
	rcText.left += 2;
	rcText.top += 1;

	m_TextEdit.SetWndRect( eg_recti(2,FONT_SIZE+2,2+TextEditWidth,FONT_SIZE+2+TextBoxHeight) );
}

void EGWndPropControl_Text::CommitChangesToSourceData( HWND CommitControl )
{
	unused( CommitControl );

	eg_string_big OldText;
	if( m_ControlInfo.Editor )
	{
		OldText = *m_ControlInfo.Editor->ToString();
	}

	eg_string_big NewText = m_TextEdit.GetText();
	m_TextEdit.SetDirty( false );
	
	if( OldText != NewText )
	{
		if( m_ControlInfo.Editor )
		{
			m_ControlInfo.Editor->SetFromString( NewText );
			m_Parent->OnPropChangedValue( m_ControlInfo.Editor );
		}
		RefreshFromSourceData();
	}
}

void EGWndPropControl_Text::RefreshFromSourceData()
{
	eg_string_big TextString;
	if( m_ControlInfo.Editor )
	{
		TextString = *m_ControlInfo.Editor->ToString();
	}
	m_TextEdit.SetText( TextString );
	m_bIsDirty = false;
}

///////////////////////////////////////////////////////////////////////////////
// EGWndPropControl_Filename
///////////////////////////////////////////////////////////////////////////////

EGWndPropControl_Filename::EGWndPropControl_Filename( IWndPropControlOwner* Parent , const egPropControlInfo& Info , eg_t Type , eg_cpstr Ext )
: Super( Parent , Info , EGWndPropControl_Text::eg_t::TEXT )
, m_Type( Type )
, m_Ext( Ext )
{
	m_WndButton = CreateButtonControl( 2 + 0 + 2 , FONT_SIZE+2 , 40 , m_Height , L"..." , m_Font ); 
}

LRESULT EGWndPropControl_Filename::WndProc( UINT message , WPARAM wParam , LPARAM lParam )
{
	switch( message )
	{
		case WM_COMMAND:
		{
			HWND WndCtrl = reinterpret_cast<HWND>(lParam);
			if( m_WndButton == WndCtrl )
			{
				eg_string_big CurText;
				if( m_ControlInfo.Editor )
				{
					CurText = *m_ControlInfo.Editor->ToString();
				}
				eg_char8 Filename[eg_string_big::STR_SIZE];
				CurText.CopyTo( Filename , countof(Filename) );
				
				eg_char8 Filter[eg_string_big::STR_SIZE];
				if( m_Ext.Len() > 0 )
				{
					EGString_FormatToBuffer( Filter , countof(Filter) , ".%s\n*.%s\n*.*\n*.*\n" , m_Ext.String() , m_Ext.String() );
				}
				else
				{
					EGString_Copy( Filter , "*.*\n*.*\n" , countof(Filter) );
				}
				eg_size_t StrLen = EGString_StrLen( Filter );
				for( eg_size_t i=0; i<StrLen; i++ )
				{
					if( Filter[i] == '\n' )
					{
						Filter[i] = '\0';
					}
				}
				eg_bool bGotFilename = false;
				
				if( m_Type == eg_t::BrowseDir )
				{
					bGotFilename = EGWndHelper_GetFolder( GetWnd() , Filename , countof(Filename) );
				}
				else
				{
					bGotFilename = EGWndHelper_GetFile( GetWnd() , Filename , countof(Filename) , m_Type == eg_t::Save , Filter , m_Ext );
				}

				if( bGotFilename )
				{
					m_TextEdit.SetText( Filename );
					CommitChangesToSourceData( m_TextEdit.GetWnd() );
				}
				return 0;
			}
		} break;
	}

	return Super::WndProc( message , wParam , lParam );
}

void EGWndPropControl_Filename::OnWmSize( const eg_ivec2& NewClientSize )
{
	Super::OnWmSize( NewClientSize );

	eg_int TextBoxHeight = FONT_SIZE + 8;
	eg_int TextEditWidth = NewClientSize.x - 44;

	m_TextEdit.SetWndPos( 2 , FONT_SIZE + 2 , TextEditWidth , TextBoxHeight );
	SetWindowPos( m_WndButton , nullptr , 2 + TextEditWidth + 2 , FONT_SIZE + 2 , 40 , TextBoxHeight , SWP_NOZORDER );
}
