// WinCon - A console interface for the dedicated server.
// (c) 2015 Beem Software
#include "EGEgWinCon.h"
#include "EGEngineConfig.h"
#include "EGEngine.h"
#include "EGThread.h"
#include "EGDebugText.h"
#include "EGWindowsAPI.h"
#include "EGWnd.h"

static eg_cpstr16 WinCon_WindowClass      = L"ConWindowClass";
static eg_cpstr16 WinCon_TextDisplayClass = L"ConTextDisplayClass";
static eg_cpstr16 WinCon_EntryClass       = L"ConEntryClass";

static class EGConsoleThread: private IThreadProc
{
private:
	static const eg_uint CD_LINES    = 30;
	static const eg_uint CD_FONTSIZE = 15;


	static const eg_uint ID_QUITBUTTON=103;
	static const eg_uint ID_TEXTEDIT=102;

	static const UINT WM_REFRESH_CONSOLE    = WM_USER+20;
private:
	HWND     m_ConWnd;
	HFONT    m_Font;
	eg_bool  m_BlinkOn;
	EGThread m_Thread;
	eg_bool  m_IsRunning:1;
	eg_bool  m_IsWithGame:1;
	eg_real  m_Scaling = 1.f;
public:
	EGConsoleThread()
	: m_Thread( EGThread::egInitParms( "ConsoleThread" , EGThread::eg_init_t::LowPriority ) )
	, m_ConWnd( nullptr )
	, m_Font( nullptr )
	, m_IsRunning( false )
	, m_IsWithGame( false )
	, m_BlinkOn( false )
	{ 

	}

	~EGConsoleThread()
	{

	}

	static LRESULT CALLBACK WinCon_EntryProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		#define BLINKER_TIMER (0x12001200)

		switch(uMsg)
		{
		case WM_NCCREATE:
		{
			EGConsoleThread* _this = (EGConsoleThread*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			SetWindowLongPtr( hwnd , GWLP_USERDATA , reinterpret_cast<LONG_PTR>(_this) );
			return DefWindowProcW( hwnd , uMsg , wParam , lParam );
		} break;
		case WM_CHAR:
			SendMessage( GetParent(hwnd) , uMsg , wParam , lParam );
			break;
		case WM_DESTROY:
			KillTimer(hwnd, BLINKER_TIMER);
			break;
		case WM_TIMER:
		{
			EGConsoleThread* _this = reinterpret_cast<EGConsoleThread*>(GetWindowLongPtr( hwnd , GWLP_USERDATA ));
			if(wParam==BLINKER_TIMER)
			{
				_this->m_BlinkOn=!_this->m_BlinkOn;
				InvalidateRect(hwnd, NULL, TRUE);
			}
		} break;
		case WM_CREATE:
			/* Get the font that was passed over here. */
			SetTimer(hwnd, BLINKER_TIMER, GetCaretBlinkTime(), NULL);
			break;
		case WM_REFRESH_CONSOLE:
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case WM_MOUSEMOVE:
		{
			SetFocus( hwnd );
		} break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			SetFocus(hwnd);
			break;
		case WM_PAINT:
		{
			EGConsoleThread* _this = reinterpret_cast<EGConsoleThread*>(GetWindowLongPtr( hwnd , GWLP_USERDATA ));
			PAINTSTRUCT ps;	
			HDC hdc=BeginPaint(hwnd, &ps);
			HFONT hOldFont=(HFONT)SelectObject(hdc, _this->m_Font);

			MainConsole.Lock();
			eg_string Line = MainConsole.GetCmdLine();
			MainConsole.Unlock();
			if( _this->m_BlinkOn )
			{
				Line.Append( "_" );
			}
			
			TextOutA(hdc, 0, 0, Line, Line.Len());

			SelectObject(hdc, hOldFont);
			EndPaint(hwnd, &ps);
		} break;
		default:
			return DefWindowProcW(hwnd, uMsg, wParam, lParam);
		}
		return 0l;
	}

