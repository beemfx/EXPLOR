// UI Clear Z Object
// (c) 2016 Beem Media
#pragma once
#include "EGUiWidget.h"

class EGUiClearZWidget: public EGUiWidget
{
	EG_CLASS_BODY( EGUiClearZWidget , EGUiWidget )

public:
	virtual void Init( EGMenu* InOwner , const egUiWidgetInfo* InInfo ) override final;
	virtual void Draw( eg_real AspectRatio ) override final;
	virtual eg_bool IgnoreDuringSetAllVisibilty() const override final { return true; }
};
