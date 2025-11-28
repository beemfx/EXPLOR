// (c) 2017 Beem Media

#include "EGWndAppWindow.h"

eg_cpstr16 EGWndAppWindow::CLASS_NAME = L"EG_WINDOWS_APP";

EGWndAppWindow::EGWndAppWindow( eg_cpstr16 AppName )
: EGWndBase( nullptr, CLASS_NAME, AppName, WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN )
{

}

EGWndAppWindow::~EGWndAppWindow()
{

}

eg_int EGWndAppWindow::GetAppHeight() const
{
	RECT rcWnd = GetWndRect();
	return rcWnd.bottom - rcWnd.top;
}

eg_int EGWndAppWindow::GetAppWidth() const
{
	RECT rcWnd = GetWndRect();
	return rcWnd.right - rcWnd.left;
}

void EGWndAppWindow::SetAppWidth( eg_int DesiredWidth )
{
	RECT rcDesktop;
	GetWindowRect( GetDesktopWindow() , &rcDesktop );
	RECT rcWnd = GetWndRect();
	rcWnd.left = (rcDesktop.right - EGWnd_GetAppScaledSize(DesiredWidth))/2;
	rcWnd.right = rcWnd.left + EGWnd_GetAppScaledSize(DesiredWidth);
	SetWndRect( rcWnd );
}

void EGWndAppWindow::SetAppHeight( eg_int DesiredHeight )
{
	RECT rcDesktop;
	GetWindowRect( GetDesktopWindow() , &rcDesktop );
	RECT rcWnd = GetWndRect();
	rcWnd.top = (rcDesktop.top + EGWnd_GetAppScaledSize(DesiredHeight))/2;
	rcWnd.bottom = rcWnd.top + EGWnd_GetAppScaledSize(DesiredHeight);
	SetWndRect( rcWnd );
}

void EGWndAppWindow::InitClass( HINSTANCE hInst , HMODULE hResModule , eg_cpstr16 Icon , eg_cpstr16 IconSm )
{
	WNDCLASSEXW wcex;
	wcex.cbSize         = sizeof(WNDCLASSEX);
	wcex.style          = 0;//CS_HREDRAW|CS_VREDRAW;
	wcex.lpfnWndProc    = EGWndBase::WndProcShell;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInst;
	wcex.hIcon          = LoadIconW( hResModule , Icon );
	wcex.hCursor        = LoadCursorW( nullptr , IDC_ARROW );
	wcex.hbrBackground  = NULL;
	wcex.lpszMenuName   = nullptr;
	wcex.lpszClassName  = CLASS_NAME;
	wcex.hIconSm        = LoadIconW( hResModule , IconSm );
	RegisterClassExW(&wcex);
}

void EGWndAppWindow::DeinitClass( HINSTANCE hInst )
{
	UnregisterClassW( CLASS_NAME , hInst );
}

