// (c) 2017 Beem Media

#include "EGWndBase.h"
#include "EGWndDraw.h"

EGWndBase::EGWndBase( HWND hwndParent, eg_cpstr16 szWindowClass, eg_cpstr16 szTitle, DWORD Style ) 
: m_hwnd( NULL )
, m_bDoubleBufferDraw( DOUBLE_BUFFER_BY_DEFAULT )
, m_bRegisteredTabTo( false )
{
	HWND hwnd = CreateWindowW(
		szWindowClass, szTitle,
		Style,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		hwndParent, nullptr,
		nullptr, this );

	if( NULL == hwnd )
	{
		DWORD Error = GetLastError();
		return;
	}

	// We set the hwnd variable after creation because the subclass can't sue it during creation.
	m_hwnd = hwnd;
}

EGWndBase::EGWndBase( eg_cpstr16 szWindowClass , eg_cpstr16 szTitle , DWORD Style ) 
: m_hwnd( NULL )
, m_bDoubleBufferDraw( DOUBLE_BUFFER_BY_DEFAULT )
, m_bRegisteredTabTo( false )
{
	HWND hwnd = CreateWindowExW(
		0,
		szWindowClass, szTitle,
		Style,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr, nullptr,
		nullptr, this );

	if( NULL == hwnd )
	{
		return;
	}

	// We set the hwnd variable after creation because the subclass can't sue it during creation.
	m_hwnd = hwnd;
}

EGWndBase::~EGWndBase()
{
	HWND hwnd = m_hwnd;
	m_hwnd = NULL;
	DestroyWindow( hwnd );
}

eg_recti EGWndBase::GetViewArea() const
{
	eg_recti Out(0,0,0,0);
	RECT WindowRect; 
	if( GetClientRect( m_hwnd , &WindowRect ) )
	{
		Out.left = WindowRect.left;
		Out.top = WindowRect.top;
		Out.right = WindowRect.right;
		Out.bottom = WindowRect.bottom;
	}
	return Out;
}

eg_recti EGWndBase::GetWndArea() const
{
	eg_recti Out( 0, 0, 0, 0 );
	RECT WindowRect;
	if( GetWindowRect( m_hwnd , &WindowRect ) )
	{
		Out.left = WindowRect.left;
		Out.top = WindowRect.top;
		Out.right = WindowRect.right;
		Out.bottom = WindowRect.bottom;
	}
	return Out;
}

void EGWndBase::RegisterForTabTo( eg_bool bRegister )
{
	if( m_bRegisteredTabTo != bRegister )
	{
		m_bRegisteredTabTo = bRegister;
		if( m_bRegisteredTabTo )
		{
			EGWnd_RegisterTabTo( GetWnd() );
		}
		else
		{
			EGWnd_UnregisterTabTo( GetWnd() );
		}
	}
}


LRESULT CALLBACK EGWndBase::WndProcShell( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	if( WM_NCCREATE == message )
	{
		EGWndBase* _this = (EGWndBase*)( (LPCREATESTRUCT)lParam )->lpCreateParams;
		SetWindowLongPtrW( hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( _this ) );
		return DefWindowProcW( hwnd, message, wParam, lParam );
	}

	EGWndBase* _this = reinterpret_cast<EGWndBase*>( GetWindowLongPtr( hwnd, GWLP_USERDATA ) );
	if( _this && _this->m_hwnd )
	{
		assert( hwnd == _this->m_hwnd );
		return _this->WndProc( message, wParam, lParam );
	}

	return DefWindowProcW( hwnd, message, wParam, lParam );
}

HFONT EGWndBase::CreateBasicFont( eg_cpstr Face, eg_int Size, eg_bool bIsBold, eg_bool bIsItalic )
{
	LOGFONTW lf;
	zero( &lf );
	lf.lfHeight = Size;
	lf.lfWeight = bIsBold ? FW_BOLD : FW_NORMAL;
	lf.lfItalic = bIsItalic ? TRUE : FALSE;
	lf.lfCharSet = ANSI_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_MODERN;
	eg_string_big( Face ).CopyTo( lf.lfFaceName, countof( lf.lfFaceName ) );
	return CreateFontIndirectW( &lf );
}

HPEN EGWndBase::CreateBasicPen( int Width, COLORREF Color )
{
	return CreatePen( PS_SOLID, Width, Color );
}

HBRUSH EGWndBase::CreateSolidBrush( COLORREF Color )
{
	LOGBRUSH lb;
	zero( &lb );
	lb.lbStyle = BS_SOLID;
	lb.lbHatch = HS_CROSS;
	lb.lbColor = Color;
	return CreateBrushIndirect( &lb );
}

