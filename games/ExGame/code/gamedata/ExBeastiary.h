// (c) 2017 Beem Media

#pragma once

#include "EGDataAsset.h"
#include "ExCore.h"
#include "EGAssetPath.h"
#include "EGEngineConfig.h"
#include "ExBeastiary.reflection.h"

egreflect struct exBeastSkillInfo
{
	egprop eg_string_crc SkillId = CT_Clear;
	egprop eg_real ChanceToUse = 10.f;
};

egreflect struct exBeastBehaviorData
{
	egprop eg_int BaseXPReward     = 1;
	egprop eg_int BaseGoldDropMin  = 0;
	egprop eg_int BaseGoldDropMax  = 0;
	egprop exAttrBaseSet Attributes       = CT_Default;
	egprop ex_target_strategy TargetStrategy = ex_target_strategy::Agression;
	egprop eg_bool bHasRangedAttack = false;
	egprop EGArray<exAttrModifier> CombatModifiers;
	egprop EGArray<exBeastSkillInfo> Skills;
};

egreflect struct exBeastInfo
{
	egprop eg_string_crc Id               = CT_Clear;
	egprop eg_string_crc Name             = CT_Clear;
	egprop eg_d_string   Name_enus        = CT_Clear;
	egprop ex_class_t    Class            = ex_class_t::Unknown;
	egprop eg_asset_path ImagePath        = EXT_TEX;
	egprop eg_bool       bIsBossCreature  = false;
	egprop exBeastBehaviorData BehaviorData;

	exBeastInfo() = default;
	exBeastInfo( eg_ctor_t Ct );

	void ResetAttributesForClass();

	ex_xp_value GetXPReward( ex_attr_value Level ) const;
	eg_int GetGoldDropAmount( ex_attr_value Level , class EGRandom& Rng ) const;
	eg_ivec2 GetGoldDropRange( ex_attr_value Level ) const;
};

egreflect struct exBeastiaryCategory
{
	egprop eg_string_crc CategoryId = CT_Clear;
	egprop EGArray<exBeastInfo> Beasts;
};

struct exRandomBeastParms
{

};

egreflect class ExBeastiary : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExBeastiary , EGDataAsset )
	EG_FRIEND_RFL( ExBeastiary )

private:

	egprop EGArray<exBeastiaryCategory> m_Categories;
	egprop EGArray<eg_string_crc> m_DebugMenuSortOrder;

	EGArray<const exBeastInfo*> m_Beasts;
	EGArray<eg_string_crc> m_DebugMenuFinalSortOrder;

	exBeastInfo m_DummyInfo = CT_Default;
	static ExBeastiary* s_Inst;

public:

	static void Init( eg_cpstr Filename );
	static void Deinit();
	static ExBeastiary& Get(){ return *s_Inst; }

	const exBeastInfo& GetRandomBeast( class EGRandom& Rng , const exRandomBeastParms& Parms ) const;
	const exBeastInfo& FindInfo( eg_string_crc BeastId ) const;
	eg_bool Contains( eg_string_crc BeastId ) const;
	eg_int GetNumBeasts() const { return m_Beasts.LenAs<eg_int>(); }
	const exBeastInfo& GetBeastByIndex( eg_int BeastIndex ) const;
	const EGArray<eg_string_crc>& GetDebugMenuSortOrder() const { return m_DebugMenuFinalSortOrder; }

	virtual void OnPropChanged( const egRflEditor& ChangedProperty , egRflEditor& RootEditor , eg_bool& bNeedsRebuildOut ) override;
	virtual void PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor ) override;

private:

	void ResetPropertiesForClass( const egRflEditor& ChangedProperty );
	void RefreshVisibleProperties( egRflEditor& RootEditor );
};