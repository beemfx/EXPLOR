///////////////////////////////////////////////////////////////////////////////
// InputButtons - Helper functions for converting game buttons to strings
// and back (for saving key bindings) as well as a struct for  helping keep
// track of keyboard and mouse state.
//
// (c) 2015 Beem Software
///////////////////////////////////////////////////////////////////////////////
#pragma once

enum eg_gp_btn
{
	#define GAMEPAD_BUTTON( _name_ ) GP_BTN_##_name_ ,
	#define KBM_BUTTON( _name_ )
	#include "EGInputButtons.items"
	#undef GAMEPAD_BUTTON
	#undef KBM_BUTTON

	GP_BTN_COUNT            ,
};

enum eg_kbm_btn
{
	#define GAMEPAD_BUTTON( _name_ ) 
	#define KBM_BUTTON( _name_ ) KBM_BTN_##_name_ ,
	#include "EGInputButtons.items"
	#undef GAMEPAD_BUTTON
	#undef KBM_BUTTON
	
	KBM_BTN_COUNT           ,    /* Only to count how many keys there are */
};


eg_kbm_btn  InputButtons_StringToKbmButton( eg_cpstr Str );
eg_cpstr InputButtons_KbmButtonToString( eg_kbm_btn Button );
eg_gp_btn   InputButtons_StringToGpButton( eg_cpstr Str );
eg_cpstr InputButtons_GpButtonToString( eg_gp_btn Button );

enum INPUT_MOUSE_MODE
{
	INPUT_MOUSE_MODE_CURSOR,
	INPUT_MOUSE_MODE_DELTA,
};

struct egInputKbmData
{
	static const eg_uint KEYS_PER_FLAG = sizeof(eg_flag)*8;
	static const eg_uint ARRAY_SIZE = (KBM_BTN_COUNT/KEYS_PER_FLAG)+1;
	eg_flags Pressed[ARRAY_SIZE];
	eg_flags Released[ARRAY_SIZE];

	//Buffer for typed characters.
	static const eg_uint CBUFFER_SIZE=10;
	eg_char CharBuffer[CBUFFER_SIZE];
	eg_uint CharBufferCount;

	void OnChar( eg_char c )
	{
		if( CharBufferCount < CBUFFER_SIZE )
		{
			CharBuffer[CharBufferCount] = c;
			CharBufferCount++;
		}
		else
		{
			assert( false ); //Too many keys, increase buffer size?
		}
	}

	eg_bool IsPressed( eg_kbm_btn Key )const
	{
		eg_uint ArrayIndex = Key/KEYS_PER_FLAG;
		eg_uint FlagIndex  = Key%KEYS_PER_FLAG;
		if( ArrayIndex >= ARRAY_SIZE || FlagIndex >= KEYS_PER_FLAG )
		{
			assert( false ); //Invalid Key?
			return false;
		}

		return Pressed[ArrayIndex].IsSet(1<<FlagIndex);
	}

	eg_bool IsReleased( eg_kbm_btn Key )const
	{
		eg_uint ArrayIndex = Key/KEYS_PER_FLAG;
		eg_uint FlagIndex  = Key%KEYS_PER_FLAG;
		if( ArrayIndex >= ARRAY_SIZE || FlagIndex >= KEYS_PER_FLAG )
		{
			assert( false ); //Invalid Key?
			return false;
		}

		return Released[ArrayIndex].IsSet(1<<FlagIndex);
	}

	void SetPressed( eg_kbm_btn Key , eg_bool Set )
	{
		eg_uint ArrayIndex = Key/KEYS_PER_FLAG;
		eg_uint FlagIndex  = Key%KEYS_PER_FLAG;
		if( ArrayIndex >= ARRAY_SIZE || FlagIndex >= KEYS_PER_FLAG )
		{
			assert( false ); //Invalid Key?
			return;
		}
		return Pressed[ArrayIndex].SetState( 1<<FlagIndex , Set );
	}

	void SetReleased( eg_kbm_btn Key , eg_bool Set )
	{
		eg_uint ArrayIndex = Key/KEYS_PER_FLAG;
		eg_uint FlagIndex  = Key%KEYS_PER_FLAG;
		if( ArrayIndex >= ARRAY_SIZE || FlagIndex >= KEYS_PER_FLAG )
		{
			assert( false ); //Invalid Key?
			return;
		}
		return Released[ArrayIndex].SetState( 1<<FlagIndex , Set );
	}
};

struct egRawKbmData
{
	egInputKbmData KbData;
	eg_ivec2       RawMouseDelta;
	eg_uint32      RawMouseDeltaNumSamples;
	eg_vec2        MousePos;
	eg_bool        bHadInput;

	egRawKbmData( eg_ctor_t Ct )
	: KbData()
	, RawMouseDelta(0,0)
	, RawMouseDeltaNumSamples(0)
	, MousePos(0,0)
	, bHadInput( false )
	{
		unused( Ct );
		assert( CT_Clear == Ct );
	}
};
