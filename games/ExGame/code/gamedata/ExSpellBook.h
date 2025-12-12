// (c) 2016 Beem Media

#pragma once

#include "ExGameTypes.h"
#include "ExSpellInfo.h"
#include "EGDataAsset.h"
#include "ExSpellbook.reflection.h"

egreflect struct exSpellBookCategory
{
	egprop eg_string_crc CategoryName = CT_Clear;
	egprop EGArray<exSpellInfo> Spells;
};


egreflect class ExSpellBook : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExSpellBook , EGDataAsset )
	EG_FRIEND_RFL( ExSpellBook )

private:

	egprop EGArray<exSpellBookCategory> m_Categories;

	ExSpellList m_AllSpells;
	exSpellInfo m_DefaultSpell;

	static ExSpellBook* s_Inst;

public:

	static void Init( eg_cpstr Filename );
	static void Deinit();
	static ExSpellBook& Get(){ return *s_Inst; }

	void GetAll( ExSpellList& AllOut ) const;
	const exSpellInfo& FindInfo( const eg_string_crc& SpellId ) const;

private:

	virtual void PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor );
	virtual void OnPropChanged( const egRflEditor& ChangedProperty , egRflEditor& RootEditor , eg_bool& bNeedsRebuildOut ) override;
	void RefreshVisibleProperties( egRflEditor& RootEditor );
};