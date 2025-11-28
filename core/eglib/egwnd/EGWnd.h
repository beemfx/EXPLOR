// (c) 2017 Beem Media

#pragma once

#include "EGWndTypes.h"
#include "EGWindowsAPI.h"
#include "EGCppLibAPI.h"

static const UINT WM_EG_COMMIT               = WM_USER+11;
static const UINT WM_EG_REFRESH_OFFSETS      = WM_USER+12;
static const UINT WM_EG_COMMIT_ON_FOCUS_LOST = WM_USER+13;
static const UINT WM_EG_CONTROL_CHANGED      = WM_USER+14;
static const UINT WM_EG_FOCUS_FROM_TAB       = WM_USER+15;
static const UINT WM_EG_PROP_CONTROL_REFRESH = WM_USER+16;
static const UINT WM_EG_USER                 = WM_USER+50;

class EGWndAppWindow;
class EGWndDc;
class EGWndDcFont;
class EGWndDcBrush;

typedef std::function<EGWndAppWindow*(int nCmdShow)> EGWndCreateWndFn;
typedef std::function<void(EGWndAppWindow*)> EGWndDestroyWndFn;

struct egWndAppInfo
{
	const HINSTANCE  hInst;
	const HMODULE    hResModule;
	const int        nCmdShow;
	const eg_bool    bUpdateEveryFrame;
	const eg_cpstr16 Icon;
	const eg_cpstr16 IconSm; 
	const eg_cpstr16 Accel;

	egWndAppInfo( HINSTANCE hInstIn , HMODULE hResModuleIn , int nCmdShowIn , eg_bool bUpdateEveryFrameIn , eg_cpstr16 IconIn , eg_cpstr16 IconSmIn , eg_cpstr16 AccelIn )
	: hInst( hInstIn )
	, hResModule( hResModuleIn )
	, nCmdShow( nCmdShowIn )
	, bUpdateEveryFrame( bUpdateEveryFrameIn )
	, Icon( IconIn )
	, IconSm( IconSmIn )
	, Accel( AccelIn )
	{

	}
};

class EGWndAppParms
{
private:

	struct egParmInfo
	{
		eg_string_small Type;
		eg_string_big   Value;
	};

private:

	EGArray<egParmInfo> m_Parms;

public:

	void AddParm( eg_cpstr Type , eg_cpstr Value )
	{
		egParmInfo NewParm;
		NewParm.Type = Type;
		NewParm.Value = Value;
		m_Parms.Append( NewParm );
	}

	void AppendLastParm( eg_cpstr Value )
	{
		if( m_Parms.Len() == 0 )
		{
			AddParm( "" , Value );
		}
		else
		{
			if( m_Parms[m_Parms.Len()-1].Value.Len() > 0 )
			{
				m_Parms[m_Parms.Len()-1].Value.Append( ' ' );
			}

			m_Parms[m_Parms.Len()-1].Value.Append( Value );
		}
	}

	eg_bool ContainsType( eg_cpstr Type ) const
	{
		for( const egParmInfo& Info : m_Parms )
		{
			if( Info.Type.EqualsI( Type ) )
			{
				return true;
			}
		}
		return false;
	}

	eg_string_big GetParmValue( eg_cpstr Type ) const
	{
		for( const egParmInfo& Info : m_Parms )
		{
			if( Info.Type.EqualsI( Type ) )
			{
				return Info.Value;
			}
		}
		return "";
	}
};

void EGWnd_Init( HINSTANCE hInst );
void EGWnd_Deinit( HINSTANCE hInst );
void EGWnd_GetAppParms( const EGArray<eg_cpstr16>& ArgsArray , EGWndAppParms& ParmsOut );
void EGWnd_GetAppParmsFromCommandLine( EGWndAppParms& ParmsOut );
void EGWnd_RunApp( const egWndAppInfo& AppInfo , const EGWndCreateWndFn& CreateWndFn , const EGWndDestroyWndFn& DestroyWndFn );
void EGWnd_ClearFocus();
HINSTANCE EGWnd_GetInst();
UINT EGWnd_GetEditorSerializationClipboardFormat();
UINT EGWnd_GetEgsmNodeClipboardFormat();

