// (c) 2017 Beem Media

#pragma once

#include "EGWndPanel.h"
#include "EGCamera2.h"
#include "EGCameraController.h"
#include "EGForceSampler.h"

class EGComponent;

class EGDefEdPreviewPanel : public EGWndPanel
{
	EG_DECL_SUPER( EGWndPanel )

private:

	enum class eg_capture_reason
	{
		None,
		MoveCamera,
		MoveComponent,
	};

public:

	EGDefEdPreviewPanel( EGWndPanel* Parent );
	~EGDefEdPreviewPanel();

	virtual void OnPaint( HDC hdc ) override final;
	virtual void OnDrawBg( HDC hdc ) override final;
	virtual void OnWmLButtonDown( const eg_ivec2& MousePos ) override final;
	virtual void OnWmLButtonUp( const eg_ivec2& MousePos ) override final;
	virtual void OnWmRButtonDown( const eg_ivec2& MousePos ) override final;
	virtual void OnWmRButtonUp( const eg_ivec2& MousePos ) override final;
	virtual void OnWmMouseMove( const eg_ivec2& MousePos ) override final;
	virtual void OnWmSize( const eg_ivec2& NewClientSize ) override final;
	virtual void OnCaptureLost() override final;
	virtual void OnEndCapture( const eg_ivec2& MousePos ) override final;
	void HandleAppUpdate( eg_real DeltaTime );

	eg_real GetAspectRatio() const;

	static EGDefEdPreviewPanel* GetPanel() { return GlobalPreviewPanel; }

private:

	RECT GetCustomWindowRect() const override final;
	EGComponent* GetCaptureObj( const eg_ivec2& MousePos );

private:

	EGComponent*       m_CaptureObj;
	eg_transform       m_CaptureObjIntialPos;
	eg_capture_reason  m_CaptureReason = eg_capture_reason::None;
	eg_vec3            m_LastMouseInput = CT_Clear;
	EGForceSampler     m_MouseSmoother;
	eg_vec3            m_LastMoveForce = CT_Clear;
	EGForceSampler     m_MoveSmoother;

private:

	static EGDefEdPreviewPanel* GlobalPreviewPanel;
};
