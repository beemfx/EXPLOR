// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

typedef bool eg_bool;
typedef unsigned __int8 eg_byte;
typedef signed __int8 eg_int8;
typedef signed __int16 eg_int16;
typedef signed __int32 eg_int32;
typedef signed __int64 eg_int64;
typedef signed __int32 eg_int;

typedef float eg_real32;
typedef double eg_real64;
typedef float eg_real;

typedef char eg_char8;
typedef wchar_t eg_char16;
typedef char eg_char;

typedef eg_char* eg_pstr;
typedef const eg_char* eg_cpstr;
typedef eg_char8* eg_pstr8;
typedef const eg_char8* eg_cpstr8;
typedef eg_char16* eg_pstr16;
typedef const eg_char16* eg_cpstr16;

typedef eg_char16 eg_loc_char;
typedef eg_char16 eg_filename_char;

#if _WIN64
typedef unsigned __int64 eg_intptr;
typedef unsigned __int64 eg_mem_size;
typedef unsigned __int64 eg_file_size;
#else
typedef unsigned __int32 eg_intptr;
typedef unsigned __int32 eg_mem_size;
typedef unsigned __int32 eg_file_size;
#endif

static const eg_real EG_SMALL_EPS = 0.0001f; //For floating point comparisons. Etc.
static const eg_real EG_SMALL_NUMBER = .001f; // 1mm.
static const eg_real EG_REALLY_SMALL_NUMBER = .00001f;

static_assert( sizeof(void*) == sizeof(eg_intptr) , "Bad primitive declaration for eg_intptr" );
static_assert( sizeof(void*) == sizeof(eg_mem_size) , "Bad primitive declaration for eg_mem_size" );
static_assert( sizeof(void*) == sizeof(eg_file_size) , "Bad primitive declaration for eg_file_size" );

enum eg_ctor_t
{
	CT_Clear ,    // Hint that memory in the class should be cleared
	CT_Preserve , // Hint that memory in the class should be preserved
	CT_Default ,  // Hint to use default initialization
};

static const eg_real EG_PI_CONST = 3.141592654f;

struct eg_angle
{
private:

	eg_real m_RadValue;

public:

	static const eg_angle Zero;
	static const eg_angle PI;

public:

	static const eg_angle FromDeg( eg_real InDegrees ) { eg_angle Out; Out.m_RadValue = ToRad( InDegrees ); return Out; }
	static const eg_angle FromRad( eg_real InRadians ) { eg_angle Out; Out.m_RadValue = InRadians; return Out; }

	eg_real ToDeg() const { return ToDeg( m_RadValue ); }
	eg_real ToRad() const { return m_RadValue; }

	eg_angle operator -() const { return FromRad( -m_RadValue ); }

	eg_angle operator + ( const eg_angle& rhs ) const { return FromRad( m_RadValue + rhs.m_RadValue ); }
	const eg_angle& operator += ( const eg_angle& rhs ) { *this = *this + rhs; return *this; }
	eg_angle operator - ( const eg_angle& rhs ) const { return FromRad( m_RadValue - rhs.m_RadValue ); }
	const eg_angle& operator -= ( const eg_angle& rhs ) { *this = *this - rhs; return *this; }

	eg_angle operator * ( eg_real Scalar ) const { return FromRad(m_RadValue * Scalar); }
	const eg_angle& operator *= ( eg_real Scalar ) { *this = (*this * Scalar); return *this; }

	void NormalizeThis()
	{
		while( m_RadValue < 0.f )
		{
			m_RadValue += PI.ToRad()*2.f;
		}
		while( m_RadValue >= PI.ToRad()*2.f )
		{
			m_RadValue -= PI.ToRad()*2.f;
		}
	}

private:

	static inline eg_real ToRad( eg_real d ){ return ((d) * (EG_PI_CONST / 180.0f)); }
	static inline eg_real ToDeg( eg_real r ){ return ((r) * (180.0f / EG_PI_CONST)); }
};

static inline eg_angle EG_Deg( eg_real InDegrees ) { return eg_angle::FromDeg( InDegrees ); }
static inline eg_angle EG_Rad( eg_real InRadians ) { return eg_angle::FromRad( InRadians ); }

class eg_str_const8 // http://en.cppreference.com/w/cpp/language/constexpr
{
private:

	const eg_char8* const p_;
	const eg_int sz_;

public:

	eg_str_const8() = delete;
	eg_str_const8( const eg_str_const8& rhs ) = delete;

	template<eg_int N>
	constexpr eg_str_const8( const eg_char8(&a)[N] ): p_(a)	, sz_(N-1) { }
	constexpr eg_char8 operator[](eg_int n) const { return p_[n]; }
	constexpr eg_int size() const { return sz_; }
};

class eg_str_const16 // http://en.cppreference.com/w/cpp/language/constexpr
{
private:

	const eg_char16* const p_;
	const eg_int sz_;

public:

	eg_str_const16() = delete;
	eg_str_const16( const eg_str_const16& rhs ) = delete;

	template<eg_int N> constexpr eg_str_const16( const eg_char16(&a)[N] ) : p_(a) , sz_(N-1) { }
	constexpr eg_char16 operator[](eg_int n) const { return p_[n]; }
	constexpr eg_int size() const { return sz_; }
};

struct eg_irect
{
	eg_int Left, Top, Right, Bottom;

	eg_irect() = default;
	eg_irect( eg_int InLeft , eg_int InTop , eg_int InRight , eg_int InBottom ) : Left( InLeft ) , Top( InTop ) , Right( InRight ) , Bottom( InBottom ){ }

	eg_int GetWidth()const{ return (Right-Left); }
	eg_int GetHeight()const{ return (Bottom-Top); }

	void OffsetThis( eg_int OffsetX , eg_int OffsetY ) { Left += OffsetX; Right += OffsetX; Top += OffsetY; Bottom += OffsetY; }
	void InflateThis( eg_int InflateX , eg_int InflateY ) { Left -= InflateX; Right += 2*InflateX; Top -= InflateY; Bottom += 2*InflateY; }
};

struct eg_rect
{
	eg_real Left, Top, Right, Bottom;

	eg_rect() = default;
	eg_rect( eg_real InLeft , eg_real InTop , eg_real InRight , eg_real InBottom ) : Left( InLeft ) , Top( InTop ) , Right( InRight ) , Bottom( InBottom ){ }

	eg_real GetWidth() const { return (Right-Left); }
	eg_real GetHeight() const { return (Bottom-Top); }
};
