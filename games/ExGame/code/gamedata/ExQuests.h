// (c) 2016 Beem Media

#pragma once

#include "ExGameTypes.h"
#include "EGDataAsset.h"
#include "ExQuests.reflection.h"

class ExGame;

egreflect enum class ex_stage_status
{
	NotStarted,
	InProgress,
	Complete,
	Failed,
};

egreflect struct exQuestStageData
{
	egprop eg_int StageValue = 0;
	egprop exLocTextMultiline Description;
	egprop ex_stage_status Status = ex_stage_status::InProgress;
};

egreflect struct exQuestInfo
{
	egprop eg_string_crc  Id = CT_Clear;
	egprop exLocText QuestName;
	egprop eg_string_crc StageVar = CT_Clear;
	egprop eg_int SortPriority = 0;
	egprop EGArray<exQuestStageData> Stages;

	eg_string_crc GetStageDesc( const ExGame* Game ) const;
	eg_string_crc GetStageDesc( eg_int CurrentStageValue ) const;
	ex_stage_status GetStageStatus( const ExGame* Game ) const;
	ex_stage_status GetStageStatus( eg_int CurrentStageValue ) const;

};

egreflect class ExQuests : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExQuests , EGDataAsset )

friend struct __EGRfl_ExQuests;

private:

	static ExQuests* s_Inst;

private:

	egprop EGArray<exQuestInfo> m_Quests;

public:

	static void Init( eg_cpstr Filename );
	static void Deinit();
	static ExQuests& Get(){ return *s_Inst; }

	void GetActiveQuests( const ExGame* Game , EGArray<exQuestInfo>& Out ) const;  // Works on server and client (as long as var data is replicated)
};