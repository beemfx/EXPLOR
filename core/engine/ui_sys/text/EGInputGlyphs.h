#pragma once

void EGInputGlyphs_SetClientForInputGlyphs( class EGClient* Client );
eg_string_crc EGInputGlyphs_InputActionToKey( eg_string_crc InputActionCrc );
eg_d_string EGInputGlyphs_InputActionToKeyStr( eg_string_crc InputActionCrc );
eg_bool EGInputGlyphs_IsGamepad();