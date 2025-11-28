// (c) 2018 Beem Media

#include "EGWndTextNodeWndControl.h"

const eg_recti EGWndTextNodeWndControl::DEFAULT_POSE = { 0 , 0 , 100 , 10 };

EGWndTextNodeWndControl::EGWndTextNodeWndControl( EGWndBase* Parent )
: EGWndTextNodeWndControl( Parent , DEFAULT_POSE ) 
{
}

EGWndTextNodeWndControl::EGWndTextNodeWndControl( EGWndBase* Parent, const eg_recti& Pose )
: EGWndChildBase( Parent )
{
	SetWndPos( Pose.left , Pose.top , Pose.GetWidth() , Pose.GetHeight() );
	RebuildControl();
}

EGWndTextNodeWndControl::~EGWndTextNodeWndControl()
{
	if( m_EditControl )
	{
		if( m_bTabRegistered )
		{
			EGWnd_UnregisterTabTo( m_EditControl );
			m_bTabRegistered = false;
		}
		DestroyWindow( m_EditControl );
	}
}

HBRUSH EGWndTextNodeWndControl::OnWmCtlColorEdit( HWND WndControl , HDC hdc )
{
	unused( WndControl );

	SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_EDIT ) );
	SetBkColor( hdc , EGWnd_GetColor( m_bDirty ? egw_color_t::BG_EDIT_DIRTY : egw_color_t::BG_EDIT ) );
	return EGWnd_GetBrush( m_bDirty ? egw_color_t::BG_EDIT_DIRTY : egw_color_t::BG_EDIT );
}

HBRUSH EGWndTextNodeWndControl::OnWmCtlColorStatic( HWND WndControl , HDC hdc )
{
	unused( WndControl );

	SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_STATIC ) );
	SetBkColor( hdc , EGWnd_GetColor( m_bDirty ? egw_color_t::BG_STATIC : egw_color_t::BG_STATIC ) );
	return EGWnd_GetBrush( m_bDirty ? egw_color_t::BG_STATIC : egw_color_t::BG_STATIC );
}

void EGWndTextNodeWndControl::OnWmControlCmd( HWND WndControl, eg_int NotifyId, eg_int ControlId )
{
	Super::OnWmControlCmd( WndControl , NotifyId , ControlId );

	if( WndControl == m_EditControl )
	{
		switch( NotifyId )
		{
			case EN_CHANGE:
			{
				SyncText();
				// EGLogf( eg_log_t::General , "Text Changed: %s" , *m_Text );
				SetDirty( true );
			} break;
		}
	}
}

void EGWndTextNodeWndControl::OnWmSize( const eg_ivec2& NewClientSize )
{
	Super::OnWmSize( NewClientSize );

	if( m_EditControl )
	{
		RECT EditControlSize;
		zero( &EditControlSize );
		EditControlSize.right = NewClientSize.x;
		EditControlSize.bottom = NewClientSize.y;
		InflateRect( &EditControlSize , -1 , -1 );
		SetWindowPos( m_EditControl , nullptr , EditControlSize.left , EditControlSize.top , EditControlSize.right-EditControlSize.left , EditControlSize.bottom-EditControlSize.top , SWP_NOZORDER );
	}
}

void EGWndTextNodeWndControl::OnWmEgCommit( HWND CommitCtrl )
{
	Super::OnWmEgCommit( CommitCtrl );

	if( CommitCtrl == m_EditControl )
	{
		PostMessageW( GetParent(GetWnd()) , WM_EG_COMMIT , 0 , reinterpret_cast<LPARAM>(GetWnd()) );
	}
}

void EGWndTextNodeWndControl::OnPaint( HDC hdc )
{
	RECT ViewRect = GetViewRect();
	eg_bool bIsFocused = m_EditControl && GetFocus() == m_EditControl;
	FillRect( hdc , &ViewRect , EGWnd_GetBrush( bIsFocused ? egw_color_t::BG_EDIT_HL : egw_color_t::BG_STATIC ) );
}

void EGWndTextNodeWndControl::SetText( eg_cpstr NewText )
{
	eg_int SavedCursorPos = GetCursorPos();
	m_Text = NewText;
	SetWindowTextA( m_EditControl , *m_Text );
	SetDirty( false );
	SetCursorPos( SavedCursorPos );
}

eg_string_big EGWndTextNodeWndControl::GetText() const
{
	return *m_Text;
}

void EGWndTextNodeWndControl::SetStatic( eg_bool bStatic )
{
	if( m_bStatic != bStatic )
	{
		m_bStatic = bStatic;
		RebuildControl();
	}
}

void EGWndTextNodeWndControl::SetCommitAction( eg_commit_action NewCommitAction )
{
	m_CommitAction = NewCommitAction;
}

void EGWndTextNodeWndControl::SetMultiline( eg_bool bMultiline )
{
	if( m_bMultiline != bMultiline )
	{
		m_bMultiline = bMultiline;
		RebuildControl();
	}
}

void EGWndTextNodeWndControl::SetWordWrap( eg_bool bNewWordWrap )
{
	if( m_bWordWrap != bNewWordWrap )
	{
		m_bWordWrap = bNewWordWrap;
		RebuildControl();
	}
}

void EGWndTextNodeWndControl::SetUseDirtyState( eg_bool bNewValue )
{
	m_bUseDirtyState = bNewValue; 
	if( !m_bUseDirtyState )
	{
		m_bDirty = false;
	}
	UpdateAppearance();
}

void EGWndTextNodeWndControl::SetFont( HFONT Font )
{
	if( m_Font != Font )
	{
		m_Font = Font;
		SendMessageW( m_EditControl , WM_SETFONT , reinterpret_cast<WPARAM>(Font) , TRUE );
	}
}

