// (c) 2019 Beem Media

#include "ExMapInfos.h"
#include "EGRandom.h"

EG_CLASS_DECL( ExMapInfos )

ExMapInfos* ExMapInfos::s_Inst = nullptr;

ExMapInfos::ExMapInfos()
{

}

void ExMapInfos::Init( eg_cpstr Filename )
{
	assert( nullptr == s_Inst );
	s_Inst = EGDataAsset::LoadDataAsset<ExMapInfos>( EGString_ToWide(Filename) );
	assert( nullptr != s_Inst ); // Will crash
	Get();
	s_Inst->LoadMetaData();
}

void ExMapInfos::Deinit()
{
	EGDeleteObject( s_Inst );
	s_Inst = nullptr;
}

const exMapInfoData& ExMapInfos::FindInfo( eg_string_crc CrcId ) const
{
	for( const exMapInfoData& Props : m_MapInfos )
	{
		if( Props.Id == CrcId )
		{
			return Props;
		}
	}

	return m_DefaultInfo;
}

const exMapInfoData& ExMapInfos::FindInfoByWorldFilename( eg_cpstr WorldFilename ) const
{
	for( const exMapInfoData& Props : m_MapInfos )
	{
		if( Props.World.FullPath.EqualsI( WorldFilename ) )
		{
			return Props;
		}
	}

	return m_DefaultInfo;
}

void ExMapInfos::PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor )
{
	Super::PostLoad( Filename , bForEditor , RflEditor );

	if( bForEditor )
	{
		// for( exMapInfoData& Props : m_MapInfos )
		// {
		// 	Props.EncounterData.ForceThrillSeekerSquaresRevealedThreshold = EG_To<eg_int>(.75f * Props.EncounterData.ForReferenceTotalSquares);
		// }
	}
}

void ExMapInfos::LoadMetaData()
{
	for( exMapInfoData& Props : m_MapInfos )
	{
		eg_d_string16 PathWithExtension = EGString_Format( "%s.%s" , *Props.MapMetaData.FullPath , Props.MapMetaData.Ext.String() );
		EGGcxMapMetaData* MetaData = EGDataAsset::LoadDataAsset<EGGcxMapMetaData>( *PathWithExtension );
		if( MetaData )
		{
			Props.MapMetaInfo = MetaData->GetMetaInfo();
			EGDeleteObject( MetaData );
		}
	}
}

eg_real exMapInfoMonsterData::GetEncounterProbabilityPercent( eg_int LowLevel , eg_int HiLevel , ex_encounter_disposition Disposition , eg_bool bNewlyRevealed , eg_int NumSquaresRevealed ) const
{
	eg_int ThreshIdx = -1;

	// If the high level is just one level above low level use the low level as the threshold, otherwise 
	// the case is probably that high level fighters are taking low level ones into the dungeon and so
	// thrill seeker mode should be explicitely turned on.
	const eg_int ForceThrillSeekerCompareLevel = (HiLevel - LowLevel) > 1 ? HiLevel : LowLevel;

	if( ForceThrillSeekerCompareLevel < ForceThrillSeekerLevelThreshold && NumSquaresRevealed >= ForceThrillSeekerSquaresRevealedThreshold )
	{
		Disposition = ex_encounter_disposition::FirstProbabilitySet;
	}

	if( Disposition == ex_encounter_disposition::Default )
	{
		for( eg_int i=0; i<RandomEncounterThresholdPercents.LenAs<eg_int>(); i++ )
		{
			const exMapInfoThreshold& Thresh = RandomEncounterThresholdPercents[i];
		
			if( LowLevel >= Thresh.Level || 0 == i )
			{
				ThreshIdx = i;
			}
		}
	}
	else if( Disposition == ex_encounter_disposition::FirstProbabilitySet )
	{
		// We pick the first probability.
		ThreshIdx = 0;
	}

	if( RandomEncounterThresholdPercents.IsValidIndex( ThreshIdx ) )
	{
		const exMapInfoThreshold& Threshold = RandomEncounterThresholdPercents[ThreshIdx];

		eg_real FinalPct = Threshold.Value;

		if( Disposition == ex_encounter_disposition::Default && !bNewlyRevealed )
		{
			FinalPct *= RandomEncounterVisitedVertexAdjustment;
		}

		return FinalPct;
	}

	return 0.f;
}

eg_string_crc exMapInfoEncounterData::GetEncounter( EGRandom& Rng ) const
{
	if( EncounterTypes.IsEmpty() )
	{
		return CT_Clear;
	}

	auto GetRandomFromWeight = [this,&Rng]( eg_int TotalWeight ) -> eg_string_crc
	{
		const eg_int RandomValue = Rng.GetRandomRangeI( 1 , TotalWeight );
		eg_int WeightConsidered = 0;
		for( eg_uint ItemIdx = 0; ItemIdx < EncounterTypes.Len(); ItemIdx++ )
		{
			const exMapInfoEncounterItem& Info = EncounterTypes[ItemIdx];
			WeightConsidered += Info.Weight;

			if( RandomValue <= WeightConsidered )
			{
				return Info.EncounterId;
			}
		}

		assert( false ); // Calculation incorrect?
		return EncounterTypes[0].EncounterId;
	};

	auto UseCountsAsWeights = [this,&Rng,&GetRandomFromWeight]() -> eg_string_crc
	{
		eg_int NumWeights = 0;
		for( const exMapInfoEncounterItem& Info : EncounterTypes )
		{
			NumWeights += Info.Weight;
		}

		return GetRandomFromWeight( NumWeights );
	};

	return UseCountsAsWeights();
}
