// UI Camera Object
// (c) 2016 Beem Media
#pragma once
#include "EGUiWidget.h"
#include "EGCamera2.h"

class EGUiCameraWidget: public EGUiWidget
{
	EG_CLASS_BODY( EGUiCameraWidget , EGUiWidget )

private:
	EGCamera2 m_Camera;

public:
	EGUiCameraWidget(): m_Camera( CT_Default ){ }

	virtual void Init( EGMenu* InOwner , const egUiWidgetInfo* InInfo ) override final;
	virtual void Draw( eg_real AspectRatio ) override final;
	virtual void Update( eg_real DeltaTime , eg_real AspectRatio ) override final;
	virtual eg_mat GetProjMatrix()const override final;
	virtual eg_mat GetViewMatrix()const override final;
	virtual eg_bool IgnoreDuringSetAllVisibilty() const override final { return true; }
};