void EGWndBase::SetWndRect( const RECT& rcPos )
{
	SetWindowPos( m_hwnd, nullptr, rcPos.left, rcPos.top, rcPos.right - rcPos.left, rcPos.bottom - rcPos.top, SWP_NOACTIVATE | SWP_NOZORDER );
	FullRedraw();
}

void EGWndBase::SetWndRect( const eg_recti& Pose )
{
	SetWindowPos( GetWnd() , NULL , Pose.left , Pose.top , Pose.GetWidth() , Pose.GetHeight() , SWP_NOZORDER );
	FullRedraw();
}

void EGWndBase::SetWndPos( eg_int x, eg_int y, eg_int Width, eg_int Height )
{
	RECT rc = { x , y , x+Width , y+Height };
	SetWndRect( rc );
}

eg_string_big EGWndBase::GetWndText( HWND hwnd )
{
	eg_char16 Buffer[eg_string_big::STR_SIZE];
	GetWindowTextW( hwnd, Buffer, countof( Buffer ) );
	return eg_string_big( Buffer );
}

eg_string_big EGWndBase::GetWndText() const
{
	return GetWndText( GetWnd() );
}

void EGWndBase::DrawRect( HDC hdc , const RECT& rc )
{
	Rectangle( hdc , rc.left , rc.top , rc.right , rc.bottom );
}

void EGWndBase::DrawBorder( HDC hdc, const RECT& rc )
{
	MoveToEx( hdc , rc.left , rc.top , nullptr );
	LineTo( hdc , rc.right , rc.top );
	LineTo( hdc , rc.right , rc.bottom );
	LineTo( hdc , rc.left , rc.bottom );
	LineTo( hdc , rc.left , rc.top );
}

void EGWndBase::EnableDoubleBuffering( eg_bool bDoubleBuffer )
{
	m_bDoubleBufferDraw = bDoubleBuffer;
	FullRedraw();
}

void EGWndBase::ScrollContent( eg_int xAmount, eg_int yAmount )
{
	if( m_bDoubleBufferDraw )
	{
		FullRedraw();
	}
	else
	{
		RECT rcClient;
		GetClientRect( m_hwnd , &rcClient );
		ScrollWindow( m_hwnd , xAmount , yAmount , NULL , &rcClient );
		UpdateWindow( m_hwnd );
	}
}

void EGWndBase::FullRedraw()
{
	RedrawWindow( m_hwnd , nullptr , nullptr , RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW );
}

void EGWndBase::SetWndTimer( eg_uint64 TimerId, eg_uint FrequencyMs )
{
	SetTimer( m_hwnd , static_cast<UINT_PTR>(TimerId) , FrequencyMs , nullptr );
}

void EGWndBase::KillWndTimer( eg_uint64 TimerId )
{
	KillTimer( m_hwnd , static_cast<UINT_PTR>(TimerId) );
}

void EGWndBase::SetWndStyle( DWORD Style, eg_bool bEnabled )
{
	LONG CurStyle = GetWindowLongW( GetWnd() , GWL_STYLE );

	if( bEnabled )
	{
		CurStyle = CurStyle|Style;
	}
	else
	{
		CurStyle = CurStyle&(~Style);
	}

	SetWindowLongW( GetWnd() , GWL_STYLE , CurStyle );
}

