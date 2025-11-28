///////////////////////////////////////////////////////////////////////////////
//
// Localize Module
//
///////////////////////////////////////////////////////////////////////////////
#pragma once

void EGLocalize_Init( eg_loc_lang Language );
void EGLocalize_Deinit( void );
void EGLocalize_Localize( eg_string_crc TextCrc , class eg_loc_text& LocTextOut  );
eg_loc_text EGLocalize_Localize( eg_string_crc TextCrc );
void EGLocalize_DumpDb();

