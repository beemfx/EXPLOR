// (c) 2017 Beem Media

#pragma once

#include "EGWnd.h"
#include "EGSmFile2.h"
#include "EGWndChildBase.h"

class EGSmFile2;
class ISmEdApp;

class ISmEditor
{
public:
	virtual EGWndBase* GetOwnerWnd()=0;
	virtual void OnStatesChanged( eg_bool bOnlyRepaint = false )=0;
	virtual void StartDrag( eg_size_t FromState , eg_size_t FromChoice )=0;
	virtual void EndDrag( eg_size_t HitStateIndex, eg_bool bDontClear )=0;
	virtual void SetDragHitNode( eg_size_t StateIndex )=0;
	virtual eg_size_t SetDragHitMousePos( const eg_ivec2& GlobalMousePose ) = 0;
	virtual POINT GetView() const=0;
	virtual void SetDirty()=0;
};

class EGSmNodeConsts
{
private:

	static const DWORD STATE_WIDTH = 250;
	static const LONG STATE_HEIGHT = (12 + 6)*3;
	static const LONG CHOICE_HEIGHT = 12 + 4;
	static const LONG HEADER_START_BORDER_SIZE = 4;

public:

	static DWORD GetStateWidth() { return EGWnd_GetAppScaledSize( STATE_WIDTH ); }
	static LONG GetStateHeight() { return EGWnd_GetAppScaledSize( STATE_HEIGHT ); }
	static LONG GetChoiceHeight() { return EGWnd_GetAppScaledSize( CHOICE_HEIGHT ); }
	static LONG GetHeaderStartBorderSize() { return EGWnd_GetAppScaledSize( HEADER_START_BORDER_SIZE ); }
};

class EGSmEdNodeWnd: public EGWndChildBase
{
private: typedef EGWndChildBase Super;

private:


private:

	static const COLORREF BG_COLOR = RGB(200,200,200);
	static const COLORREF HEADER_START_BORDER_COLOR = RGB(0,0,0);
	static const COLORREF TERMINAL_BOX_COLOR = RGB(255,0,0);

	enum class eg_capture_reason
	{
		Moving,
		Connecting,
	};

private:

	ISmEditor*        m_ConvEditor;
	ISmEdApp*const    m_App;
	const eg_size_t   m_StateIndex;
	HFONT             m_Font;
	HFONT             m_FontHeader;
	HBRUSH            m_BgBrush;
	HBRUSH            m_TerminalBrush;
	HPEN              m_HeaderBorderPen;
	HPEN              m_BorderPen;
	HPEN              m_FocusPen;
	eg_size_t         m_EditIndex;
	POINT             m_LastPos;
	eg_capture_reason m_CaptureReason;
	egsmVarDeclScr    m_NativeEventDecl;
	eg_bool           m_bDragHasLeftBounds = false;

public:

	EGSmEdNodeWnd( ISmEditor* ConvEditor , ISmEdApp* App , eg_size_t StateIndex );
	virtual ~EGSmEdNodeWnd() override;

	void UpdateView();

	eg_bool DoesMouseHit( const eg_ivec2& GlobalMousePos ) const;
	eg_size_t GetStateIndex() const { return m_StateIndex; }

private:

	virtual void OnWmMouseMove( const eg_ivec2& MousePos ) override final;
	virtual void OnWmLButtonDown( const eg_ivec2& MousePos ) override final;
	virtual void OnWmLButtonUp( const eg_ivec2& MousePos ) override final;
	virtual void OnWmRButtonDown( const eg_ivec2& MousePos ) override final;
	virtual void OnWmRButtonUp( const eg_ivec2& MousePos ) override final;
	virtual void OnPaint( HDC hdc ) override final;
	virtual void OnDrawBg( HDC hdc ) override final;
	virtual void OnWmMenuCmd( eg_int CmdId , eg_bool bFromAccelerator ) override final;
	virtual void OnCaptureLost() override final;


private:

	const egsmNodeScr& GetMyNode() const;
	egsmNodeScr& GetMyNode();
	eg_size_t YHitToIndex( LONG y );
	eg_string_big FormatStateBody( const egsmNodeScr& State );
};