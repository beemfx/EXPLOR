// (c) 2017 Beem Media

#pragma comment( lib , "Comctl32.lib" )

#include "EGWnd.h"
#include "EGWndPanel.h"
#include "EGWndPropControl.h"
#include "EGWndPropControl_Anchor.h"
#include "EGWndAppWindow.h"
#include "EGTimer.h"
#include "EGWndDraw.h"

static HINSTANCE g_hInst = nullptr;
static HMODULE g_RichEditLib = nullptr;

static UINT EGWnd_EditorSerializeClipboardFormat = 0;
static UINT EGWnd_EgsmNodeClipboardFormat = 0;

namespace EGWndRes
{
	eg_color32 Colors[enum_count(egw_color_t)];
	EGWndDcBrush Brushes[enum_count(egw_color_t)];
	EGWndDcFont Fonts[enum_count(egw_font_t)];
	eg_int FontSizes[enum_count(egw_font_t)];

	static void CreateResources();
	static void DestroyResources();

	static void CreateResources()
	{
		DestroyResources();

		Colors[enum_index(egw_color_t::DEFAULT)] = eg_color32(255,255,255);
		Colors[enum_index(egw_color_t::BG_STATIC)] = eg_color32(45,45,48);
		Colors[enum_index(egw_color_t::FG_STATIC)] = false ? eg_color32(255,180,0) : eg_color32(241,241,241);
		Colors[enum_index(egw_color_t::BG_EDIT)] = eg_color32(30,30,30);
		Colors[enum_index(egw_color_t::BG_EDIT_DIRTY)] = eg_color32(80,30,30);
		Colors[enum_index(egw_color_t::FG_EDIT)] = false ? eg_color32(189,183,107) : eg_color32(255,180,0);
		Colors[enum_index(egw_color_t::BG_EDIT_HL)] = eg_color32(255,255,255);
		Colors[enum_index(egw_color_t::FG_EDIT_HL)] = eg_color32(0,0,0);
		Colors[enum_index(egw_color_t::GARBAGE)] = eg_color32(255,0,255);
		Colors[enum_index(egw_color_t::BG_BTN)] = eg_color32(100,100,70);
		Colors[enum_index(egw_color_t::FG_BTN)] = eg_color32(255,255,255);
		Colors[enum_index(egw_color_t::BG_BTN_HOVER)] = eg_color32(100,100,0);
		Colors[enum_index(egw_color_t::FG_BTN_HOVER)] = eg_color32(255,255,255);
		Colors[enum_index(egw_color_t::BG_BTN_SELECTED)] = eg_color32(255,215,0);
		Colors[enum_index(egw_color_t::FG_BTN_SELECTED)] = eg_color32(0,0,0);
		Colors[enum_index(egw_color_t::BG_CONSOLE)] = eg_color32(10,10,10);

		static_assert( countof(Colors) == 16 , "Something needs to be added or removed." );

		FontSizes[enum_index(egw_font_t::DEFAULT)] = EGWnd_GetAppScaledSize(16);
		FontSizes[enum_index(egw_font_t::DEFAULT_BOLD)] = EGWnd_GetAppScaledSize(16);
		FontSizes[enum_index(egw_font_t::SMALL)] = EGWnd_GetAppScaledSize(12);
		FontSizes[enum_index(egw_font_t::SMALL_BOLD)] = EGWnd_GetAppScaledSize(12);
		FontSizes[enum_index(egw_font_t::CONSOLE)] = EGWnd_GetAppScaledSize(15);
		FontSizes[enum_index(egw_font_t::CODE_EDIT)] = EGWnd_GetAppScaledSize(16);

		Fonts[enum_index(egw_font_t::DEFAULT)].InitFont( L"Arial" , EGWnd_GetFontSize( egw_font_t::DEFAULT ) , false , false );
		Fonts[enum_index(egw_font_t::DEFAULT_BOLD)].InitFont( L"Arial" , EGWnd_GetFontSize( egw_font_t::DEFAULT_BOLD ) , true , false );
		Fonts[enum_index(egw_font_t::SMALL)].InitFont( L"Tahoma" , EGWnd_GetFontSize( egw_font_t::SMALL ) , false , false );
		Fonts[enum_index(egw_font_t::SMALL_BOLD)].InitFont( L"Tahoma" , EGWnd_GetFontSize( egw_font_t::SMALL_BOLD ) , true , false );
		Fonts[enum_index(egw_font_t::CONSOLE)].InitFont( CreateFontW( FontSizes[enum_index(egw_font_t::CONSOLE)] , 0 , 0 , 0 , FW_DEMIBOLD , FALSE, FALSE, FALSE , ANSI_CHARSET , 0 , 0 , DEFAULT_QUALITY , DEFAULT_PITCH , L"Courier New") );
		Fonts[enum_index(egw_font_t::CODE_EDIT)].InitFont( CreateFontW( FontSizes[enum_index(egw_font_t::CODE_EDIT)] , 0 , 0 , 0 , FW_DONTCARE , FALSE, FALSE, FALSE , ANSI_CHARSET , OUT_DEFAULT_PRECIS , CLIP_DEFAULT_PRECIS , DEFAULT_QUALITY , FIXED_PITCH , L"Lucida Sans Typewriter") );

		static_assert( countof(Fonts) == 6 , "Something needs to be added or removed." );

		for( eg_size_t i=0; i<countof(Brushes); i++ )
		{
			Brushes[i].InitSolidBrush( Colors[i] );
		}
	}

