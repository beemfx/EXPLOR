// (c) 2017 Beem Media

#include "EGWndChildBase.h"

eg_cpstr16 EGWndChildBase::CLASS_NAME = L"EGWndChildBase";

void EGWndChildBase::InitClass( HINSTANCE hInst )
{
	WNDCLASSEXW wcex;
	wcex.cbSize         = sizeof(WNDCLASSEX);
	wcex.style          = CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
	wcex.lpfnWndProc    = EGWndBase::WndProcShell;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInst;
	wcex.hIcon          = NULL;
	wcex.hCursor        = LoadCursorW( nullptr , IDC_ARROW );
	wcex.hbrBackground  = NULL;
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = CLASS_NAME;
	wcex.hIconSm        = NULL;
	RegisterClassExW( &wcex );
}

void EGWndChildBase::DeinitClass( HINSTANCE hInst )
{
	UnregisterClassW( CLASS_NAME , hInst );
}

EGWndChildBase::EGWndChildBase( EGWndBase* OwnerWindow )
: EGWndBase( OwnerWindow->GetWnd() , CLASS_NAME , L"" , WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN )
, m_OwnerWindow( OwnerWindow )
{

}

EGWndChildBase::~EGWndChildBase()
{

}
