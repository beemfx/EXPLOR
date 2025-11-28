// (c) 2017 Beem Media

#include "EGWndPropControl.h"
#include "EGResourceLib.h"
#include "../../tools/EGEdResLib/resource.h"
#include "EGFileData.h"
#include "EGCopyPasteDeserializer.h"

EGWndPropControl::EGWndPropControl( IWndPropControlOwner* Parent , const egPropControlInfo& Info )
: Super( Parent->GetOwningWnd() )
, m_ControlInfo( Info )
, m_Parent( Parent )
{
	m_Height = FONT_SIZE*2;
	m_BkBrush = EGWnd_GetBrush( egw_color_t::BG_STATIC );
	m_Font = EGWnd_GetFont( egw_font_t::DEFAULT_BOLD );
}

EGWndPropControl::~EGWndPropControl()
{

}

LRESULT EGWndPropControl::WndProc( UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
	case WM_EG_CONTROL_CHANGED:
	{
		HWND WndControl = reinterpret_cast<HWND>( lParam );
		if( !m_bIsDirty )
		{
			m_bIsDirty = true;
			InvalidateRect( WndControl, nullptr, true );
			UpdateWindow( WndControl );
		}
	} break;
	case WM_LBUTTONDOWN:
	case WM_NCLBUTTONDOWN:
	{
		EGWnd_ClearFocus();
	} return Super::WndProc( message, wParam, lParam);
	case WM_CTLCOLORBTN:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
	{ 
		HDC hdc = reinterpret_cast<HDC>(wParam);
		HWND Wnd = reinterpret_cast<HWND>(lParam);
		egw_color_t Color = m_bIsDirty && WM_CTLCOLOREDIT == message && m_bDrawDirtyControls ? egw_color_t::BG_EDIT_DIRTY : egw_color_t::BG_EDIT;
		SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_EDIT) );
		SetBkColor( hdc , EGWnd_GetColor( Color ) );
		return reinterpret_cast<LRESULT>(EGWnd_GetBrush( Color ) );
	} break;
	case WM_CTLCOLORSTATIC:
	{ 
		HDC hdc = reinterpret_cast<HDC>(wParam);
		HWND Wnd = reinterpret_cast<HWND>(lParam);
		SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_STATIC ) );
		SetBkColor( hdc , EGWnd_GetColor( egw_color_t::BG_STATIC ) );
		return reinterpret_cast<LRESULT>(EGWnd_GetBrush( egw_color_t::BG_STATIC ) );
	} break;
	default: return Super::WndProc( message , wParam , lParam );
	}

	return 0l;
}

void EGWndPropControl::OnWmSize( const eg_ivec2& NewClientSize )
{
	Super::OnWmSize( NewClientSize );
}

void EGWndPropControl::OnPaint( HDC hdc )
{
	const RECT rcView = GetViewRect();
	SetBkMode( hdc , TRANSPARENT );
	SetBrush( hdc , m_BkBrush );
	SetFont( hdc , m_Font );
	SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_STATIC ) );
	//Rectangle( hdc , rcView.left , rcView.top , rcView.right , rcView.bottom );
	RECT rcText = rcView;
	rcText.left += 2;
	rcText.top += 1;
	DrawTextA( hdc , m_ControlInfo.DisplayName , -1 , &rcText , DT_SINGLELINE );
}

void EGWndPropControl::OnDrawBg( HDC hdc )
{
	FillRect( hdc , &GetViewRect() , EGWnd_GetBrush( egw_color_t::BG_STATIC ) );
}

void EGWndPropControl::OnWmRButtonUp( const eg_ivec2& MousePos )
{
	unused( MousePos );

	HMENU ContextMenu = LoadMenuW( EGResourceLib_GetLibrary() , L"EGED_PROP_CONTEXT" );
	POINT HitPos;
	GetCursorPos( &HitPos );
	TrackPopupMenu( GetSubMenu( ContextMenu , IsArrayChild() ? 2 : 0 ) , TPM_RIGHTBUTTON , HitPos.x , HitPos.y , 0 , GetWnd() , nullptr );
}