LRESULT EGWndBase::WndProc( UINT Msg, WPARAM wParam, LPARAM lParam )
{	
	auto DoubleBufferDraw = [this]() -> void
	{
		RECT rcClient = GetViewRect();

		eg_int Width = rcClient.right - rcClient.left;
		eg_int Height = rcClient.bottom - rcClient.top;

		PAINTSTRUCT ps;
		zero( &ps );
		HDC DC = BeginPaint( m_hwnd , &ps );
		HDC MemDC = CreateCompatibleDC( DC );
		HBITMAP MemBM = CreateCompatibleBitmap( DC , Width , Height );
		SelectObject( MemDC , MemBM );

		// BEGIN DRAW
		OnDrawBg( EGWndDc( MemDC ) );
		OnPaint( EGWndDc( MemDC ) );
		// END DRAW

		BitBlt( DC , 0 , 0 , Width , Height , MemDC , 0 , 0 , SRCCOPY );
		DeleteObject( MemBM );
		DeleteDC( MemDC );
		EndPaint( m_hwnd , &ps );
	};

	if( m_bRegisteredTabTo )
	{
		if( TabToProcHandler( GetWnd() , Msg , wParam , lParam ) )
		{
			return 0;
		}
	}

	switch( Msg )
	{
	case WM_EG_COMMIT:
	{
		OnWmEgCommit( reinterpret_cast<HWND>(lParam) );
	} return 0;
	case WM_NOTIFY:
	{
		OnWmNotify( reinterpret_cast<HWND>(wParam) , *reinterpret_cast<NMHDR*>(lParam) );
	} return 0;
	case WM_CHAR:
	{
		OnWmChar( static_cast<eg_char16>(wParam) , static_cast<eg_uint32>(lParam) );
	} return 0;
	case WM_SIZE:
	{
		OnWmSize( eg_ivec2( LOWORD(lParam) , HIWORD(lParam) ) );
	} return 0;
	case WM_TIMER:
	{
		OnWmTimer( wParam );
	} return 0;
	case WM_LBUTTONDOWN:
	{
		OnWmLButtonDown( eg_ivec2( GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) ) );
	} return 0;
	case WM_NCLBUTTONDOWN:
	{
		EGWnd_ClearFocus();
	} return DefWindowProcW( GetWnd() , Msg , wParam , lParam );
	case WM_LBUTTONUP:
	{
		OnWmLButtonUp( eg_ivec2( GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) ) );
	} return 0;
	case WM_RBUTTONDOWN:
	{
		OnWmRButtonDown( eg_ivec2( GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) ) );
	} return 0;
	case WM_NCRBUTTONDOWN:
	{
		EGWnd_ClearFocus();
	} return DefWindowProcW( GetWnd() , Msg , wParam , lParam );
	case WM_RBUTTONUP:
	{
		OnWmRButtonUp( eg_ivec2( GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) ) );
	} return 0;
	case WM_LBUTTONDBLCLK:
	{
		const eg_ivec2 MousePos( GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) );
		if( m_bDoubleClicksAllowed )
		{
			OnWmLButtonDoubleClick( MousePos );
		}
		else
		{
			OnWmLButtonDown( MousePos );
		}
	} return 0;
	case WM_RBUTTONDBLCLK:
	{
		const eg_ivec2 MousePos( GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) );
		if( m_bDoubleClicksAllowed )
		{
			OnWmRButtonDoubleClick( MousePos );
		}
		else
		{
			OnWmRButtonDown( MousePos );
		}
	} return 0;
	case WM_MOUSEMOVE:
	{
		OnWmMouseMove( eg_ivec2( GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) ) );
	} return 0;
	case WM_VSCROLL:
	{
		OnWmVScroll( LOWORD(wParam) , HIWORD(wParam) );
	} return 0;
	case WM_HSCROLL:
	{
		OnWmHScroll( LOWORD(wParam) , HIWORD(wParam) );
	} return 0;
	case WM_MOUSEWHEEL:
	{
		eg_bool bHandled = OnWmMouseWheel( GET_WHEEL_DELTA_WPARAM(wParam) );
		if( !bHandled )
		{
			return DefWindowProcW( GetWnd() , Msg, wParam, lParam );
		}
	} return 0;
	case WM_MOUSELEAVE:
	{
		// assert( m_bTrackingMouseLeave ); // Bad flow?
		m_bTrackingMouseLeave = false;
		OnWmMouseLeave();
	} return 0;
	case WM_CANCELMODE:
	{
		if( m_Capture.IsCapturing() )
		{
			m_Capture.OnCancelMode();
			OnCaptureLost();
		}
	} return 0;
	case WM_ERASEBKGND:
	{
		if( m_bDoubleBufferDraw )
		{

		}
		else
		{
			HDC hdc = reinterpret_cast<HDC>( wParam );
			OnDrawBg( EGWndDc( hdc ) );
		}
	} return 0;
	case WM_PAINT:
	{
		if( m_bDoubleBufferDraw )
		{
			DoubleBufferDraw();
		}
		else
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint( m_hwnd, &ps );
			OnPaint( EGWndDc( hdc ) );
			EndPaint( m_hwnd, &ps );
		}
	} return 0;
	case WM_COMMAND:
	{
		if( lParam != 0 )
		{
			HWND WndControl = reinterpret_cast<HWND>(lParam);
			OnWmControlCmd( WndControl , HIWORD(wParam) , LOWORD(wParam) );
		}
		else
		{
			OnWmMenuCmd( LOWORD(wParam) , HIWORD(wParam) == 1 );
		}
	} return 0;
	case WM_ACTIVATEAPP:
	{
		OnWmActivateApp( wParam == TRUE , static_cast<DWORD>(lParam) );
	} return 0;
	case WM_QUIT:
	{
		OnWmQuit( static_cast<eg_int>(wParam) );
	} return 0;
	case WM_CLOSE:
	{
		OnWmClose();	
	} return 0;
	case WM_GETMINMAXINFO:
	{
		OnWmGetMinMaxInfo( reinterpret_cast<MINMAXINFO*>(lParam) );
	} return 0;
	case WM_CTLCOLOREDIT:
	{
		HBRUSH EditBrush = OnWmCtlColorEdit( reinterpret_cast<HWND>(lParam) , reinterpret_cast<HDC>(wParam) );
		if( EditBrush )
		{
			return reinterpret_cast<LRESULT>(EditBrush);
		}
	} break;
	case WM_CTLCOLORSTATIC:
	{
		HBRUSH EditBrush = OnWmCtlColorStatic( reinterpret_cast<HWND>(lParam) , reinterpret_cast<HDC>(wParam) );
		if( EditBrush )
		{
			return reinterpret_cast<LRESULT>(EditBrush);
		}
	} break;
	}

	if( Msg >= WM_USER && OnWmUserMsg( Msg , wParam , lParam ) )
	{
		return 0;
	}

	return DefWindowProcW( GetWnd() , Msg, wParam, lParam );
}

