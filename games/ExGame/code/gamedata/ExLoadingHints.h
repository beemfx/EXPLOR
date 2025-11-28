// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "EGDataAsset.h"

class EGRandom;

egreflect struct exLoadingHintInfo
{
	egprop eg_string_crc HintText = CT_Clear;
	egprop eg_d_string_ml HintText_enus = CT_Clear;
};

egreflect class ExLoadingHints : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExLoadingHints , EGDataAsset )
	EG_FRIEND_RFL( ExLoadingHints )

private:

	static ExLoadingHints* s_Inst;

	egprop EGArray<exLoadingHintInfo> m_Hints;

	mutable EGArray<const exLoadingHintInfo*> m_UnviewedHints;

public:

	static ExLoadingHints& Get() { return *s_Inst; }

	static void Init( eg_cpstr Filename );
	static void Deinit();

	exLoadingHintInfo GetRandomHint( EGRandom& Rand ) const;
	eg_bool IsReady() const { return s_Inst && m_Hints.Len() > 0; }
};