void EGWndPropControl::OnWmMenuCmd( eg_int CmdId, eg_bool bFromAccelerator )
{
	switch( CmdId )
	{
		case ID_COPYPASTE_COPY:
			CopyToClipboard();
			break;
		case ID_COPYPASTE_PASTE:
			PasteFromClipboard();
			break;
		case ID_COPYPASTE_CUT:
			CopyToClipboard();
			DeleteFromArray();
			break;
		case ID_COPYPASTE_DELETE:
			DeleteFromArray();
			break;
		case ID_COPYPASTE_INSERT:
			InsertBeforeArray();
			break;
		default:
			Super::OnWmMenuCmd( CmdId, bFromAccelerator );
			break;
	}
}

void EGWndPropControl::CopyToClipboard()
{
	if( OpenClipboard( GetWnd() ) )
	{
		EGFileData ClipboardData( eg_file_data_init_t::HasOwnMemory );
		if( m_ControlInfo.Editor )
		{
			m_ControlInfo.Editor->Serialize( eg_rfl_serialize_fmt::XML , 0 , ClipboardData );
		}
		eg_char8 Terminal = '\0';
		ClipboardData.Write( &Terminal , sizeof(Terminal) );

		if( ClipboardData.GetSize() > 0 )
		{
			HGLOBAL ClipboardMem = GlobalAlloc( GMEM_MOVEABLE|GMEM_ZEROINIT , ClipboardData.GetSize() );
			if( ClipboardMem )
			{
				void* Buffer = GlobalLock( ClipboardMem );
				if( Buffer )
				{
					EGMem_Copy( Buffer , ClipboardData.GetData() , ClipboardData.GetSize() );
					GlobalUnlock( ClipboardMem );
				}
				HANDLE ClipboardHandle = SetClipboardData( EGWnd_GetEditorSerializationClipboardFormat() , ClipboardMem );
				if( nullptr == ClipboardHandle )
				{
					MessageBoxW( GetWnd() , L"Failed to copy to clipboard." , L"Warning" , MB_OK );
				}
			}
		}

		CloseClipboard();
	}
}

void EGWndPropControl::DeleteFromArray()
{
	if( m_ControlInfo.Parent )
	{
		m_ControlInfo.Parent->DeleteArrayChild( m_ControlInfo.Editor );
		m_Parent->OnPropChangedValue( m_ControlInfo.Parent );
		m_ControlInfo.Editor = nullptr;
	}

	m_Parent->PropEditorRebuild();
}

void EGWndPropControl::InsertBeforeArray()
{
	if( m_ControlInfo.Parent )
	{
		m_ControlInfo.Parent->InsertArrayChildBefore( m_ControlInfo.Editor );
		m_Parent->OnPropChangedValue( m_ControlInfo.Parent );
		m_ControlInfo.Editor = nullptr;
	}

	m_Parent->PropEditorRebuild();
}

void EGWndPropControl::PasteFromClipboard()
{
	if( OpenClipboard( GetWnd() ) )
	{
		HANDLE ClipboardHandle = GetClipboardData( EGWnd_GetEditorSerializationClipboardFormat() );
		if( ClipboardHandle )
		{
			const eg_char* Buffer = static_cast<const eg_char*>(GlobalLock( ClipboardHandle ));
			if( Buffer )
			{
				eg_size_t BufferSize = 0;
				for( eg_size_t i=0; ; i++ )
				{
					if( Buffer[i] == '\0' )
					{
						BufferSize = i;
						break;
					}
				}

				if( m_ControlInfo.Editor )
				{
					EGCopyPasteRflDeserializer( m_ControlInfo.Editor , Buffer , BufferSize );
				}

				m_Parent->PropEditorRebuild();
				
				GlobalUnlock( ClipboardHandle );
			}
		}
		else
		{
			MessageBoxW( GetWnd() , L"Nothing to paste." , L"Warning" , MB_OK );
		}

		CloseClipboard();

	}
}

