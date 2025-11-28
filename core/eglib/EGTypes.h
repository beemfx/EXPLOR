/******************************************************************************
File: EGTypes.h
Purpose: Header for basic game types, these should be used rather than the
types built into C++.

(c) 2011 Beem Software
******************************************************************************/
#pragma once

/******************
*** Game types. ***
******************/

//Numerical types:

typedef unsigned long  eg_uint, eg_uint32;
typedef unsigned short eg_uint16;
typedef unsigned __int64 eg_uint64;

typedef unsigned char  eg_uint8;
#if defined( __WIN32__ )
typedef unsigned int   eg_size_t;
typedef signed int     eg_signed_size_t;
#elif defined( __WIN64__ )
typedef unsigned __int64 eg_size_t;
typedef signed __int64 eg_signed_size_t;
#else
#error Platform not set.
#endif

#if defined( __WIN32__ )
typedef eg_int32 eg_intptr_t;
typedef eg_uint32 eg_uintptr_t;
#elif defined( __WIN64__ )
typedef eg_int64 eg_intptr_t;
typedef eg_uint64 eg_uintptr_t;
#else
#error Platform not set.
#endif

//Character and string types:

static const eg_uint EG_MAX_PATH=128;
typedef eg_char eg_path[EG_MAX_PATH];
typedef eg_char8 eg_path8[EG_MAX_PATH];
typedef eg_char16 eg_path16[EG_MAX_PATH];

typedef eg_loc_char* eg_plocstr;
typedef const eg_loc_char* eg_cplocstr;

/***********************
*** Game Definitions ***
***********************/

static const eg_real EG_PI    = 3.141592654f;
static const eg_real EG_1BYPI = 0.318309886f;

static const eg_uint EG_ALIGNMENT=16;

#define EG_ALIGN __declspec(align(16))
#define EG_INLINE inline

//The print function type.
typedef void ( * EG_FN_PRINT)(eg_cpstr strFormat, ...);

/*************************
*** Flag functionality ***
*************************/
/*
	Flags are store in eg_uints a flag is a constant eg_uint with
	only one bit set (two could be set if the flag covers more than
	one sub flag). Flags can be added, removed, and checked from
	flag structures using this class.
*/
typedef eg_uint eg_flag;

#define EG_DECLARE_FLAG( name , index) static const eg_flag name = (1 << index)

class eg_flags
{
public:
	EG_INLINE eg_flags(): m_flags(0)                { }
	EG_INLINE eg_flags( eg_ctor_t Ct )              { if( Ct == CT_Clear || Ct == CT_Default ){ m_flags=0; } }
	EG_INLINE eg_flags(eg_flag rhs): m_flags(rhs)   { }
public:
	EG_INLINE eg_bool IsSet(const eg_flag flag)const{ return (m_flags&flag)!=0; }
	EG_INLINE void Set(const eg_flag flag)          { m_flags|=flag; }
	EG_INLINE void UnSet(const eg_flag flag)        { m_flags&=~flag; }
	EG_INLINE void Toggle( const eg_flag flag )     { if( IsSet(flag) ){ UnSet(flag); } else { Set(flag); } }
	EG_INLINE void SetState( const eg_flag flag , eg_bool On ){ if( On ){ Set(flag); }else{ UnSet( flag); } }
	EG_INLINE void operator=(const eg_flags& rhs)   { m_flags = rhs.m_flags; }
	EG_INLINE void Clear()                          { m_flags = 0; }
	EG_INLINE void operator|=(const eg_flag&rhs)    { Set(rhs); }
	EG_INLINE void operator|=(const eg_flags&rhs)   { Set(rhs.m_flags); }
	EG_INLINE void operator^=(const eg_flag& rhs)   { UnSet(rhs); }
	EG_INLINE void operator^=(const eg_flags& rhs)  { UnSet(rhs.m_flags); }
	EG_INLINE operator const eg_flag()const         { return m_flags;	}
private:
	eg_flag m_flags;
};

/***********************
*** Basic Structures ***
***********************/

struct eg_recti
{
	eg_int left, top, right, bottom;

	eg_recti() = default;
	eg_recti( eg_int InLeft , eg_int InTop , eg_int InRight , eg_int InBottom )	: left( InLeft ) , top( InTop ) , right( InRight ) , bottom( InBottom ){ }

	eg_int GetWidth()const{ return (right-left); }
	eg_int GetHeight()const{ return (bottom-top); }

	void OffsetThis( eg_int OffsetX , eg_int OffsetY ) { left += OffsetX; right += OffsetX; top += OffsetY; bottom += OffsetY; }
	void InflateThis( eg_int InflateX , eg_int InflateY ) { left -= InflateX; right += 2*InflateX; top -= InflateY; bottom += 2*InflateY; }
};

struct eg_rectr
{
	eg_real left, top, right, bottom;

	eg_rectr() = default;
	eg_rectr( eg_real InLeft , eg_real InTop , eg_real InRight , eg_real InBottom )	: left( InLeft ) , top( InTop ) , right( InRight ) , bottom( InBottom ){ }

