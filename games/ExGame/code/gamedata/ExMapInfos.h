// (c) 2019 Beem Media

#pragma once

#include "ExGameTypes.h"
#include "EGDataAsset.h"
#include "EGAssetPath.h"
#include "EGGcxMapMetaData.h"

class EGRandom;

static const eg_vec3 ExMapInfo_DefaultDof(9*6.f,128*6.f,20*6.f);

egreflect struct exMapInfoThreshold
{
	egprop eg_int  Level = 1;
	egprop eg_real Value = 0.f;
};

egreflect struct exMapInfoEncounterItem
{
	egprop eg_string_crc EncounterId = CT_Clear;
	egprop eg_int        Weight = 10;
};

egreflect struct exMapInfoEncounterData
{
	egprop EGArray<exMapInfoEncounterItem> EncounterTypes;

	eg_string_crc GetEncounter( EGRandom& Rng ) const;
};

egreflect struct exMapInfoMonsterData
{
	egprop exMapInfoEncounterData RandomEncouter;
	egprop eg_string_crc Fixed = CT_Clear;
	egprop eg_string_crc Boss = CT_Clear;
	egprop eg_int SafeTurnsAfterCombat = 0;
	egprop EGArray<exMapInfoThreshold> RandomEncounterThresholdPercents;
	egprop eg_real RandomEncounterVisitedVertexAdjustment = .1f;
	egprop eg_real RestingEncounterPercent = 5.f;
	egprop eg_int ForceThrillSeekerLevelThreshold = 1;
	egprop eg_int ForceThrillSeekerSquaresRevealedThreshold = 100;
	egprop eg_int ForReferenceTotalSquares = 0;

	eg_real GetEncounterProbabilityPercent( eg_int LowLevel , eg_int HiLevel , ex_encounter_disposition Disposition , eg_bool bNewlyRevealed , eg_int NumSquaresRevealed ) const;
};

egreflect struct exMapInfoFogData
{
	egprop eg_int ViewDistance = 11;
	egprop eg_real ViewDistanceUnitSize = 6.f;
	egprop eg_real FogStartOffset = 8.f;
	egprop eg_real FogEndOffset = 4.f;
	egprop eg_vec3 DepthOfFieldRange = ExMapInfo_DefaultDof;
	egprop eg_real DepthOfFieldBlurFactor = .0009f;
};

egreflect struct exMapInfoData
{
	egprop eg_string_crc          Id                    = CT_Clear;
	egprop eg_asset_path          MapMetaData           = "egasset";
	egprop eg_asset_path          World                 = "egworld";
	egprop eg_string_crc          NameCrc               = CT_Clear;
	egprop eg_d_string            NameCrc_enus          = "";
	egprop eg_d_string            ScriptId              = "";
	egprop eg_asset_path          MusicTrack            = eg_asset_path_special_t::Sound;
	egprop ex_map_info_map_t      MapType               = ex_map_info_map_t::Dungeon;
	egprop eg_bool                bCanUseTownPortal     = true;
	egprop eg_string_crc          DayNightCycleId       = CT_Clear;
	egprop eg_int                 DefaultLockLevel      = 1;
	egprop exMapInfoFogData       FogData;
	egprop exMapInfoMonsterData   EncounterData;

	egGcxMapMetaInfo              MapMetaInfo;
};

egreflect class ExMapInfos : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExMapInfos , EGDataAsset )
	EG_FRIEND_RFL( ExMapInfos )

private:

	static ExMapInfos* s_Inst;

	egprop EGArray<exMapInfoData> m_MapInfos;

	exMapInfoData m_DefaultInfo;

public:

	static ExMapInfos& Get() { return *s_Inst; }

	ExMapInfos();

	static void Init( eg_cpstr Filename );
	static void Deinit();

	const exMapInfoData& FindInfo( eg_string_crc CrcId ) const;
	const exMapInfoData& FindInfoByWorldFilename( eg_cpstr WorldFilename ) const;

	virtual void PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor ) override;

private:

	void LoadMetaData();
};
