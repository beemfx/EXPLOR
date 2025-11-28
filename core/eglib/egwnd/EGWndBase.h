// (c) 2017 Beem Media

#pragma once

#include "EGWnd.h"

class EGWndDc;

class EGWndBase
{
protected:

	static const eg_bool DOUBLE_BUFFER_BY_DEFAULT = true;

protected:

	HWND    m_hwnd;
	eg_bool m_bDoubleBufferDraw:1;
	eg_bool m_bRegisteredTabTo:1;

private:

	EGWndMouseCapture m_Capture;
	eg_bool           m_bTrackingMouseLeave = false;
	eg_bool           m_bDoubleClicksAllowed = false;

public:

	EGWndBase( eg_cpstr16 szWindowClass , eg_cpstr16 szTitle , DWORD Style );
	EGWndBase( HWND hwndParent , eg_cpstr16 szWindowClass , eg_cpstr16 szTitle , DWORD Style );

	virtual ~EGWndBase();

	HWND GetWnd()const{ return m_hwnd; }
	RECT GetViewRect()const{ RECT Out; GetClientRect( m_hwnd , &Out ); return Out; }
	RECT GetWndRect()const{ RECT Out; GetWindowRect( m_hwnd , &Out ); return Out; }
	eg_recti GetViewArea() const;
	eg_recti GetWndArea() const;
	void RegisterForTabTo( eg_bool bRegister );
	static LRESULT CALLBACK WndProcShell( HWND hwnd , UINT message , WPARAM wParam , LPARAM lParam );
	static HFONT CreateBasicFont( eg_cpstr Face , eg_int Size , eg_bool bIsBold , eg_bool bIsItalic );
	static void SetFont( HDC hdc , HFONT Font ){ SelectObject( hdc , static_cast<HGDIOBJ>(Font) ); }
	static HPEN CreateBasicPen( int Width , COLORREF Color );
	static void SetPen( HDC hdc , HPEN Pen ){ SelectObject( hdc , static_cast<HGDIOBJ>(Pen) ); }
	static HBRUSH CreateSolidBrush( COLORREF Color );
	static void SetBrush( HDC hdc , HBRUSH Brush ){ SelectObject( hdc , static_cast<HGDIOBJ>(Brush) ); }
	void SetWndRect( const RECT& rcPos );
	void SetWndRect( const eg_recti& Pose );
	void SetWndPos( eg_int x , eg_int y , eg_int Width , eg_int Height );
	static eg_string_big GetWndText( HWND hwnd );
	eg_string_big GetWndText() const;
	static void DrawRect( HDC hdc , const RECT& rc );
	static void DrawBorder( HDC hdc , const RECT& rc );

	void EnableDoubleBuffering( eg_bool bDoubleBuffer );
	void ScrollContent( eg_int xAmount, eg_int yAmount );

	void FullRedraw();

	void SetWndTimer( eg_uint64 TimerId , eg_uint FrequencyMs );
	void KillWndTimer( eg_uint64 TimerId );
	void SetWndStyle( DWORD Style , eg_bool bEnabled );
	void SetDoubleClicksAllowed( eg_bool bNewValue ) { m_bDoubleClicksAllowed = bNewValue; }

	virtual LRESULT WndProc( UINT Msg , WPARAM wParam , LPARAM lParam );

	// Windows Message Handlers
	virtual eg_bool OnWmUserMsg( UINT Msg , WPARAM wParam , LPARAM lParam ) { unused(  Msg , wParam , lParam ); return false; }
	virtual void OnPaint( HDC hdc );
	virtual void OnPaint( EGWndDc& Dc );
	virtual void OnDrawBg( HDC hdc );
	virtual void OnDrawBg(  EGWndDc& Dc );
	virtual void OnWmNotify( HWND WndControl , const NMHDR& NotifyInfo ){ unused( WndControl , NotifyInfo ); }
	virtual HBRUSH OnWmCtlColorEdit( HWND WndControl , HDC hdc ) { unused( WndControl , hdc ); return nullptr; }
	virtual HBRUSH OnWmCtlColorStatic( HWND WndControl , HDC hdc ) { unused( WndControl , hdc ); return nullptr; }
	virtual void OnWmLButtonDown( const eg_ivec2& MousePos );
	virtual void OnWmLButtonUp( const eg_ivec2& MousePos ){ unused( MousePos ); }
	virtual void OnWmRButtonDown( const eg_ivec2& MousePos );
	virtual void OnWmRButtonUp( const eg_ivec2& MousePos ){ unused( MousePos ); }
	virtual void OnWmMouseMove( const eg_ivec2& MousePos ){ unused( MousePos ); }
	virtual void OnWmChar( eg_char16 Char , eg_uint32 OtherInfo ){ unused( Char , OtherInfo ); }
	virtual void OnWmTimer( eg_uint64 TimerId ){ unused( TimerId ); }
	virtual void OnWmSize( const eg_ivec2& NewClientSize ){ unused( NewClientSize ); }
	virtual void OnWmVScroll( eg_int Request , eg_int ThumbPos ){ unused( Request , ThumbPos ); }
	virtual void OnWmHScroll( eg_int Request , eg_int ThumbPos ){ unused( Request , ThumbPos ); }
	virtual eg_bool OnWmMouseWheel( eg_int Delta ){ unused( Delta ); return false; }
	virtual void OnWmMouseLeave(){ }
	virtual void OnWmControlCmd( HWND WndControl , eg_int NotifyId , eg_int ControlId ){ unused( WndControl , NotifyId , ControlId ); }
	virtual void OnWmMenuCmd( eg_int CmdId , eg_bool bFromAccelerator ){ unused( CmdId , bFromAccelerator ); }
	virtual void OnWmActivateApp( eg_bool bActivating , DWORD ThreadId ){ unused( bActivating , ThreadId ); }
	virtual void OnWmQuit( eg_int ExitCode ){ unused( ExitCode ); }
	virtual void OnWmClose(){ }
	virtual void OnWmGetMinMaxInfo( MINMAXINFO* MinMaxInfo ){ unused( MinMaxInfo ); }
	virtual void OnWmEgCommit( HWND CommitCtrl ) { unused( CommitCtrl ); }
	virtual void OnWmLButtonDoubleClick( const eg_ivec2& MousePos ) { unused( MousePos ); }
	virtual void OnWmRButtonDoubleClick( const eg_ivec2& MousePos ) { unused( MousePos ); }

	virtual void SetVisible( eg_bool bVisible );
	eg_bool IsVisible() const;
	void BeginCapture( const eg_ivec2& MousePos , HCURSOR CursorToUse = nullptr );
	void EndCapture( const eg_ivec2& MousePos );
	eg_bool IsCapturing() const { return m_Capture.IsCapturing(); }
	eg_ivec2 GetBeginCapturePos() const;
	void TrackMouseLeave();

	virtual void OnEndCapture( const eg_ivec2& MousePos ){ unused( MousePos ); }
	virtual void OnCaptureLost(){ }

protected:

	static eg_bool TabToProcHandler( HWND hWnd , UINT uMsg , WPARAM wParam , LPARAM lParam );
};