void EGWndBase::OnPaint( HDC hdc )
{
	unused( hdc );
}

void EGWndBase::OnPaint( EGWndDc& Dc )
{
	OnPaint( Dc.GetWindowsDC() );
}

void EGWndBase::OnDrawBg( HDC hdc )
{
	FillRect( hdc , &GetViewRect() , EGWnd_GetBrush( egw_color_t::GARBAGE ) );
}

void EGWndBase::OnDrawBg( EGWndDc& Dc )
{
	OnDrawBg( Dc.GetWindowsDC() );
}

void EGWndBase::OnWmLButtonDown( const eg_ivec2& MousePos )
{
	unused( MousePos );

	EGWnd_ClearFocus();
}

void EGWndBase::OnWmRButtonDown( const eg_ivec2& MousePos )
{
	unused( MousePos );

	EGWnd_ClearFocus();
}


void EGWndBase::SetVisible( eg_bool bVisible )
{
	ShowWindow( m_hwnd , bVisible ? SW_SHOWNORMAL : SW_HIDE );
}

eg_bool EGWndBase::IsVisible() const
{
	return IsWindowVisible( GetWnd() ) == TRUE;
}

void EGWndBase::BeginCapture( const eg_ivec2& MousePos, HCURSOR CursorToUse /*= nullptr */ )
{
	m_Capture.OnBeginCapture( GetWnd() , MousePos.x , MousePos.y , CursorToUse );
}

void EGWndBase::EndCapture( const eg_ivec2& MousePos )
{
	if( m_Capture.IsCapturing() )
	{
		m_Capture.OnEndCapture();
		OnEndCapture( MousePos );
	}
}

eg_ivec2 EGWndBase::GetBeginCapturePos() const
{
	assert( m_Capture.IsCapturing() ) ; // Why would you need this now.
	POINT CapturePoint = m_Capture.GetBeginCaptureMousePos();
	return eg_ivec2( CapturePoint.x , CapturePoint.y );
}

void EGWndBase::TrackMouseLeave()
{
	if( !m_bTrackingMouseLeave )
	{
		m_bTrackingMouseLeave = true;

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = GetWnd();
		TrackMouseEvent( &tme );
	}
}

eg_bool EGWndBase::TabToProcHandler( HWND hWnd , UINT uMsg , WPARAM wParam , LPARAM lParam )
{
	unused( lParam );

	switch( uMsg )
	{
	case WM_CHAR:
	{
		eg_bool bShiftHeld = (GetKeyState ( VK_SHIFT )&0x8000) != 0;

		switch( wParam )
		{
		case VK_TAB:
		case VK_ESCAPE:
			return true;
		}

	} break;
	case WM_KEYDOWN:
	{
		eg_bool bShiftHeld = (GetKeyState ( VK_SHIFT )&0x8000) != 0;

		switch( wParam )
		{
		case VK_TAB:
			EGWnd_TabTo( hWnd , bShiftHeld ? -1 : 1 );
			return true;
		case VK_ESCAPE:
			EGWnd_ClearFocus();
			return true;
		}

	} break;
	case WM_DESTROY:
	{
		EGWnd_UnregisterTabTo( hWnd );
	} break;
	}
	return false;
}