	static LRESULT CALLBACK WinCon_ConTextDisplayProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{	
		switch(uMsg)
		{
		case WM_NCCREATE:
		{
			EGConsoleThread* _this = (EGConsoleThread*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			SetWindowLongPtr( hwnd , GWLP_USERDATA , reinterpret_cast<LONG_PTR>(_this) );
			return DefWindowProcW( hwnd , uMsg , wParam , lParam );
		} break;
		case WM_CREATE:
		{
			MainConsole.Lock();
			unsigned long nEntries=MainConsole.GetNumLines();
			MainConsole.Unlock();
			PostMessage( hwnd , WM_REFRESH_CONSOLE , 0 , 0 );
			break;
		}
		case WM_CHAR:
			SendMessage( GetParent(hwnd) , uMsg , wParam , lParam );
			break;
		case WM_MOUSEMOVE:
		{
			SetFocus( hwnd );
		} break;
		case WM_MOUSEWHEEL:
		{
			SHORT Scroll = HIWORD( wParam );
			Scroll /= WHEEL_DELTA;
			for( eg_uint i=0; i<3; i++ )
			{
				SendMessage( hwnd , WM_VSCROLL , MAKEWORD( (Scroll > 0 ? SB_LINEUP:SB_LINEDOWN),0) , 0 );
			}
		} break;
		case WM_REFRESH_CONSOLE:
		{
			SCROLLINFO si;
			zero( &si );
			si.cbSize=sizeof(SCROLLINFO);
			si.fMask=SIF_POS|SIF_RANGE;
			GetScrollInfo(hwnd, SB_VERT, &si);
			int DistFromBottom = si.nMax - si.nPos;
			if( si.nPos < CD_LINES )
			{
				DistFromBottom = 0;
			}
			si.fMask=SIF_RANGE|SIF_POS|SIF_PAGE;
			si.nMin=0;
			MainConsole.Lock();
			si.nMax=EG_Max<int>( MainConsole.GetNumLines()-1 , 0 );
			MainConsole.Unlock();
			si.nPage=CD_LINES;
			si.nPos=EG_Max( 0 , si.nMax-DistFromBottom );
			SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
			InvalidateRect(hwnd, NULL, TRUE);
		} break;
		case WM_VSCROLL:
		{
			SCROLLINFO si;
			zero( &si );
			si.cbSize=sizeof(SCROLLINFO);
			si.fMask=SIF_ALL;
			GetScrollInfo(hwnd, SB_VERT, &si);

			switch(LOWORD(wParam))
			{
			case SB_PAGEDOWN:
				si.nPos+=si.nPage/2;
				break;
			case SB_LINEDOWN:
				si.nPos++;
				break;
			case SB_PAGEUP:
				si.nPos-=si.nPage/2;
				break;
			case SB_LINEUP:
				si.nPos--;
				break;
			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				si.nPos=si.nTrackPos;
				break;
			case SB_TOP:
				si.nPos=si.nMin;
				break;
			case SB_BOTTOM:
				si.nPos=si.nMax;
				break;
			}
			SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
			RedrawWindow(hwnd, NULL, NULL, RDW_ERASE|RDW_INVALIDATE);	
		} break;
		case WM_PAINT:
		{
			EGConsoleThread* _this = reinterpret_cast<EGConsoleThread*>(GetWindowLongPtr( hwnd , GWLP_USERDATA ));
			PAINTSTRUCT ps;
			RECT wrc;
			TEXTMETRIC tm;

			GetClientRect(hwnd, &wrc);
			
			eg_uint nNumEntries=0;//MainConsole.GetNumLines();
		
			HDC hdc=BeginPaint(hwnd, &ps);
			HFONT hOldFont=(HFONT)SelectObject(hdc, _this->m_Font);
			GetTextMetrics(hdc, &tm);
			SetBkColor( hdc , RGB(0,0,0) );

			/* Update the scroll info. */
			SCROLLINFO si;
			zero( &si );
			si.cbSize=sizeof(SCROLLINFO);
			si.fMask=SIF_POS|SIF_PAGE|SIF_RANGE;
			GetScrollInfo(hwnd, SB_VERT, &si);

			MainConsole.Lock();
			for( eg_uint i=0; i<CD_LINES; i++ )
			{
				eg_string Line = MainConsole.GetLineAt(i+si.nPos);
				eg_color32 Color = DebugText_GetStringColorAndRemoveColorTag( &Line );
				SetTextColor( hdc, RGB( Color.R , Color.G , Color.B ) );
				TextOutA(hdc, 2, tm.tmHeight*i, Line, Line.Len());
			}
			MainConsole.Unlock();

			SelectObject(hdc, hOldFont);
			EndPaint(hwnd, &ps);
			break;
		}
		case WM_DESTROY:
			break;
		default:
			return DefWindowProcW(hwnd, uMsg, wParam, lParam);
		}
		return 0l;
	}

	static BOOL CALLBACK WinCon_UserUpdateEnumChildProc( HWND hwnd , LPARAM lParam )
	{
		unused( lParam );
		PostMessage( hwnd , WM_REFRESH_CONSOLE , 0 , 0 );
		return TRUE;
	}

	static LRESULT CALLBACK WinCon_ConWindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
	{
		switch(uMsg)
		{
		case WM_NCCREATE:
		{
			EGConsoleThread* _this = (EGConsoleThread*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			SetWindowLongPtr( hwnd , GWLP_USERDATA , reinterpret_cast<LONG_PTR>(_this) );
			return DefWindowProcW( hwnd , uMsg , wParam , lParam );
		} break;
		case WM_CREATE:
		{
			EGConsoleThread* _this = reinterpret_cast<EGConsoleThread*>(GetWindowLongPtr( hwnd , GWLP_USERDATA ));
			HINSTANCE hInst=NULL;//GetModuleHandle(NULL);
			RECT rcMainWindow;

			GetClientRect(hwnd, &rcMainWindow);

			/* Select the font we want. */
			_this->m_Font=CreateFontW(
				_this->GetFontSize(),
				0,
				0,
				0,
				FW_DEMIBOLD,
				FALSE,
				FALSE,
				FALSE,
				ANSI_CHARSET,
				0,
				0,
				DEFAULT_QUALITY,
				DEFAULT_PITCH,
				L"Courier New");

			HWND hwndConsole=CreateWindowW(
				WinCon_TextDisplayClass,
				WinCon_TextDisplayClass,
				WS_CHILD|WS_VSCROLL|WS_VISIBLE,
				5,
				5,
				rcMainWindow.right-10,
				CD_LINES*_this->GetFontSize(),
				hwnd,
				NULL,
				hInst,
				_this);

			ShowWindow(hwnd, SW_SHOWNORMAL);
			UpdateWindow(hwnd);

			HWND hwndEntry=CreateWindowExW(
				WS_EX_CLIENTEDGE,
				WinCon_EntryClass,
				WinCon_EntryClass,
				WS_CHILD|WS_VISIBLE,
				EG_To<eg_int>(5*_this->m_Scaling),
				CD_LINES*_this->GetFontSize()+10,
				rcMainWindow.right-10,
				EG_To<eg_int>(25*_this->m_Scaling),
				hwnd,
				reinterpret_cast<HMENU>(static_cast<eg_uintptr_t>(ID_TEXTEDIT)),
				hInst,
				_this);

			UpdateWindow(hwndEntry);

			HWND hwndQuitButton=CreateWindowW(
				L"BUTTON",
				L"Quit",
				WS_CHILD|WS_VISIBLE,
				(rcMainWindow.right-75)/2,
				rcMainWindow.bottom-25-10,
				75,
				25,
				hwnd,
				reinterpret_cast<HMENU>(static_cast<eg_uintptr_t>(ID_QUITBUTTON)),
				hInst,
				0);

			SetFocus(hwndEntry);

			//return DefWindowProcW(hwnd, uMsg, wParam, lParam);
		} break;
		case WM_MOUSEMOVE:
		{
			SetFocus( hwnd );
		} break;
		case WM_REFRESH_CONSOLE:
			EnumChildWindows( hwnd , WinCon_UserUpdateEnumChildProc , 0 );
			break;
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case ID_QUITBUTTON:
				Engine_QueMsg( "engine.quit()" );
				break;
			default:
				break;
			}	
		} break;
		case WM_CHAR:
			MainConsole.OnChar( (eg_char)wParam );
			SendMessage(hwnd, WM_REFRESH_CONSOLE, 0, 0);
			break;
		case WM_CLOSE:
			Engine_QueMsg( "engine.quit()" );
			break;
		case WM_DESTROY:
		{
			EGConsoleThread* _this = reinterpret_cast<EGConsoleThread*>(GetWindowLongPtr( hwnd , GWLP_USERDATA ));
			DeleteObject(_this->m_Font);
		} break;
		default:
			return DefWindowProcW(hwnd, uMsg, wParam, lParam);
		}
		return 0l;
	}

private:

	eg_int GetFontSize() const { return EG_To<eg_int>(CD_FONTSIZE * m_Scaling); }

	virtual void OnStart() override
	{
		WNDCLASSW wc;
		zero( &wc );

		wc.cbClsExtra=0;
		wc.cbWndExtra=0;
		wc.hbrBackground=(HBRUSH)COLOR_WINDOW;
		wc.hCursor=LoadCursorW(NULL, IDC_ARROW);
		wc.hIcon=LoadIconW( GetModuleHandleW(NULL) , L"EGICON" );
		wc.hInstance=NULL;
		wc.lpfnWndProc=WinCon_ConWindowProc;
		wc.lpszClassName=WinCon_WindowClass;
		wc.lpszMenuName=NULL;
		wc.style=0;
		RegisterClassW(&wc);

		wc.cbClsExtra=0;
		wc.cbWndExtra=0;
		wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.hCursor=LoadCursorW(NULL, IDC_ARROW);
		wc.hIcon=NULL;
		wc.hInstance=NULL;
		wc.lpszMenuName=NULL;
		wc.style=CS_VREDRAW|CS_HREDRAW;
		wc.lpfnWndProc=WinCon_ConTextDisplayProc;
		wc.lpszClassName=WinCon_TextDisplayClass;
		RegisterClassW(&wc);

		wc.cbClsExtra=0;
		wc.cbWndExtra=0;
		wc.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.hCursor=LoadCursorW(NULL, IDC_ARROW);
		wc.hIcon=NULL;
		wc.hInstance=NULL;
		wc.lpfnWndProc=WinCon_EntryProc;
		wc.lpszClassName=WinCon_EntryClass;
		wc.lpszMenuName=NULL;
		wc.style=CS_VREDRAW|CS_HREDRAW;
		RegisterClassW(&wc);

		RECT RcDesktop;
		zero( &RcDesktop );
		//GetClientRect( GetDesktopWindow() , &RcDesktop );
		SystemParametersInfoW( SPI_GETWORKAREA , 0 , &RcDesktop , 0 );

		HDC ScreenDC = GetDC( NULL );
		m_Scaling = EG_Max( 1.f , EGWnd_GetDPIScaling( ScreenDC ) );
		ReleaseDC( NULL , ScreenDC );
		ScreenDC = nullptr;

		const eg_int WIDTH  = EG_To<eg_int>(800 * m_Scaling);
		const eg_int HEIGHT = EG_To<eg_int>(600 * m_Scaling);

		int X = ((RcDesktop.right-RcDesktop.left)-WIDTH)/2;
		int Y = ((RcDesktop.bottom-RcDesktop.top)-HEIGHT)/2;

		// If this is not a dedicated server we want the console below the game
		// so we can see output.
		if( m_IsWithGame )
		{
			Y = RcDesktop.bottom-HEIGHT;
		}

		m_ConWnd = CreateWindowW(
			WinCon_WindowClass,
			EGString_ToWide(Engine_GetName()),
			WS_SYSMENU|WS_MINIMIZEBOX,
			X,
			Y,
			WIDTH,
			HEIGHT,
			NULL,
			NULL,
			NULL,
			this);
	}