void EGWnd_RegisterTabTo( HWND Wnd );
void EGWnd_UnregisterTabTo( HWND Wnd );
void EGWnd_TabTo( HWND WndCurrent , eg_int Direction );
eg_int EGWnd_GetDPI( HDC hdc );
eg_real EGWnd_GetDPIScaling( HDC hdc );
eg_real EGWnd_GetAppScaling();
eg_int EGWnd_GetAppScaledSize( eg_int BaseSize );
eg_ivec2 EGWnd_GetDesktopResolution();

HFONT EGWnd_GetFont( egw_font_t Type );
EGWndDcFont& EGWnd_GetDcFont( egw_font_t Type );
eg_int EGWnd_GetFontSize( egw_font_t Type );
HBRUSH EGWnd_GetBrush( egw_color_t Type );
EGWndDcBrush& EGWnd_GetDcBrush( egw_color_t Type );
COLORREF EGWnd_GetColor( egw_color_t Type );
eg_color32 EGWnd_GetDcColor( egw_color_t Type );


//
// Helper Class for mouse capture
//
class EGWndMouseCapture
{
private:
	HWND    m_Owner;
	HCURSOR m_RestoreCursor;
	POINT   m_BeginCaptureMousePos;
	eg_bool m_bIsCapturing:1;
public:
	EGWndMouseCapture()
	: m_Owner(nullptr)
	, m_RestoreCursor(nullptr)
	, m_bIsCapturing(false)
	{
		zero( &m_BeginCaptureMousePos );
	}

	~EGWndMouseCapture(){ }

	void OnBeginCapture( HWND Owner , eg_int MouseX , eg_int MouseY, HCURSOR CaptureCursor = nullptr )
	{
		assert( !m_bIsCapturing && nullptr == m_Owner ); // Bad flow?
		if( !m_bIsCapturing )
		{
			m_Owner = Owner;
			HWND OldCapture = SetCapture( m_Owner );
			m_bIsCapturing = true;
			m_BeginCaptureMousePos.x = MouseX;
			m_BeginCaptureMousePos.y = MouseY;
			if( CaptureCursor )
			{
				m_RestoreCursor = SetCursor( CaptureCursor );
			}
			assert( nullptr == OldCapture ); // Should only be capturing one thing at a time.
		}
	}

	void OnEndCapture()
	{
		if( m_bIsCapturing )
		{
			ReleaseCapture();
			m_bIsCapturing = false;
			m_BeginCaptureMousePos.x = 0;
			m_BeginCaptureMousePos.y = 0;
			m_Owner = nullptr;
			if( m_RestoreCursor )
			{
				SetCursor( m_RestoreCursor );
				m_RestoreCursor = nullptr;
			}
		}
	}

	void OnCancelMode()
	{
		if( m_bIsCapturing )
		{
			OnEndCapture();
		}
	}

	eg_bool IsCapturing() const
	{
		return m_bIsCapturing;
	}

	POINT GetBeginCaptureMousePos() const
	{
		return m_BeginCaptureMousePos;
	}
};

enum class egw_menu_item_t
{
	Button,
	Separator,
};

struct EGWndMenuItemDef
{
	egw_menu_item_t           Type = egw_menu_item_t::Button;
	eg_string_small           Label = L"";
	eg_uint                   Id = 0;
	EGArray<EGWndMenuItemDef> Children;

	void AddCmd( eg_cpstr Text , eg_uint CmdId )
	{
		Children.ExtendSize( 1 );
		Children.Last().Label = Text;
		Children.Last().Id = CmdId;
	}

	void AddSeparator()
	{
		Children.ExtendSize( 1 );
		Children.Last().Type = egw_menu_item_t::Separator;
	}
};

HMENU EGWnd_CreateMenu( const EGWndMenuItemDef& MenuDef, eg_bool bIsPopupMenu );
