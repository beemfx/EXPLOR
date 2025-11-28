// (c) 2020 Beem Media. All Rights Reserved.

#include "EGMath_Color.h"

const eg_color32 eg_color32::Black(0,0,0);
const eg_color32 eg_color32::White(255,255,255);
const eg_color32 eg_color32::Red(255,0,0);
const eg_color32 eg_color32::Blue(0,0,255);
const eg_color32 eg_color32::Green(0,255,0);
const eg_color32 eg_color32::Yellow(255,255,0);
const eg_color32 eg_color32::Magenta(255,0,255);
const eg_color32 eg_color32::Cyan(0,255,255);
const eg_color32 eg_color32::ClearBlack(0,0,0,0);
const eg_color32 eg_color32::ClearWhite(0,255,255,255);

const eg_color eg_color::Black(0.f,0.f,0.f,1.f);
const eg_color eg_color::White(1.f,1.f,1.f,1.f);
const eg_color eg_color::Red(1.f,0.f,0.f,1.f);
const eg_color eg_color::Blue(0.f,0.f,1.f,1.f);
const eg_color eg_color::Green(0.f,1.f,0.f,1.f);
const eg_color eg_color::Yellow(1.f,1.f,0.f,1.f);
const eg_color eg_color::Magenta(1.f,0.f,1.f,1.f);
const eg_color eg_color::Cyan(0.f,1.f,1.f,1.f);
const eg_color eg_color::ClearBlack(0.f,0.f,0.f,0.f);
const eg_color eg_color::ClearWhite(1.f,1.f,1.f,0.f);

eg_color32::eg_color32( const eg_color& rhs )
{
	R = rhs.r >= 1.0f ? 0xff : rhs.r <= 0.0f ? 0x00 : static_cast<eg_uint8>(rhs.r * 255.0f + 0.5f);
	G = rhs.g >= 1.0f ? 0xff : rhs.g <= 0.0f ? 0x00 : static_cast<eg_uint8>(rhs.g * 255.0f + 0.5f);
	B = rhs.b >= 1.0f ? 0xff : rhs.b <= 0.0f ? 0x00 : static_cast<eg_uint8>(rhs.b * 255.0f + 0.5f);
	A = rhs.a >= 1.0f ? 0xff : rhs.a <= 0.0f ? 0x00 : static_cast<eg_uint8>(rhs.a * 255.0f + 0.5f);
}

eg_color::eg_color( const eg_color32& dw )
{
	static const eg_real f = 1.0f / 255.0f;
	r = f * static_cast<eg_real>(dw.R);
	g = f * static_cast<eg_real>(dw.G);
	b = f * static_cast<eg_real>(dw.B);
	a = f * static_cast<eg_real>(dw.A);
}