	static void DestroyResources()
	{
		for( eg_size_t i=0; i<countof(Brushes); i++ )
		{
			Brushes[i].Deinit();
		}

		for( eg_size_t i=0; i<countof(Fonts); i++ )
		{
			Fonts[i].Deinit();
		}

		for( eg_size_t i=0; i<countof(FontSizes); i++ )
		{
			FontSizes[i] = 12;
		}

		for( eg_size_t i=0; i<countof(Colors); i++ )
		{
			Colors[i] = eg_color32(0,0,0);
		}
	}
}

static const eg_size_t EGWND_MAX_TABTO_CONTROLS=1000;
static eg_real EGWND_APP_SCALING = 1.f;
static EGFixedArray<HWND,EGWND_MAX_TABTO_CONTROLS> EGWnd_TabToTable( CT_Clear );

void EGWnd_Init( HINSTANCE hInst )
{
#if(_WIN32_WINNT >= 0x0600)
	SetProcessDPIAware();
#endif

	{
		HDC AppDC = GetDC( NULL );
		EGWND_APP_SCALING = EGWnd_GetDPIScaling( AppDC );
		ReleaseDC( NULL , AppDC );
		AppDC = nullptr;
	}

	g_hInst = hInst;

	g_RichEditLib = LoadLibraryW( L"Msftedit.dll" );

	EGWndChildBase::InitClass( hInst );
	EGWndPropControl::InitClass( hInst );

	zero( &EGWndRes::Fonts );
	zero( &EGWndRes::FontSizes );
	zero( &EGWndRes::Colors );
	zero( &EGWndRes::Brushes );

	EGWndRes::CreateResources();

	EGWnd_EditorSerializeClipboardFormat = RegisterClipboardFormatW( L"EGEditorSerialize" );
	EGWnd_EgsmNodeClipboardFormat = RegisterClipboardFormatW( L"EGEditorEgsmNode" );
}

void EGWnd_Deinit( HINSTANCE hInst )
{
	assert( hInst == g_hInst );

	auto DoDelete = []( auto& Obj ) -> void
	{
		DeleteObject( Obj );
		Obj = nullptr;
	};

	EGWndRes::DestroyResources();

	EGWndPropControl::DeinitClass( hInst );
	EGWndChildBase::DeinitClass( hInst );

	if( g_RichEditLib )
	{
		FreeLibrary( g_RichEditLib );
	}

	g_hInst = nullptr;
}

void EGWnd_GetAppParms( const EGArray<eg_cpstr16>& ArgsArray, EGWndAppParms& ParmsOut )
{
	for( const eg_string_big Str : ArgsArray )
	{
		if( Str.Len() > 0 && Str.CharAt(0) == '-' )
		{
			ParmsOut.AddParm( Str , "" );
		}
		else
		{
			ParmsOut.AppendLastParm( Str );
		}
	}
}

void EGWnd_GetAppParmsFromCommandLine( EGWndAppParms& ParmsOut )
{
	int nArgs = 0;
	LPWSTR* szArglist = CommandLineToArgvW( GetCommandLineW(), &nArgs );
	if( szArglist )
	{
		EGArray<eg_cpstr16> ArgsArray;
		for( int i = 0; i < nArgs; i++ )
		{
			ArgsArray.Append( szArglist[i] );
		}

		EGWnd_GetAppParms( ArgsArray , ParmsOut );

		LocalFree( szArglist );
	}
}