	eg_real GetWidth()const{ return (right-left); }
	eg_real GetHeight()const{ return (bottom-top); }
};

#include "EGMath_Types.h"
#include "EGVector.h"


/******************
*** Video Types ***
******************/

typedef eg_uint16 egv_index; //Index.

#include "EGVertexTypes.h"

struct egScreenViewport
{
	eg_real Left;
	eg_real Right;
	eg_real Bottom;
	eg_real Top;
	eg_real Near;
	eg_real Far;

	eg_real GetAspectRatio()const
	{
		return (Right-Left)/(Top-Bottom);
	}
};

enum class eg_text_align
{
	LEFT,
	CENTER,
	RIGHT,
};

//Keeping track of loaded states is pretty common in a lot of game assets.
enum LOAD_S
{
	LOAD_NOT_LOADED,
	LOAD_LOADING,
	LOAD_LOADED,
	LOAD_LOADED_WITH_ERROR,
};

enum class eg_menuinput_t
{
	BUTTON_PRIMARY, //A
	BUTTON_BACK,    //B
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	NEXT_PAGE,
	PREV_PAGE,
	NEXT_SUBPAGE,
	PREV_SUBPAGE,
	SCROLL_UP, //Mouse only
	SCROLL_DOWN, //Mouse only
};

enum class eg_menumouse_e
{
	POSITION,
	MOVE,
	PRESSED,
	RELEASED,
};

enum class eg_camera_t
{
	ORTHO,
	PERSP,
};

enum class eg_anchor_t
{
	CENTER,
	// X
	RIGHT,
	LEFT,
	// Y
	TOP,
	BOTTOM,
};

enum class eg_loc_lang : eg_uint32
{
	UNK,
	ENUS,
	ESES,
	FRFR,
	DEDE,
};

//The following is the enum of the collisions groups,
//there can be up to 32, but that many are not necessary.
//Technically nothing should be assigned to group zero, so
//if anything is assigned to that group the program should be
//debugged. Group 4 is the trigger. (Triggers are not implemented yet).
enum class eg_phys_group : eg_uint8
{
	Unknown       ,
	Entity        , //Entities.
	ControlEntity , //An actor that is meant to control an entity (restrict y-rotation etc) (lower dominance, since it get's pushed around)
	Map           , //Map geometry.
	WorldBounds   , //Planes that bound the world.
	Trigger       , //Triggers.
	TempIgnore    , //A group to temporarily ignore.
};

struct eg_anchor
{
	eg_anchor_t X;
	eg_anchor_t Y;

	eg_anchor() = default;
	eg_anchor( eg_anchor_t InX , eg_anchor_t InY ): X(InX) , Y(InY){ }
	eg_anchor( eg_ctor_t Ct )
	{
		if( Ct == CT_Default || Ct == CT_Clear )
		{
			X = eg_anchor_t::CENTER;
			Y = eg_anchor_t::CENTER;
		}
	}

	eg_bool operator==( const eg_anchor& rhs ) const
	{
		return X == rhs.X && Y == rhs.Y;
	}

	eg_bool operator!=( const eg_anchor& rhs ) const
	{
		return !(*this == rhs);
	}
};

struct egInvalidId
{
	eg_uint DummyVar;
};

static const egInvalidId INVALID_ID = { 0 };

#define DECLARE_ID_STRUCT( _T_ ) \
struct _T_ \
{ \
	eg_uint Id; \
	\
	_T_() = default; \
	_T_( const egInvalidId& ):Id(0){ } \
	eg_bool operator==( const _T_& rhs )const{ return Id == rhs.Id; } \
	eg_bool operator!=( const _T_& rhs )const{ return !(Id==rhs.Id); } \
	const _T_& operator=( const egInvalidId& ){ Id = 0; return *this; }\
	static eg_uint IdToIndex( const _T_& Id ){ return Id.Id-1; } \
	static _T_ IndexToId( eg_uint Index ){ _T_ Out(INVALID_ID); Out.Id = Index+1; return Out; } \
	static eg_uint IdToRaw( const _T_& Id ){ return Id.Id; } \
	static _T_ RawToId( eg_uint Index ){ _T_ Out(INVALID_ID); Out.Id = Index; return Out; } \
	friend eg_bool operator==( const _T_& lhs , const egInvalidId&  ){ return lhs.Id == 0; } \
	friend eg_bool operator!=( const _T_& lhs , const egInvalidId& ){ return lhs.Id != 0; } \
	friend eg_bool operator==( const egInvalidId& , const _T_& rhs ){ return rhs.Id == 0; } \
	friend eg_bool operator!=( const egInvalidId& , const _T_& rhs ){ return rhs.Id != 0; } \
	friend eg_bool operator<( const _T_& lhs , const _T_& rhs ) { return lhs.Id < rhs.Id; } \
}; \

DECLARE_ID_STRUCT( eg_ent_id );
DECLARE_ID_STRUCT( eg_lockstep_id );