// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExGameTypes.h"
#include "EGDataAsset.h"
#include "ExGlobalText.reflection.h"

egreflect class ExGlobalText : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExGlobalText , EGDataAsset )
	EG_FRIEND_RFL( ExGlobalText )

private:

	egprop EGArray<exLocTextMultiline> GlobalTexts;
};
