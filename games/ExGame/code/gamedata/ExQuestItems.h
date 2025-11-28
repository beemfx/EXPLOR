// (c) 2016 Beem Media

#pragma once

#include "EGTextFormat.h"
#include "EGDataAsset.h"
#include "EGAssetPath.h"
#include "EGEngineConfig.h"
#include "ExQuestItems.reflection.h"

egreflect struct exQuestItemInfo : public IEGCustomFormatHandler
{
	egprop eg_string_crc Id = CT_Clear;
	egprop eg_string_crc Name = CT_Clear;
	egprop eg_d_string   Name_enus = CT_Clear;
	egprop eg_string_crc Desc = CT_Clear;
	egprop eg_d_string   Desc_enus = CT_Clear;
	egprop eg_string_crc HasVarId = CT_Clear;
	egprop eg_asset_path IconPath = EXT_TEX;

	exQuestItemInfo() = default;
	exQuestItemInfo( eg_ctor_t Ct ): Id( Ct ) , Name( Ct ) , HasVarId( Ct ) { }

	virtual void FormatText( eg_cpstr Flags , EGTextParmFormatter* Formatter ) const override final;
};

egreflect class ExQuestItems : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExQuestItems , EGDataAsset )
	EG_FRIEND_RFL( ExQuestItems )

private:

	egprop EGArray<exQuestItemInfo> m_QuestItems;

	static ExQuestItems* s_Inst;

public:

	static void Init( eg_cpstr Filename );
	static void Deinit();
	static ExQuestItems& Get() { return *s_Inst; }

public:

	void GetOwnedItems( const class ExGame* Game , EGArray<exQuestItemInfo>& Out ) const;  // Works on server and client (as long as var data is replicated)
	void SetHasQuestItem( class ExGame* Game , eg_string_crc ItemId , eg_bool bHas ) const;
	void UnlockAllQuestItems( class ExGame* Game ) const;
	exQuestItemInfo GetItemInfo( eg_string_crc ItemId ) const;
};