void EGWnd_RunApp( const egWndAppInfo& AppInfo , const EGWndCreateWndFn& CreateWndFn , const EGWndDestroyWndFn& DestroyWndFn )
{
	EGWnd_Init( AppInfo.hInst );
	EGWndAppWindow::InitClass( AppInfo.hInst , AppInfo.hResModule , AppInfo.Icon, AppInfo.IconSm );

	{
		int nArgs;
		LPWSTR* szArglist = CommandLineToArgvW( GetCommandLineW(), &nArgs );
		EGArray<eg_cpstr16> ArgsArray;
		for( int i = 0; i < nArgs; i++ )
		{
			ArgsArray.Append( szArglist[i] );
		}

		EGWndAppWindow* AppWnd = CreateWndFn( AppInfo.nCmdShow );

		if( AppWnd )
		{
			AppWnd->OnAppStart( ArgsArray );
		}

		if( szArglist )
		{
			LocalFree( szArglist );
		}

		HACCEL hAccel = AppInfo.Accel ? LoadAcceleratorsW( AppInfo.hResModule , AppInfo.Accel ) : nullptr;

		eg_uint64 LastUpdate = Timer_GetRawTime();

		while( AppWnd && !AppWnd->IsDone() )
		{
			MSG msg;
			zero( &msg );
			while( PeekMessageW( &msg , NULL , 0 , 0 , PM_REMOVE ) )
			{
				if( hAccel )
				{
					TranslateAcceleratorW( msg.hwnd, hAccel, &msg );
				}
				TranslateMessage( &msg );
				DispatchMessageW( &msg );
			}

			if( !AppInfo.bUpdateEveryFrame && !AppWnd->IsDone() )
			{
				WaitMessage();
			}

			eg_uint64 NowTime = Timer_GetRawTime();
			eg_real DeltaTime = Timer_GetRawTimeElapsedSec( LastUpdate, NowTime );

			if( DeltaTime >= 1.f / 30.f )
			{
				LastUpdate = NowTime;
				AppWnd->OnAppUpdate( DeltaTime );
			}
			else
			{
				Sleep( 0 );
			}

			// OutputDebugStringA( EGString_Format( "Updated: %g\r\n" , DeltaTime ) );
		}

		DestroyWndFn( AppWnd );
		AppWnd = nullptr;
	}

	EGWndAppWindow::DeinitClass( AppInfo.hInst );
	EGWnd_Deinit( AppInfo.hInst );
}

void EGWnd_ClearFocus()
{
	SetFocus( GetForegroundWindow() );
}

HINSTANCE EGWnd_GetInst()
{
	return g_hInst;
}

UINT EGWnd_GetEditorSerializationClipboardFormat()
{
	return EGWnd_EditorSerializeClipboardFormat;
}

UINT EGWnd_GetEgsmNodeClipboardFormat()
{
	return EGWnd_EgsmNodeClipboardFormat;
}

HFONT EGWnd_GetFont( egw_font_t Type )
{
	return EGWnd_GetDcFont( Type ).GetWindowsFont();
}

EGWndDcFont& EGWnd_GetDcFont( egw_font_t Type )
{
	if( 0 <= enum_index(Type) && enum_index(Type) < countof(EGWndRes::Fonts) )
	{
		return EGWndRes::Fonts[enum_index(Type)];
	}

	return EGWndRes::Fonts[enum_index(egw_color_t::DEFAULT)];
}

eg_int EGWnd_GetFontSize( egw_font_t Type )
{
	if( 0 <= enum_index(Type) && enum_index(Type) < countof(EGWndRes::FontSizes) )
	{
		return EGWndRes::FontSizes[enum_index(Type)];
	}

	return EGWndRes::FontSizes[enum_index(egw_color_t::DEFAULT)];
}

HBRUSH EGWnd_GetBrush( egw_color_t Type )
{
	return EGWnd_GetDcBrush( Type ).GetWindowsBrush();
}

EGWndDcBrush& EGWnd_GetDcBrush( egw_color_t Type )
{
	if( 0 <= enum_index(Type) && enum_index(Type) < countof(EGWndRes::Brushes) )
	{
		return EGWndRes::Brushes[enum_index(Type)];
	}

	return EGWndRes::Brushes[enum_index(egw_color_t::DEFAULT)];
}

COLORREF EGWnd_GetColor( egw_color_t Type )
{
	const eg_color32 Out = EGWnd_GetDcColor( Type );
	return RGB(Out.R,Out.G,Out.B);
}

eg_color32 EGWnd_GetDcColor( egw_color_t Type )
{
	if( 0 <= enum_index( Type ) && enum_index( Type ) < countof( EGWndRes::Colors ) )
	{
		return EGWndRes::Colors[enum_index( Type )];
	}

	return EGWndRes::Colors[enum_index( egw_color_t::DEFAULT )];
}

