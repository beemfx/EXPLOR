///////////////////////////////////////////////////////////////////////////////
// InputButtons 
// (c) 2015 Beem Software
///////////////////////////////////////////////////////////////////////////////
#include "EGInputButtons.h"

static const struct
{
	eg_kbm_btn          Button;
	eg_string_crc    CrcId;
	eg_cpstr         String;
}
InputButtons_KbmStrTable[] =
{
	#define GAMEPAD_BUTTON( _name_ )
	#define KBM_BUTTON( _name_ )  { KBM_BTN_##_name_ , eg_crc("KBM_"#_name_) , "KBM_"#_name_ },
	#include "EGInputButtons.items"
	#undef GAMEPAD_BUTTON
	#undef KBM_BUTTON

	{ KBM_BTN_COUNT , eg_crc("") , "" },
};

static const struct
{
	eg_gp_btn        Button;
	eg_string_crc CrcId;
	eg_cpstr      String;
}
InputButtons_GpStrTable[] =
{
	#define GAMEPAD_BUTTON( _name_ ) { GP_BTN_##_name_ , eg_crc("GP_"#_name_) , "GP_"#_name_ },
	#define KBM_BUTTON( _name_ ) 
	#include "EGInputButtons.items"
	#undef GAMEPAD_BUTTON
	#undef KBM_BUTTON

	{ GP_BTN_COUNT , eg_crc("") , "" },
};

eg_kbm_btn InputButtons_StringToKbmButton( eg_cpstr Str )
{
	eg_string_crc CrcStr = eg_string_crc(Str);
	for( eg_uint i=0; i<countof(InputButtons_KbmStrTable); i++ )
	{
		if( InputButtons_KbmStrTable[i].CrcId == CrcStr )
		{
			return InputButtons_KbmStrTable[i].Button;
		}
	}
	assert( CrcStr == eg_crc("") ); //Not a button string.
	return KBM_BTN_COUNT;
}

eg_gp_btn InputButtons_StringToGpButton( eg_cpstr Str )
{
	eg_string_crc CrcStr = eg_string_crc(Str);
	for( eg_uint i=0; i<countof(InputButtons_GpStrTable); i++ )
	{
		if( InputButtons_GpStrTable[i].CrcId == CrcStr )
		{
			return InputButtons_GpStrTable[i].Button;
		}
	}
	assert( CrcStr == eg_crc("") ); //Not a button string.
	return GP_BTN_NONE;
}

eg_cpstr InputButtons_KbmButtonToString( eg_kbm_btn Button )
{
	if( KBM_BTN_COUNT == Button ) return "";
	eg_uint Index = Button;
	assert( InputButtons_KbmStrTable[Index].Button == Button );
	return InputButtons_KbmStrTable[Index].String;
}

eg_cpstr InputButtons_GpButtonToString( eg_gp_btn Button )
{
	if( GP_BTN_COUNT == Button ) return "";
	eg_uint Index = Button;
	assert( InputButtons_GpStrTable[Index].Button == Button );
	return InputButtons_GpStrTable[Index].String;
}

