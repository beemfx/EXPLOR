// (c) 2017 Beem Media

#pragma once

#include "EGDataAsset.h"
#include "ExDayNightCycles.reflection.h"

struct exDayNightCycleState
{
	eg_real  NormalizedTime = 0.f;
	eg_color SunColor = eg_color(1.f,1.f,1.f,1.f);
	eg_color TorchColor = eg_color(1.f,1.f,1.f,1.f);
	eg_real  SunIntensity = 1.f;
	eg_real  TorchIntensity = 1.f;
	eg_real  AmbientIntensity = 1.f;
};

egreflect struct exDayNightCycleColor
{
	egprop eg_string_crc Id               = CT_Clear;
	egprop eg_color32    SunColor         = eg_color32(eg_color::White);
	egprop eg_real       SunIntensity     = 1.f;
	egprop eg_color32    TorchColor       = eg_color32(eg_color::White);
	egprop eg_real       TorchIntensity   = 1.f;
	egprop eg_real       AmbientIntensity = 1.f;
};

egreflect struct exDayNightCycleBlock
{
	egprop eg_real       Hour         = 0.f;
	egprop eg_string_crc CycleColorId = CT_Clear;

};

egreflect struct exDayNightCycle
{
	egprop eg_string_crc                 Id = CT_Clear;
	egprop EGArray<exDayNightCycleBlock> CycleBlocks;
};

egreflect class ExDayNightCycles : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExDayNightCycles , EGDataAsset )
	EG_FRIEND_RFL( ExDayNightCycles )

private:
	egprop EGArray<exDayNightCycleColor> m_Colors;
	egprop EGArray<exDayNightCycle>      m_Cycles;

	static ExDayNightCycles* s_Inst;

public:

	static void Init( eg_cpstr Filename );
	static void Deinit();
	static ExDayNightCycles& Get(){ return *s_Inst; }

	void GetCycles( eg_string_crc Id , EGArray<exDayNightCycleState>& Out );
};