void EGWndTextNodeWndControl::SetDirty( eg_bool bDirty )
{
	if( m_bDirty != bDirty && m_bUseDirtyState )
	{
		m_bDirty = bDirty && m_bUseDirtyState;
		UpdateAppearance();
	}
}

void EGWndTextNodeWndControl::SetCursorStart()
{
	SetCursorPos( 0 );
}

void EGWndTextNodeWndControl::SetCursorEnd()
{
	SetCursorPos( m_Text.LenAs<eg_int>() );
}

void EGWndTextNodeWndControl::SetCursorPos( eg_int NewCursorPos )
{
	if( m_EditControl && !m_bStatic )
	{
		Edit_SetSel( m_EditControl , NewCursorPos , NewCursorPos );
	}
}

eg_int EGWndTextNodeWndControl::GetCursorPos()
{
	eg_int Pos = -1;
	if( m_EditControl && !m_bStatic )
	{
		DWORD Sel = Edit_GetSel( m_EditControl );
		Pos = LOWORD(Sel);
	}
	return Pos;
}

void EGWndTextNodeWndControl::UpdateAppearance()
{
	if( m_EditControl )
	{
		RedrawWindow( m_EditControl , nullptr , nullptr , RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW );
	}
	FullRedraw();
}

void EGWndTextNodeWndControl::RebuildControl()
{
	if( m_EditControl )
	{
		if( m_bTabRegistered )
		{
			EGWnd_UnregisterTabTo( m_EditControl );
			m_bTabRegistered = false;
		}
		DestroyWindow( m_EditControl );
		m_EditControl = nullptr;
	}

	DWORD dwStyles = WS_CHILD|WS_VISIBLE;

	if( m_bStatic )
	{
		if( !m_bMultiline )
		{
			dwStyles |= SS_ENDELLIPSIS;
		}
	}
	else
	{
		if( m_bMultiline )
		{
			dwStyles |= ES_MULTILINE;
			dwStyles |= WS_VSCROLL;
			dwStyles |= ES_AUTOVSCROLL;
			if( !m_bWordWrap )
			{
				dwStyles |= ES_AUTOHSCROLL;
				dwStyles |= WS_HSCROLL;
			}
		}
		else
		{
			dwStyles |= ES_AUTOHSCROLL;
		}

		dwStyles |= WS_BORDER;
	}

	RECT rcWnd = GetViewRect();
	m_EditControl = CreateWindowW( 
		m_bStatic ? L"STATIC" : L"EDIT" 
		, L"" 
		, dwStyles 
		, 1 , 1 , rcWnd.right-1 , rcWnd.bottom-1 
		, GetWnd() 
		, nullptr , nullptr , nullptr );

	if( m_EditControl && !m_bStatic )
	{
		m_bTabRegistered = true;
		EGWnd_RegisterTabTo( m_EditControl );
	}

	SetWindowTextA( m_EditControl , *m_Text );
	SendMessageW( m_EditControl , WM_SETFONT , reinterpret_cast<WPARAM>(m_Font) , TRUE );
	SetWindowSubclass( m_EditControl , EditControlSubclassProc , 0 , reinterpret_cast<DWORD_PTR>(this) );
}

void EGWndTextNodeWndControl::SyncText()
{
	eg_char TempStr[1024];
	TempStr[0] = '\0';
	GetWindowTextA( m_EditControl , TempStr , countof(TempStr) );
	m_Text = TempStr;
}

LRESULT EGWndTextNodeWndControl::EditControlSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData )
{
	unused( uIdSubclass );

	EGWndTextNodeWndControl* _this = reinterpret_cast<EGWndTextNodeWndControl*>(dwRefData);

	assert( _this->m_EditControl == hWnd );

	auto PostCommitMessage = [&_this,&hWnd]() -> void
	{
		_this->SetCursorEnd();
		SendMessageW( _this->GetWnd() , WM_EG_COMMIT , 0 , reinterpret_cast<LPARAM>(hWnd)  );
	};

	auto IsShiftHeld = []() -> eg_bool
	{
		return (GetKeyState ( VK_SHIFT )&0x8000) != 0;
	};

	auto IsControlHeld = []() -> eg_bool
	{
		return (GetKeyState ( VK_CONTROL )&0x8000) != 0;
	};

	switch( uMsg )
	{
		case WM_SETFOCUS:
		{
			_this->FullRedraw();
		} break;

		case WM_KILLFOCUS:
		{
			if( _this->m_bCommitOnFocusLost )
			{
				PostCommitMessage();
			}
			_this->FullRedraw();
		} break;

		case WM_CHAR:
		{
			if( wParam == VK_TAB )
			{
				if( !_this->m_bWantTab )
				{
					EGWnd_TabTo( hWnd , IsShiftHeld() ? -1 : 1 );
					return 0;
				}
			}
			else if( wParam == VK_ESCAPE )
			{
				EGWnd_ClearFocus();
				return 0;
			}
			else if( wParam == VK_RETURN )
			{
				eg_bool bShiftHeld = IsShiftHeld();

				switch( _this->m_CommitAction )
				{
				case eg_commit_action::None:
					break;
				case eg_commit_action::PressEnter:
					if( !bShiftHeld )
					{
						PostCommitMessage();
						return 0;
					}
					break;
				case eg_commit_action::PressShiftEnter:
					if( bShiftHeld )
					{
						PostCommitMessage();
						return 0;
					}
					break;
				}
			}
		} break;
	}

	return DefSubclassProc( hWnd , uMsg , wParam , lParam );
}