HWND EGWndPropControl::CreateStaticText( eg_int x, eg_int y, eg_int Width, eg_int Height, eg_cpstr16 Text, HFONT Font /*= nullptr */ )
{
	HWND WndOut = CreateWindowExW(
		0, 
		WC_STATICW,
		Text,
		WS_VISIBLE|WS_CHILD,
		x,
		y,
		Width,
		Height,
		GetWnd(),
		NULL,
		NULL,
		NULL);

	if( Font != nullptr )
	{
		SendMessage( WndOut , WM_SETFONT , reinterpret_cast<WPARAM>(m_Font) , MAKEWORD(1,0) );
	}

	return WndOut;
}

HWND EGWndPropControl::CreateButtonControl( eg_int x, eg_int y, eg_int Width, eg_int Height, eg_cpstr16 Text, HFONT Font /*= nullptr */ )
{
	HWND WndOut = CreateWindowExW(
		0, 
		WC_BUTTONW,
		Text,
		WS_VISIBLE|WS_CHILD/*|BS_OWNERDRAW*/, // TODO: Owner draw.
		x,
		y,
		Width,
		Height,
		GetWnd(),
		NULL,
		NULL,
		NULL);

	SetWindowSubclass( WndOut , ButtonControlSubproc , 0 , 0 );
	EGWnd_RegisterTabTo( WndOut );

	if( Font != nullptr )
	{
		SendMessage( WndOut , WM_SETFONT , reinterpret_cast<WPARAM>(m_Font) , MAKEWORD(1,0) );
	}

	return WndOut;
}

HWND EGWndPropControl::CreateCheckboxControl( eg_int x, eg_int y, eg_int Width, eg_int Height, eg_cpstr16 Text, HFONT Font /*= nullptr */ )
{
	HWND WndOut = CreateWindowExW(
		0, 
		WC_BUTTONW,
		Text,
		WS_VISIBLE|WS_CHILD|BS_CHECKBOX,
		x,
		y,
		Width,
		Height,
		GetWnd(),
		NULL,
		NULL,
		NULL);

	SetWindowSubclass( WndOut , CheckboxControlSubproc , 0 , 0 );
	EGWnd_RegisterTabTo( WndOut );

	if( Font != nullptr )
	{
		SendMessage( WndOut , WM_SETFONT , reinterpret_cast<WPARAM>(m_Font) , MAKEWORD(1,0) );
	}

	return WndOut;
}

HWND EGWndPropControl::CreateComboBoxControl( eg_int x , eg_int y , eg_int Width , eg_int Height , eg_bool bCanManualEdit , HFONT Font /*= nullptr */ )
{
	DWORD ComboFlags = bCanManualEdit ? CBS_DROPDOWN : CBS_DROPDOWNLIST;

	HWND WndOut = CreateWindowExW(
		0, 
		WC_COMBOBOXW,
		L"",
		WS_VISIBLE|WS_CHILD|WS_OVERLAPPED|ComboFlags|CBS_HASSTRINGS|WS_VSCROLL|CBS_AUTOHSCROLL,
		x,
		y,
		Width,
		Height,
		GetWnd(),
		NULL,
		NULL,
		NULL);

	SetWindowSubclass( WndOut , ComboBoxControlSubproc , 0 , 0 );
	EGWnd_RegisterTabTo( WndOut );

	HWND WndEdit = FindWindowExW( WndOut , NULL , WC_EDITW , NULL );
	if( WndEdit )
	{
		SetWindowSubclass( WndEdit , ComboBoxEditControlSubproc , 0 , 0 );
		EGWnd_RegisterTabTo( WndEdit );
	}

	if( Font != nullptr )
	{
		SendMessageW( WndOut , WM_SETFONT , reinterpret_cast<WPARAM>(m_Font) , MAKEWORD(1,0) );
	}

	return WndOut;
}

