// ExQuests
// (c) 2016 Beem Media

#include "ExQuests.h"
#include "EGCrcDb.h"
#include "ExGame.h"
#include "EGEngineConfig.h"

EG_CLASS_DECL( ExQuests )

ExQuests* ExQuests::s_Inst = nullptr;

void ExQuests::Init( eg_cpstr Filename )
{
	assert( nullptr == s_Inst );
	s_Inst = EGDataAsset::LoadDataAsset<ExQuests>( EGString_ToWide(Filename) );
	assert( s_Inst != nullptr ); // Will crash.
	Get();
}

void ExQuests::Deinit()
{
	assert( s_Inst != nullptr );
	if( s_Inst )
	{
		EGDeleteObject( s_Inst );
		s_Inst = nullptr;
	}
}

void ExQuests::GetActiveQuests( const ExGame* Game , EGArray<exQuestInfo>& Out ) const
{
	for( eg_size_t i=0; i<m_Quests.Len(); i++ )
	{
		const eg_int QuestStage = Game->GetGameVar( m_Quests[i].StageVar ).as_int();
		ex_stage_status Status = m_Quests[i].GetStageStatus( QuestStage );
		if( Status != ex_stage_status::NotStarted )
		{
			Out.Append( m_Quests[i] );
		}
	}

	Out.Sort( [Game]( const exQuestInfo& Left , const exQuestInfo& Right ) -> eg_bool
	{
		const ex_stage_status LeftStatus = Left.GetStageStatus( Game );
		const ex_stage_status RigthStatus = Right.GetStageStatus( Game );
		const eg_bool bLeftCompletionLo = ( LeftStatus == ex_stage_status::InProgress || LeftStatus == ex_stage_status::NotStarted );
		const eg_bool bRightCompletionLo = ( RigthStatus == ex_stage_status::InProgress || RigthStatus == ex_stage_status::NotStarted );

		if( bLeftCompletionLo != bRightCompletionLo )
		{
			return bLeftCompletionLo;
		}

		return Left.SortPriority < Right.SortPriority;
	} );
}

eg_string_crc exQuestInfo::GetStageDesc( const ExGame* Game ) const
{
	eg_string_crc Out = CT_Clear;
	if( Game )
	{
		const eg_int CurrentStage = Game->GetGameVar( StageVar ).as_int();
		return GetStageDesc( CurrentStage );
	}
	return Out;
}

eg_string_crc exQuestInfo::GetStageDesc( eg_int CurrentStageValue ) const
{
	eg_string_crc Out = CT_Clear;
	for( const exQuestStageData& Stage : Stages )
	{
		if( CurrentStageValue >= Stage.StageValue )
		{
			Out = Stage.Description.Text;
		}
	}
	return Out;
}

ex_stage_status exQuestInfo::GetStageStatus( const ExGame* Game ) const
{
	ex_stage_status Out = ex_stage_status::NotStarted;
	if( Game )
	{
		const eg_int CurrentStage = Game->GetGameVar( StageVar ).as_int();
		return GetStageStatus( CurrentStage );
	}
	return Out;
}

ex_stage_status exQuestInfo::GetStageStatus( eg_int CurrentStageValue ) const
{
	ex_stage_status Out = ex_stage_status::NotStarted;
	for( const exQuestStageData& Stage : Stages )
	{
		if( CurrentStageValue >= Stage.StageValue )
		{
			Out = Stage.Status;
		}
	}
	return Out;
}
