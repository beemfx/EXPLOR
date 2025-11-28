// (c) 2016 Beem Media

#pragma once

#include "EGUiWidget.h"
#include "EGEntObj.h"

class EGDebugSphere;

class EGUiLightWidget: public EGUiWidget
{
	EG_CLASS_BODY( EGUiLightWidget , EGUiWidget )

private:

	EGDebugSphere* m_DbLightObj;
	eg_color m_LightColor;
	eg_color m_AmbientColor;
	eg_real  m_Range;
	eg_real  m_Falloff;

public:
	virtual void Init( EGMenu* InOwner , const egUiWidgetInfo* InInfo ) override final;
	virtual ~EGUiLightWidget();
	virtual void Draw( eg_real AspectRatio ) override final;
	virtual void Update( eg_real DeltaTime, eg_real AspectRatio ) override final;
	virtual eg_aabb GetBoundsInMouseSpace( eg_real AspectRatio , const eg_mat& MatView , const eg_mat& MatProj )const override final;
	virtual eg_bool IgnoreDuringSetAllVisibilty() const override final { return true; }

	//
	// ISdkMenuObj Interface:
	//
	virtual void SetPalette( eg_uint PaletteIndex , const eg_vec4& Palette ) override final;

	eg_real GetObjRadius() const { eg_real ObjRadius = EG_Min( 10.f , m_Range*.5f ); return ObjRadius; }
};