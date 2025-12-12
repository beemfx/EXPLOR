// (c) 2016 Beem Media

#pragma once

#include "ExCore.h"
#include "EGDataAsset.h"
#include "EGEngineConfig.h"
#include "EGAssetPath.h"
#include "ExPortraits.reflection.h"

egreflect struct exPortraitInfo
{
	egprop eg_string_crc Id       = CT_Clear;
	egprop eg_asset_path Filename = eg_asset_path( EXT_TEX , GAME_DATA_PATH "/portraits/T_Portrait_Empty_d" );
	egprop ex_race_t     Race     = ex_race_t::Unknown;
	egprop ex_gender_t   Gender   = ex_gender_t::Unknown;
};


egreflect class ExPortraits : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExPortraits , EGDataAsset )
	EG_FRIEND_RFL( ExPortraits )

private:

	egprop EGArray<exPortraitInfo> m_Portraits;

	static ExPortraits* s_Inst;

public:

	static void Init( eg_cpstr Filename );
	static void Deinit();
	static eg_bool IsInitialized() { return nullptr != s_Inst; }
	static ExPortraits& Get(){ return *s_Inst; }

	void GetPlayerPortraitIds( EGArray<eg_string_crc>& Out );
	exPortraitInfo FindInfo( eg_string_crc PortraitId ) const;
};