static void EGWnd_CreateMenuRecursive( HMENU Parent , const EGWndMenuItemDef& ParentDef , eg_int Depth )
{
	assert( Parent );

	for( const EGWndMenuItemDef& ChildDef : ParentDef.Children )
	{
		MENUITEMINFOW Mi;
		zero( &Mi );
		Mi.cbSize = sizeof(Mi);

		eg_char16 NameBuffer[eg_string_small::STR_SIZE];

		if( ChildDef.Type == egw_menu_item_t::Button )
		{
			if( ChildDef.Id != 0 )
			{
				Mi.fMask |= MIIM_ID;
				Mi.wID = ChildDef.Id;
			}

			if( ChildDef.Label && EGString_StrLen(ChildDef.Label) > 0 )
			{
				EGString_Copy( NameBuffer , ChildDef.Label , countof(NameBuffer) );
				Mi.fMask |= MIIM_STRING;
				Mi.dwTypeData = NameBuffer;
			}

			if( ChildDef.Children.Len() > 0 )
			{
				Mi.hSubMenu = CreatePopupMenu();
				Mi.fMask |= MIIM_SUBMENU;

				EGWnd_CreateMenuRecursive( Mi.hSubMenu , ChildDef , Depth+1 );
			}
		}
		else if( ChildDef.Type == egw_menu_item_t::Separator )
		{
			Mi.fMask = MIIM_FTYPE;
			Mi.fType = MFT_SEPARATOR;
		}

		if( Parent )
		{
			InsertMenuItemW( Parent , GetMenuItemCount(Parent) , true , &Mi );
		}
	}
}

HMENU EGWnd_CreateMenu( const EGWndMenuItemDef& MenuDef, eg_bool bIsPopupMenu )
{
	HMENU Out = nullptr;
	if( bIsPopupMenu )
	{
		Out = CreatePopupMenu();
	}
	else
	{
		Out = CreateMenu();
	}
	EGWnd_CreateMenuRecursive( Out, MenuDef , 0 );
	return Out;
}

void EGWnd_RegisterTabTo( HWND Wnd )
{
	if( EGWnd_TabToTable.Contains( Wnd ) || Wnd == nullptr )
	{
		assert( false ); // Don't register more than once!
		return;
	}

	EGWnd_TabToTable.Append( Wnd );
}

void EGWnd_UnregisterTabTo( HWND Wnd )
{
	if( EGWnd_TabToTable.Contains( Wnd ) )
	{
		EGWnd_TabToTable.DeleteByItem( Wnd );
	}
	else
	{
		assert( false ); // Never registered.
	}
}

void EGWnd_TabTo( HWND WndCurrent , eg_int Direction )
{
	eg_size_t Index = EGWnd_TabToTable.GetIndexOf( WndCurrent );
	if( EGARRAY_INVALID_INDEX == Index )
	{
		Index = 0;
	}

	Index += Direction; // We intentionally want to be able to go out of range, that way tabbing off the last control focuses the main window.

	if( EGWnd_TabToTable.IsValidIndex( Index ) )
	{
		SetFocus( EGWnd_TabToTable[Index] );
		PostMessageW( EGWnd_TabToTable[Index] , WM_EG_FOCUS_FROM_TAB , 0 , reinterpret_cast<LPARAM>(WndCurrent) );
	}
	else
	{
		EGWnd_ClearFocus();
	}
}

eg_int EGWnd_GetDPI( HDC hdc )
{
	return GetDeviceCaps( hdc , LOGPIXELSX );
}

eg_real EGWnd_GetDPIScaling( HDC hdc )
{
	return EG_To<eg_real>(EGWnd_GetDPI( hdc ) ) / USER_DEFAULT_SCREEN_DPI;
}

eg_real EGWnd_GetAppScaling()
{
	return EGWND_APP_SCALING;
}

eg_int EGWnd_GetAppScaledSize( eg_int BaseSize )
{
	return EG_To<eg_int>( BaseSize * EGWND_APP_SCALING );
}

eg_ivec2 EGWnd_GetDesktopResolution()
{
	eg_ivec2 Out(0,0);

	RECT rc;
	zero( &rc );
	if( GetWindowRect( GetDesktopWindow() , &rc ) )
	{
		Out.x = static_cast<eg_int>((rc.right - rc.left)/EGWnd_GetAppScaling());
		Out.y = static_cast<eg_int>((rc.bottom - rc.top)/EGWnd_GetAppScaling());
	}
	return Out;
}
