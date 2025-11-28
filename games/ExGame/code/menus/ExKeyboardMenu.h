// (c) 2016 Beem Media. All Rights Reserved.

#pragma once

class IExKeyboardMenuCb
{
public:
	virtual void OnTextFromKeyboardMenu( const eg_loc_char* String )=0;
};

void ExKeyboardMenu_PushMenu( class EGClient* Owner , const eg_loc_char* ValidChars , IExKeyboardMenuCb* CbInterface , eg_size_t MaxChars , const eg_loc_char* InitialString );