// (c) 2017 Beem Media

#pragma once

#include "EGEngineSerializeTypes.h"
#include "EGEntDefTypes.reflection.h"

egreflect enum class eg_ent_class_t
{
	None,
	Game,
	UI,
	GameAndUI,
};

egreflect struct egEntDefConfigOrthoCameraSetting
{
	egprop eg_real NearPlane = -100.f;
	egprop eg_real FarPlane = 100.f;
	egprop eg_real Height = 200.f;
};

egreflect struct egEntDefEdConfig
{
	egprop eg_color32           BackgroundColor = eg_color32(50,50,50);
	egprop eg_bool              bDrawGrid       = true;
	egprop eg_real              GridSpacing     = .5f;
	egprop eg_color32           AmbientLight    = eg_color32(eg_color(.2f,.2f,.2f,1.f));
	egprop eg_vec4              GamePalette0    = eg_vec4(1.f,1.f,1.f,1.f);
	egprop eg_vec4              GamePalette1    = eg_vec4(1.f,1.f,1.f,1.f);
	egprop eg_vec4              GamePalette2    = eg_vec4(1.f,1.f,1.f,1.f);
	egprop eg_bool              bDrawLights     = true;
	egprop EGArray<eg_light_ed> Lights;
	egprop eg_bool              bOrthoPreview = false;
	egprop egEntDefConfigOrthoCameraSetting OrthoCamera;
	egprop eg_vec3              CameraPos       = eg_vec3(0.f,0.f,0.f);
	egprop eg_angle             CameraYaw       = EG_Rad(0.f);
	egprop eg_angle             CameraPitch     = EG_Rad(0.f);
	egprop eg_bool              HasCameraPose   = false;

	void RefreshVisibleProperties( egRflEditor& ThisEditor );
};