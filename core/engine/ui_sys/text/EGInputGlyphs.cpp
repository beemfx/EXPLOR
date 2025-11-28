#include "EGInputGlyphs.h"
#include "EGInput.h"
#include "EGClient.h"

static EGClient* EGInputGlyphs_Client = nullptr;

void EGInputGlyphs_SetClientForInputGlyphs( class EGClient* Client )
{
	EGInputGlyphs_Client = Client;
}

eg_string_crc EGInputGlyphs_InputActionToKey( eg_string_crc InputActionCrc )
{
	if( EGInputGlyphs_Client )
	{
		if( InputActionCrc == eg_crc("MENU_LEFTRIGHT") )
		{
			return EGInputGlyphs_Client->UseGamepadHints() ? eg_crc("GP_DPAD_LEFTRIGHT") : eg_crc("KBM_LEFTRIGHT");
		}
		
		return EGInput::Get().InputActionToKey( EGInputGlyphs_Client->GetInputBindings() , InputActionCrc , EGInputGlyphs_Client->UseGamepadHints() );
	}

	return InputActionCrc;
}

eg_d_string EGInputGlyphs_InputActionToKeyStr( eg_string_crc InputActionCrc )
{
	if( EGInputGlyphs_Client )
	{
		if( InputActionCrc == eg_crc("MENU_LEFTRIGHT") )
		{
			return EGInputGlyphs_Client->UseGamepadHints() ? "GP_DPAD_LEFTRIGHT" : "KBM_LEFTRIGHT";
		}

		return EGInput::Get().InputActionToKeyStr( EGInputGlyphs_Client->GetInputBindings() , InputActionCrc , EGInputGlyphs_Client->UseGamepadHints() );
	}

	return "";
}

eg_bool EGInputGlyphs_IsGamepad()
{
	eg_bool bOut = false;

	if( EGInputGlyphs_Client )
	{
		bOut = EGInputGlyphs_Client->UseGamepadHints();
	}

	return bOut;
}

