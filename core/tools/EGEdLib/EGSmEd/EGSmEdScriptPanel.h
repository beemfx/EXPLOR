// (c) 2017 Beem Media

#pragma once

#include "EGWndPanel.h"
#include "EGSmEdNodeWnd.h"
#include "EGSmEdVarMgr.h"
#include "../EGEdResLib/resource.h"
#include "EGWndHelpers.h"
#include "EGSmEd.h"

class ISmEdApp;

class EGSmEdScriptPanel : public EGWndPanel , public ISmEditor
{
private: typedef EGWndPanel Super;

private:

	static const COLORREF CONN_COLOR = RGB(255,0,0);
	static const COLORREF BG_COLOR = RGB(70,72,70);

	struct egConnector
	{
		eg_size_t FromStateIndex;
		eg_size_t FromChoiceIndex;
		eg_size_t ToStateIndex;

		POINT LastDrawnStart;
		POINT LastDrawnEnd;

		eg_bool bHasBeenDrawn:1;
		eg_bool bActive:1;

		egConnector()
		: bHasBeenDrawn(false)
		, bActive(false)
		{
		}
	};

	struct egConvState
	{
		EGSmEdNodeWnd* Wnd;

		void Init( ISmEditor* ConvEditor , ISmEdApp* App , eg_size_t StateIndex )
		{
			assert( nullptr == Wnd );
			Wnd = new EGSmEdNodeWnd( ConvEditor , App , StateIndex );
		}

		void Deinit()
		{
			EG_SafeDelete( Wnd );
		}

		egConvState()
		: Wnd( nullptr )
		{

		}
	};

private:

	ISmEdApp*            m_App = nullptr;
	eg_bool              m_IsDraggingChoice:1;
	EGArray<egConnector> m_Connectors;
	EGArray<egConvState> m_StateWnds;
	egConnector          m_DragConnector;
	eg_vec2              m_ContextCmdPos;
	eg_size_t            m_DraggingFromStateIndex;
	eg_size_t            m_DraggingFromChoiceIndex;
	POINT                m_DragConnectorDest;
	HBRUSH               m_BgBrush;
	HPEN                 m_ConnPen;
	HPEN                 m_ConnErasePen;
	POINT                m_LastViewDragPos;

public:

	EGSmEdScriptPanel( EGWndPanel* Parent );
	virtual ~EGSmEdScriptPanel() override;
	void Init( ISmEdApp* App ){ m_App = App; }
	void ReinitializeStates();
	virtual void OnStatesChanged( eg_bool bOnlyRepaint ) override final;
	void OnAppLostFocus();
	void OnFocusChanged( const ISmEdApp::egFocus& NewFocus , const ISmEdApp::egFocus& OldFocus );

private:

	virtual LRESULT WndProc( UINT Msg , WPARAM wParam , LPARAM lParam ) override;
	virtual void OnWmMouseMove( const eg_ivec2& MousePos ) override;
	virtual void OnPaint( HDC hdc ) override;
	virtual void OnDrawBg( HDC hdc ) override;
	virtual void OnWmMenuCmd( eg_int CmdId , eg_bool bFromAccelerator ) override;
	void CreateState( egsm_node_t Type , const eg_vec2& Pos );
	void CreateNativeEvent( const egsmVarDeclScr& FnDecl , const eg_vec2& Pos );
	void UpdateChildrenView();
	static POINT ToPoint( const eg_vec2& v );
	POINT GetViewOffset() const;
	void GetConnectorPos( const egConnector& Conn , POINT* StartOut , POINT* EndOut ) const;
	void DrawConnector( HDC hdc , egConnector& Conn , eg_bool bDrawToDrag );
	virtual EGWndBase* GetOwnerWnd() override final{ return this; }
	virtual void StartDrag( eg_size_t FromState , eg_size_t FromChoice ) override final;
	virtual void EndDrag( eg_size_t HitStateIndex, eg_bool bDontClear ) override final;
	virtual void SetDragHitNode( eg_size_t StateIndex ) override final;
	virtual eg_size_t SetDragHitMousePos( const eg_ivec2& GlobalMousePose ) override final;
	virtual POINT GetView() const override final;
	virtual void SetDirty() override final;
};
