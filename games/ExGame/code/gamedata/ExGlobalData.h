// (c) 2018 Beem Media

#pragma once

#include "EGDataAsset.h"
#include "ExGameTypes.h"
#include "ExCharacterClass.h"
#include "EGAssetPath.h"
#include "ExConsts.h"
#include "ExGlobalData.reflection.h"

egreflect struct exCameraSplineData
{
	egprop eg_string_crc Id = CT_Clear;
	egprop eg_string_crc SplineId = CT_Clear;
	egprop eg_real Acceleration = .2f;
	egprop eg_real StartingSpeed = 0.f;
	egprop eg_real MaxSpeed = 5.f;
	egprop eg_bool bLooping = true;
	egprop eg_bool bJumpToStart = true;
};

egreflect struct exGlobalDataUiColors
{
	egprop eg_color32 EX_GOLD = eg_color32(255,215,0);
	egprop eg_color32 EX_INPARTY = eg_color32(0,101,0);
	egprop eg_color32 EX_ITEM = eg_color32(10,203,238);
	egprop eg_color32 EX_RARE_1 = eg_color32(255,255,255);
	egprop eg_color32 EX_RARE_2 = eg_color32(119,187,64);
	egprop eg_color32 EX_RARE_3 = eg_color32(0,86,214);
	egprop eg_color32 EX_RARE_4 = eg_color32(97,23,124);
	egprop eg_color32 EX_RARE_5 = eg_color32(255,170,0);
	egprop eg_color32 EX_CLASS = eg_color32(0,202,0);
	egprop eg_color32 EX_ATTR = eg_color32(79,162,40);// ExColor_HeaderMinor );
	egprop eg_color32 EX_ATTRV = eg_color32(255,255,255);// ExColor_HeaderMinor );
	egprop eg_color32 EX_H1 = ExColor_HeaderYellow;
	egprop eg_color32 EX_H2 = ExColor_HeaderMinor;
	egprop eg_color32 EX_HEAL = eg_color32(0,199,252);
	egprop eg_color32 EX_HURT = eg_color32(232,0,0);
	egprop eg_color32 EX_INC = eg_color32(0,199,252);
	egprop eg_color32 EX_DEC = eg_color32(232,0,0);
	egprop eg_color32 EX_WARN = ExColor_HeaderYellow;

	egprop eg_color32 EX_PDMG = eg_color32(255,255,255);
	egprop eg_color32 EX_FDMG = eg_color32(255,106,0);
	egprop eg_color32 EX_WDMG = eg_color32(0,86,214);
	egprop eg_color32 EX_EDMG = eg_color32(122,74,0);
	egprop eg_color32 EX_ADMG = eg_color32(0,162,232);
	egprop eg_color32 EX_SPELLDESC = eg_color32(150,150,150);
	egprop eg_color32 EX_CLUE = eg_color32(150,150,150);
	egprop eg_color32 EX_DISABLED_OPTION = eg_color32(150,150,150);

	egprop eg_color32 DefaultDialogHeaderColor = ExColor_HeaderYellow;
	egprop eg_color32 DefaultDialogBodyColor = eg_color32::White;
};

egreflect class ExGlobalData : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExGlobalData , EGDataAsset )
	EG_FRIEND_RFL( ExGlobalData )

private:

	egprop EGArray<eg_string_crc>  m_WorldMapMenuMaps;
	egprop exMapTravelTarget       m_StartingMapTravelTarget;
	egprop exMapTravelTarget       m_LostMapTravelTarget;
	egprop exMapTravelTarget       m_TownPortalTravelTarget;
	egprop exMapTravelTarget       m_EndGameTravelTarget;
	egprop eg_int                  m_StartingYear = 2017;
	egprop eg_int                  m_StartingDay  = 222;
	egprop eg_int                  m_StartingHour = 8;
	egprop eg_int                  m_StartingBagSize = 40;
	egprop eg_int                  m_TurnsInADay = 240; // Close to the 256 of might and magic 2 but makes day end on 00:00:00 (every turn is 6 minutes long)
	egprop eg_real                 m_RestTimePerHour = .5f;
	egprop eg_uint                 m_RestHours = 8;
	egprop eg_uint                 m_RestTilMorningTime = 6;
	egprop eg_uint                 m_MinRestHoursForRandomCombat = 4; // If resting for less than this then no encounter can occur
	egprop eg_int                  m_CharacterCreateAllocationPoints = 20;
	egprop eg_int                  m_BaseHealerHealCost = 3;
	egprop eg_int                  m_BaseHealerUnPetrifyCost = 20;
	egprop eg_int                  m_BaseHealerUnCurseCost = 20;
	egprop eg_int                  m_BaseHealerResurrectCost = 100;
	egprop eg_real                 m_CombatFleeChance = 75.f;
	egprop eg_real                 m_CombatFleeBackRowChance = 95.f;
	egprop eg_real                 m_CombatAdvanceChance = 90.f;
	egprop eg_real                 m_CombatBackAwayChance = 60.f;
	egprop eg_real                 m_CombatAggroFactor = .10f;
	egprop eg_real                 m_CombatAggroFactorWarrior = .10f;
	egprop eg_real                 m_CombatAggroFactorThief = .10f;
	egprop eg_real                 m_CombatAggroFactorCleric = .10f;
	egprop eg_real                 m_CombatAggroFactorMage = .05f;
	egprop eg_int                  m_CombatPlayerTeamFrontLineMinSize = 3;
	egprop eg_int                  m_CombatPlayerTeamFrontLineDefaultSize = 3;
	egprop eg_int                  m_CombatEnemyTeamTeamFrontLineSize = 5;
	egprop eg_int                  m_CombatThiefBurglePercent = 20;
	egprop eg_ivec2                m_CombatSpellFortifyRanksRange = eg_ivec2( 2 , 2 );
	egprop eg_real                 m_CombatPostBlurFactor = .0009f;
	egprop eg_asset_path           m_DroppedTreasureEntity = eg_asset_path_special_t::EntityDefinition;
	egprop EGArray<exRawItem>      m_DefaultInventory;
	egprop EGArray<exRawItem>      m_DefaultDebugInventory;
	egprop exCharacterClassGlobals m_CharacterClasses;
	egprop EGArray<exCameraSplineData> m_CameraSplines;
	egprop eg_string_crc           m_DefaultShopKeeperInventory;
	egprop eg_int                  m_HoursToTrain = 4;
	egprop eg_int                  m_WarriorCriticalPercent = 10;
	egprop eg_int                  m_ThiefCriticalPercent = 20;
	egprop eg_int                  m_ClericCriticalPercent = 0;
	egprop eg_int                  m_MageCriticalPercent = 0;
	egprop exGlobalDataUiColors    m_UiColors;
	egprop eg_d_string             m_SaveGamePrefix;    

	static ExGlobalData* s_Inst;

