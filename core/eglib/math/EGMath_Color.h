// (c) 2017 Beem Media

#pragma once

struct eg_color32;
struct eg_color;

struct eg_color32
{
	union
	{
		eg_uint32 AsU32;
		struct  
		{
			eg_uint8 A;
			eg_uint8 R;
			eg_uint8 G;
			eg_uint8 B;
		};
	};

	eg_color32() = default;
	eg_color32( eg_ctor_t Ct ){ if( Ct == CT_Default || Ct == CT_Clear ){ A = 255; R=G=B=0; } }
	eg_color32( eg_uint8 RIn , eg_uint8 GIn , eg_uint8 BIn ): A(255) , R(RIn) , G(GIn) , B(BIn){ }
	eg_color32( eg_uint8 AIn , eg_uint8 RIn , eg_uint8 GIn , eg_uint8 BIn ): A(AIn) , R(RIn) , G(GIn) , B(BIn){ }
	explicit eg_color32( eg_uint32 AsU32In ):AsU32(AsU32In){ }
	explicit eg_color32( const eg_color& rhs );

	eg_bool operator==( const eg_color32& rhs )
	{
		return AsU32 == rhs.AsU32;
	}

	eg_bool operator!=( const eg_color32& rhs )
	{
		return !(*this == rhs);
	}

	static const eg_color32 Black;
	static const eg_color32 White;
	static const eg_color32 Red;
	static const eg_color32 Blue;
	static const eg_color32 Green;
	static const eg_color32 Yellow;
	static const eg_color32 Magenta;
	static const eg_color32 Cyan;
	static const eg_color32 ClearBlack;
	static const eg_color32 ClearWhite;
};

static_assert( sizeof(eg_color32) == 4 , "eg_color32 wrong size!" );

struct eg_color
{
	eg_real r, g, b, a;

	eg_color() = default;
	eg_color(eg_real fr, eg_real fg, eg_real fb, eg_real fa):r(fr), g(fg), b(fb), a(fa){ }
	explicit eg_color( const eg_color32& dw );

	const eg_vec4 ToVec4()const{ return eg_vec4(r,g,b,a); }

	explicit eg_color( const eg_vec4& rhs )
	{
		r = rhs.x;
		g = rhs.y;
		b = rhs.z;
		a = rhs.w;
	}

	friend inline eg_color operator*( const eg_color& lhs , const eg_color& rhs )
	{
		eg_color Out;
		Out.r = lhs.r * rhs.r;
		Out.g = lhs.g * rhs.g;
		Out.b = lhs.b * rhs.b;
		Out.a = lhs.a * rhs.a;
		return Out;
	}

	static const eg_color Black;
	static const eg_color White;
	static const eg_color Red;
	static const eg_color Blue;
	static const eg_color Green;
	static const eg_color Yellow;
	static const eg_color Magenta;
	static const eg_color Cyan;
	static const eg_color ClearBlack;
	static const eg_color ClearWhite;
};
