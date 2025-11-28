// (c) 2017 Beem Media

#pragma once

#include "ExArmoryItem.h"
#include "EGDataAsset.h"
#include "ExArmory.reflection.h"

egreflect struct exArmoryCategory
{
	egprop eg_d_string CategoryName;
	egprop EGArray<exArmoryItem> Items;
};

egreflect class ExArmory : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExArmory , EGDataAsset )
	EG_FRIEND_RFL( ExArmory )

private:

	egprop EGArray<exArmoryCategory> m_Categories;

	static ExArmory* s_Inst;
	EGArray<exArmoryItem> m_ArmoryItems;
	exArmoryItem m_DefaultItem = CT_Default;

public:

	static ExArmory& Get(){ return *s_Inst; }

	static void Init( eg_cpstr Filename );
	static void Deinit();

	const exArmoryItem& FindInfo( eg_string_crc ItemId ) const;

private:

	virtual void PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor ) override;
};