void EGWndPropControl::SetupEditControl( EGWndTextNode& EditText, eg_int x, eg_int y, eg_int Width, eg_int Height, eg_bool bMultiline, HFONT Font )
{
	EditText.SetWndRect( eg_recti( x , y , x + Width , y + Height ) );
	EditText.SetFont( Font );
	EditText.SetMultiline( bMultiline );
	EditText.FullRedraw();
}

eg_bool EGWndPropControl::IsArrayChild() const
{
	eg_bool bIsEditorArrayChild = m_ControlInfo.Parent && m_ControlInfo.Parent->GetType() == eg_rfl_value_t::Array;
	return bIsEditorArrayChild;
}

LRESULT CALLBACK EGWndPropControl::ComboBoxControlSubproc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData )
{
	unused( dwRefData , uIdSubclass );

	if( TabToProcHandler( hWnd , uMsg , wParam , lParam ) )
	{
		return 0;
	}

	switch( uMsg )
	{
	case WM_MOUSEWHEEL:
	{
		// Pass mouse wheel messages to parent.
		PostMessageW( GetParent(hWnd) , uMsg , wParam , lParam );
	} return 0;
	case WM_KEYDOWN:
	{
		if( wParam == VK_RETURN )
		{
			if( (lParam&(1<<30)) == 0 )
			{
				PostMessageW( GetParent( hWnd ) , WM_EG_COMMIT , 0 , reinterpret_cast<LPARAM>(hWnd) );
				return 0;
			}
		}
		PostMessageW( GetParent( hWnd ) , WM_EG_CONTROL_CHANGED , 0 , reinterpret_cast<LPARAM>(hWnd) );
	} break;
	case WM_EG_COMMIT_ON_FOCUS_LOST:
	case WM_EG_CONTROL_CHANGED:
	case WM_EG_COMMIT:
	{
		// Just pass it forward.
		PostMessageW( GetParent( hWnd ) , uMsg , wParam , lParam );
	} return 0;
	}
	return DefSubclassProc( hWnd , uMsg , wParam , lParam );
}

LRESULT CALLBACK EGWndPropControl::ComboBoxEditControlSubproc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData )
{
	unused( dwRefData , uIdSubclass );

	if( TabToProcHandler( hWnd , uMsg , wParam , lParam ) )
	{
		return 0;
	}

	switch( uMsg )
	{
	case WM_KILLFOCUS:
	{
		PostMessageW( GetParent( hWnd ), WM_EG_COMMIT_ON_FOCUS_LOST, 0, reinterpret_cast<LPARAM>( hWnd ) );
	} break;
	case WM_KEYDOWN:
	{
		if( wParam == VK_RETURN )
		{
			if( (lParam&(1<<30)) == 0 )
			{
				PostMessageW( GetParent( hWnd ) , WM_EG_COMMIT , 0 , reinterpret_cast<LPARAM>(hWnd) );
				return 0;
			}
		}
		PostMessageW( GetParent( hWnd ) , WM_EG_CONTROL_CHANGED , 0 , reinterpret_cast<LPARAM>(hWnd) );
	} break;
	}
	return DefSubclassProc( hWnd , uMsg , wParam , lParam );
}

LRESULT CALLBACK EGWndPropControl::CheckboxControlSubproc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData )
{
	unused( dwRefData , uIdSubclass );

	if( TabToProcHandler( hWnd , uMsg , wParam , lParam ) )
	{
		return 0;
	}

	return DefSubclassProc( hWnd , uMsg , wParam , lParam );
}

LRESULT CALLBACK EGWndPropControl::ButtonControlSubproc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData )
{
	unused( dwRefData , uIdSubclass );

	if( TabToProcHandler( hWnd , uMsg , wParam , lParam ) )
	{
		return 0;
	}

	return DefSubclassProc( hWnd , uMsg , wParam , lParam );
}