public:

	static void Init( eg_cpstr Filename );
	static void Deinit();
	static eg_bool IsInitialized() { return nullptr != s_Inst; }
	static ExGlobalData& Get(){ return *s_Inst; }

	void GetWorldMapMenuMaps( EGArray<eg_string_crc>& MapsOut ) const;
	const exMapTravelTarget& GetStartingMapTravelTarget() const { return m_StartingMapTravelTarget; }
	const exMapTravelTarget& GetLostMapTravelTarget() const { return m_LostMapTravelTarget; }
	const exMapTravelTarget& GetTownPortalTravelTarget() const { return m_TownPortalTravelTarget; }
	const exMapTravelTarget& GetEndGameTravelTarget() const { return m_EndGameTravelTarget; }
	eg_int GetStartingYear() const { return m_StartingYear; }
	eg_int GetStartingDay() const { return m_StartingDay; }
	eg_int GetStartingHour() const { return m_StartingHour; }
	eg_int GetStartingBagSize() const { return m_StartingBagSize; }
	eg_int GetTurnsInADay() const { return m_TurnsInADay; }
	eg_real GetRestTimePerHour() const { return m_RestTimePerHour; }
	eg_uint GetRestHours() const { return m_RestHours; }
	eg_uint GetRestTilMorningTime() const { return m_RestTilMorningTime; }
	eg_uint GetMinRestHoursForRandomCombat() const { return m_MinRestHoursForRandomCombat; }
	ex_attr_value GetCharacterCreateAllocationPoints() const { return m_CharacterCreateAllocationPoints; }
	const exCharacterClassGlobals& GetCharacterClassGlobals() const { return m_CharacterClasses; }
	void GetDefaultInventory( EGArray<exRawItem>& ItemsOut ) const;
	eg_real GetCombatFleeChance( eg_bool bFrontRow ) const { return bFrontRow ? m_CombatFleeChance : m_CombatFleeBackRowChance; }
	eg_real GetCombatAdvanceChance() const { return m_CombatAdvanceChance; }
	eg_real GetCombatBackAwayChance() const { return m_CombatBackAwayChance; }
	eg_real GetCombatAggroFactor( ex_class_t ClassType ) const;
	eg_int GetCombatPlayerTeamFrontLineMinSize() const { return m_CombatPlayerTeamFrontLineMinSize; }
	eg_int GetCombatPlayerTeamFrontLineDefaultSize() const { return m_CombatPlayerTeamFrontLineDefaultSize; }
	eg_int GetCombatEnemyTeamTeamFrontLineSize() const { return m_CombatEnemyTeamTeamFrontLineSize; }
	eg_int GetCombatThiefBurglePercent() const { return m_CombatThiefBurglePercent; }
	eg_real GetCombatPostBlurFactor() const { return m_CombatPostBlurFactor; }
	eg_int GetBaseHealerHealCost() const { return m_BaseHealerHealCost; }
	eg_int GetBaseHealerUnPetrifyCost() const { return m_BaseHealerUnPetrifyCost; }
	eg_int GetBaseHealerUnCurseCost() const { return m_BaseHealerUnCurseCost; }
	eg_int GetBaseHealerResurrectCost() const { return m_BaseHealerResurrectCost; }
	eg_string_crc GetDroppedTreasureEntity() const { return eg_string_crc(*m_DroppedTreasureEntity.Path); }
	const EGArray<exCameraSplineData>& GetCameraSplines() const { return m_CameraSplines; }
	const eg_ivec2& GetCombatSpellFortifyRanksRange() const { return m_CombatSpellFortifyRanksRange; }
	const eg_string_crc& GetDefaultShopKeeperInventory() const { return m_DefaultShopKeeperInventory; }
	eg_int GetHoursToTrain() const { return m_HoursToTrain; }
	eg_int GetCriticalPercent( ex_class_t ClassType ) const;
	const exGlobalDataUiColors& GetUiColors() const { return m_UiColors; }
	const eg_d_string& GetSaveGamePrefix() const { return m_SaveGamePrefix; }

protected:

	virtual void PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor ) override;
	void RefreshVisibleProperties( egRflEditor& RootEditor );
};