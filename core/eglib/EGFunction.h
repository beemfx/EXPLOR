/******************************************************************************
File: Function.h
Purpose: Header for basic game helper functions and macros.

(c) 2011 Beem Software
******************************************************************************/
#pragma once

/************************************
*** Safe memory freeing funcions. ***
************************************/
template<class T> static EG_INLINE void EG_SafeDelete(T*& pObj)
{
	if(pObj)
		delete pObj;
		
	pObj=nullptr;
}

template<class T> static EG_INLINE void EG_SafeDeleteArray(T*& pObj)
{
	if(pObj)
		delete [] pObj;
		
	pObj=nullptr;
}

/*********************************
*** Safe COM release function. ***
*********************************/
template<class T> static EG_INLINE eg_int EG_SafeRelease(T*& pObj)
{
	eg_uint nRef = 0;
	if(pObj != nullptr)
	{
		nRef=pObj->Release();
		pObj=nullptr;
	}
	return nRef;
}

template<class T> static EG_INLINE void EG_SafeNxRelease(T*& pObj)
{
	if(pObj != nullptr)
	{
		pObj->release();
		pObj=nullptr;
	}
}

static EG_INLINE eg_uintptr_t EG_AlignUp(eg_uintptr_t In, eg_size_t Alignment=EG_ALIGNMENT)
{
	eg_uintptr_t Out = In;
	if(Alignment == EG_ALIGNMENT)
	{
		assert(16 == EG_ALIGNMENT); //This mask is for 16 bit alignment only.
		Out = ((In + EG_ALIGNMENT-1) & (~0xF));
	}
	else
	{
		//This is just kind of a crappy solution, but this doesn't happen often
		//anyway.
		for(eg_size_t i=0; i<Alignment; i++)
		{
			if(0 == (In+i)%Alignment)
			{
				Out = In+i;
				break;
			}
		}
	}
	//Double check to make sure that Out is aligned, and that
	//it wasn't too high.
	assert(0 == (Out%Alignment));
	assert(In <= Out && Out < (In+Alignment));
	return Out;
}

static inline void* EG_AlignUpPtr( const void* Ptr , eg_size_t Alignment = EG_ALIGNMENT )
{
	return reinterpret_cast<void*>(EG_AlignUp( reinterpret_cast<eg_uintptr_t>(Ptr) , Alignment ));
}

void EGVertex_ComputeTangents(egv_vert_mesh* aV, const egv_index* aI, eg_uint NumVerts, eg_uint NumTris);
eg_uint EGVertex_FromXmlTag(egv_vert_mesh* pV, eg_bool bBase64,  const class EGXmlAttrGetter& Getter , eg_bool *OutFoundTan = nullptr);
eg_string_big EGVertex_ToXmlTag( const egv_vert_mesh& Vert , eg_uint Id , eg_bool Base64 );

struct egTextNodeTagInfo
{
	eg_string Id;
	eg_string Font;
	eg_string_big LocText;
	eg_string_big LocTextEnus;
	eg_string TextContext;
	eg_real NodeWidth;
	eg_real NodeHeight;
	eg_real LineHeight;
	eg_color Color;
	eg_uint  Bone;
	eg_text_align Justify;
	eg_color DropShadowColor;
	eg_vec2  DropShadowOffset;
	eg_bool  bShadowIsBorder;
};

eg_string EGTextNodeTag_JustifyToString( eg_text_align Justify );
eg_text_align EGTextNodeTag_StringToJustify( eg_cpstr Str );
egTextNodeTagInfo EGTextNodeTag_FromXmlTag( const class EGXmlAttrGetter& Getter );
eg_string_big EGTextNodeTag_ToXmlTag( const egTextNodeTagInfo& Info , const eg_transform& Pose , eg_cpstr ExtraTags );

#if defined( __DEBUG__ )
#define EGFUNC_INLINE
#else
#define EGFUNC_INLINE static inline
#endif

EGFUNC_INLINE void EGMem_Copy( void* Dest , const void* Src , eg_size_t Size );
EGFUNC_INLINE eg_bool EGMem_Equals( const void* Left , const void* Right , eg_size_t Size );
EGFUNC_INLINE void EGMem_Set( void* Dest , eg_int Value , eg_size_t Size );
EGFUNC_INLINE eg_bool EGMem_Contains( const void* Mem , eg_size_t MemSize , const void* Question , eg_size_t QuestionSize );

#if !defined( __DEBUG__ )
#include "EGFunction.inl"
#endif

template<class T> static EG_INLINE void zero( T Obj )
{
	EGMem_Set( Obj , 0 , sizeof(*Obj) );
}

void EG_ToolsAssert( bool Success , const char* Function , const char* File , unsigned int Line );