	virtual void Update( eg_real DeltaTime ) override
	{
		unused( DeltaTime );
		EGThread_Sleep(0);
		MSG msg;
		while(PeekMessageW(&msg, m_ConWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	virtual void OnStop() override
	{
		DestroyWindow( m_ConWnd );
		m_ConWnd = nullptr;

		UnregisterClassW( WinCon_EntryClass , NULL );
		UnregisterClassW( WinCon_TextDisplayClass , NULL );
		UnregisterClassW( WinCon_WindowClass , NULL );
	}


	virtual void OnThreadMsg(eg_cpstr strParm) override
	{
		if( eg_string( strParm ) == "Refresh()" )
		{
			PostMessage( m_ConWnd , WM_REFRESH_CONSOLE , 0 , 0 );
		}
	}
public:
	void CreateCon( eg_bool IsWithGame )
	{
		m_IsRunning = true;
		m_IsWithGame = IsWithGame;
		m_Thread.RegisterProc( this );
		m_Thread.Start();
	}

	void DestroyCon()
	{
		m_Thread.Stop();
		m_Thread.UnregisterProc( this );
		m_IsRunning = false;
	}

	void RefreshCon()
	{
		if( m_IsRunning )
		{
			m_Thread.PostThreadMsg( "Refresh()" );
		}
	}

} ConsoleThread;


void WinCon_RefreshCon()
{
	ConsoleThread.RefreshCon();
}

void WinCon_CreateCon( eg_bool IsWithGame )
{
	ConsoleThread.CreateCon( IsWithGame );
}

void WinCon_DestroyCon()
{
	ConsoleThread.DestroyCon();
}