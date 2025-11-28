/******************************************************************************
File: EGCommon.h
Purpose: Header file that is included in every single game file. So this stuff
is always available without explicitly included in everything.

CL Compiler option /FI"EGCommon.h"

(c) 2011 Beem Software
******************************************************************************/
#pragma once

//Enable some warnings:
#pragma warning(3:4100 4062 4505)

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#define _HAS_EXCEPTIONS 0

//#define __EGUNICODE__

/****************
*** Assertion ***
****************/
#if defined(__DEBUG__)

	//We have to use a macro so that the break actually goes to the line of code
	//where the break occurred and not this file.
	#if defined(__WIN32__) || defined( __WIN64__ )

	#define assert(b) { if(!(b)){ __debugbreak(); } }

	#else
	#error No assert set for this platform.
	#endif

#else

	#define assert(b)

#endif

#define assert_pointer(b) assert(nullptr != b)

#define countof(b) (sizeof(b)/sizeof(0[(b)]))
#define enum_count( _enum_ ) static_cast<eg_size_t>(_enum_::COUNT)
#define enum_index( _value_ ) static_cast<eg_size_t>(_value_)

template<class T> static void unused( T& ){ }
template<class T, class U> static void unused( T& , U& ){ }
template<class T, class U, class V> static void unused( T& , U& , V& ){ }
template<class T, class U, class V, class W> static void unused( T& , U& , V& , W& ){ }
template<class T, class U, class V, class W, class Z> static void unused( T& , U& , V& , W& , Z& ){ }

template<class To,class From> To EG_To( const From& In ) { return static_cast<To>(In); }

#define assert_aligned(p) assert((reinterpret_cast<eg_size_t>((void*)(p))%EG_ALIGNMENT) == 0)
#define assert_aligned_to(p,a) assert((reinterpret_cast<eg_size_t>((void*)(p))%a) == 0)

#define EGMACROSTRING2(x) #x
#define EGMACROSTRING(x) EGMACROSTRING2(x)

#define __LINESTRING__ EGMACROSTRING(__LINE__)

#define __NMSG__ __FILE__"("__LINESTRING__"): Notice: "
#define __WMSG__ __FILE__"("__LINESTRING__"): Warning: "
#define __EMSG__ __FILE__"("__LINESTRING__"): Error: "

#define __NOTICE__(m) message(__NMSG__##m)
#define __WARNING__(m) message(__WMSG__##m)
#define __ERROR__(m) message(__EMSG__##m)

#define EG_DECL_SUPER( _superclass_ ) private: typedef _superclass_ Super;

#include "EGPrimitiveTypes.h"
#include "EGTypes.h"
#if defined( __DEBUG__ )
static const eg_bool IS_RELEASE = false;
static const eg_bool IS_DEBUG = true;
#elif defined( __RELEASE__ )
static const eg_bool IS_RELEASE = true;
static const eg_bool IS_DEBUG = false;
#endif

#include "EGStringLibrary.h"
#include "EGStackString.h"
#include "EGString.h"
#include "EGStringConvert.h"
#include "EGFunction.h"
#include "EGMath.h"
#include "EGShapes.h"
#include "EGStringCrc.h"
#include "EGMemPool.h"
#include "EGMemNew.h"
#include "EGLog.h"
#include "EGArray.h"
#include "EGMem2.h"
#include "EGMutex.h"
#include "EGObject.h"
#include "EGDynamicString.h"
#include "EGStringFormat.h"
#include "EGReflectionCore.h"
#include "EGStringEx.h"

struct eg_parm
{
	eg_parm() = delete;

	eg_parm( eg_ctor_t Ct )
	{
		if( CT_Clear == Ct || CT_Default == Ct )
		{
			Uint = 0;
		}
		else if( CT_Preserve == Ct )
		{

		}
	}

	eg_parm( eg_real R ){ Real = R; }
	eg_parm( eg_uint I ){ Uint = I; }
	eg_parm( eg_int I ){ Int = I; }
	eg_parm( eg_string_crc C ){ Crc = C.ToUint32(); }
	eg_parm( eg_bool B ){ Bool = B; }

	eg_real as_real()const{ return Real; }
	eg_uint as_uint()const{ return Uint; }
	eg_int as_int()const{ return Int; }
	eg_string_crc as_crc()const{ return eg_string_crc(Crc); }
	eg_bool as_bool()const{ return Bool; }

private:
	union
	{
		eg_real   Real;
		eg_uint   Uint;
		eg_int    Int;
		eg_uint32 Crc;
		eg_bool   Bool;
	};
};
