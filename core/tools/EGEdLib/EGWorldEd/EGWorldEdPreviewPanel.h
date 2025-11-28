// (c) 2018 Beem Media

#pragma once

#include "EGWndPanel.h"
#include "EGCameraController.h"
#include "EGForceSampler.h"

class EGWorldEdPreviewPanel : public EGWndPanel
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

	EGWorldEdPreviewPanel( EGWndPanel* Parent );
	~EGWorldEdPreviewPanel();

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

	eg_real GetAspectRatio() const;
	void SetCameraPose( const eg_transform& NewCameraPose );
	void HandleAppUpdate( eg_real DeltaTime );

private:

	RECT GetCustomWindowRect() const override final;

private:

	eg_transform       m_CaptureObjIntialPos;
	eg_capture_reason  m_CaptureReason = eg_capture_reason::None;
	EGCameraController m_CameraController;
	eg_vec3            m_LastMouseInput = CT_Clear;
	EGForceSampler     m_MouseSmoother;
	eg_vec3            m_LastMoveForce = CT_Clear;
	EGForceSampler     m_MoveSmoother;
};
