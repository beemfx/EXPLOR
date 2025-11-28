// (c) 2018 Beem Media

#pragma once

#include "EGAssetPath.h"
#include "EGReflection.h"
#include "EGRflPrimitiveTest.reflection.h"

egreflect struct egRflAllPrimitives
{
	egprop eg_string_crc StringCrcId;
	egprop eg_asset_path AssetPath;
	egprop eg_bool BoolValue;
	egprop eg_int IntValue;
	egprop eg_uint UnsignedValue;
	egprop eg_real RealValue;
	egprop eg_vec2 Vec2Value;
	egprop eg_vec3 Vec3Value;
	egprop eg_vec4 Vec4Value;
	egprop eg_ivec2 IntVec2Value;
	egprop eg_ivec3 IntVec3Value;
	egprop eg_ivec4 IntVec4Value;
	egprop eg_d_string StringValue;
	egprop eg_color32   ColorValue = CT_Default;
	egprop eg_transform TransformValue = CT_Default;
};
