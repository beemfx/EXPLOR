// (c) 2017 Beem Media

#include "ExBeastiary.h"
#include "EGCrcDb.h"
#include "EGRandom.h"
#include "EGEngineConfig.h"
#include "EGRandom.h"

EG_CLASS_DECL( ExBeastiary )

ExBeastiary* ExBeastiary::s_Inst = nullptr;

exBeastInfo::exBeastInfo( eg_ctor_t Ct ) 
: exBeastInfo()
{
	if( CT_Default == Ct )
	{
		Id = eg_crc( "MissingMonster" );
		Name = eg_loc("MissingMonsterName","Missing Monster");
		Name_enus = "Missing Monster";
		Class = ex_class_t::EnemyMonster;
		BehaviorData.BaseXPReward = 1;
		BehaviorData.BaseGoldDropMin = 0;
		BehaviorData.BaseGoldDropMax = 0;

		ImagePath = GAME_DATA_PATH "objects/monsters/MissingMonster";

		ResetAttributesForClass();
	}
}

void exBeastInfo::ResetAttributesForClass()
{
	exClassDefaults Defaults( Class );
	#define BASE_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) BehaviorData.Attributes.##_var_ = Defaults.Def##_var_;
	#include "ExAttrs.items"
}

ex_xp_value exBeastInfo::GetXPReward( ex_attr_value Level ) const
{
	return ExCore_GetRampedValue_Level( BehaviorData.BaseXPReward , Level , EX_XP_GROWTH );
}

eg_int exBeastInfo::GetGoldDropAmount( ex_attr_value Level, class EGRandom& Rng ) const
{
	const eg_ivec2 GoldDropRange = GetGoldDropRange( Level );
	const eg_int Reward = Rng.GetRandomRangeI( GoldDropRange.x , GoldDropRange.y );
	return Reward;
}

eg_ivec2 exBeastInfo::GetGoldDropRange( ex_attr_value Level ) const
{
	eg_ivec2 Out = CT_Clear;
	Out.x = BehaviorData.BaseGoldDropMin > 0 ? ExCore_GetRampedValue_Level( BehaviorData.BaseGoldDropMin , Level , EX_GOLD_GROWTH ) : 0;
	Out.y = BehaviorData.BaseGoldDropMax > 0 ? ExCore_GetRampedValue_Level( BehaviorData.BaseGoldDropMax , Level , EX_GOLD_GROWTH ) : 0;
	return Out;
}

void ExBeastiary::Init( eg_cpstr Filename )
{
	assert( nullptr == s_Inst );
	s_Inst = EGDataAsset::LoadDataAsset<ExBeastiary>( EGString_ToWide(Filename) );
	assert( s_Inst != nullptr ); // Will crash
	Get();
}

void ExBeastiary::Deinit()
{
	assert( s_Inst != nullptr );
	if( s_Inst )
	{
		EGDeleteObject( s_Inst );
	}
	s_Inst = nullptr;
}

const exBeastInfo& ExBeastiary::GetRandomBeast( class EGRandom& Rng, const exRandomBeastParms& Parms ) const
{
	unused( Parms );

	if( m_Beasts.Len() > 0 )
	{
		eg_int Index = Rng.GetRandomRangeI( 0 , static_cast<eg_int>(m_Beasts.Len())-1 );
		if( m_Beasts.IsValidIndex( Index ) && m_Beasts[Index] )
		{
			return *m_Beasts[Index];
		}
		else
		{
			assert( false ); // Rng broken?
		}
	}

	return m_DummyInfo;
}

const exBeastInfo& ExBeastiary::FindInfo( eg_string_crc BeastId ) const
{
	for( const exBeastInfo* BeastInfo : m_Beasts )
	{
		if( BeastInfo && BeastInfo->Id == BeastId )
		{
			return *BeastInfo;
		}
	}
	return m_DummyInfo;
}

eg_bool ExBeastiary::Contains( eg_string_crc BeastId ) const
{
	for( const exBeastInfo* BeastInfo : m_Beasts )
	{
		if( BeastInfo && BeastInfo->Id == BeastId )
		{
			return true;
		}
	}
	return false;
}

const exBeastInfo& ExBeastiary::GetBeastByIndex( eg_int BeastIndex ) const
{
	if( m_Beasts.IsValidIndex( BeastIndex ) && m_Beasts[BeastIndex] )
	{
		return *m_Beasts[BeastIndex];
	}
	return m_DummyInfo;
}

void ExBeastiary::OnPropChanged( const egRflEditor& ChangedProperty , egRflEditor& RootEditor , eg_bool& bNeedsRebuildOut )
{
	Super::OnPropChanged( ChangedProperty , RootEditor , bNeedsRebuildOut );

	if( EGString_Equals( ChangedProperty.GetVarName() , "Class" ) )
	{
		bNeedsRebuildOut = true;
		ResetPropertiesForClass( ChangedProperty );
	}

	if( EGString_Equals( ChangedProperty.GetVarName() , "m_Beasts" ) )
	{
		bNeedsRebuildOut = true;
		RefreshVisibleProperties( RootEditor );
	}
}

void ExBeastiary::PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor )
{
	Super::PostLoad( Filename , bForEditor , RflEditor );

	if( bForEditor )
	{
		RefreshVisibleProperties( RflEditor );

		/*
		// Temp... Copy behavior data
		for( exBeastInfo& Beast : m_Beasts )
		{
			Beast.BehaviorData.BaseXPReward = Beast.BaseXPReward;
			Beast.BehaviorData.BaseGoldDropMin = Beast.BaseGoldDropMin;
			Beast.BehaviorData.BaseGoldDropMax = Beast.BaseGoldDropMax;
			Beast.BehaviorData.Attributes = Beast.Attributes;
			Beast.BehaviorData.TargetStrategy = Beast.TargetStrategy;
			Beast.BehaviorData.bHasRangedAttack = Beast.bHasRangedAttack;
			Beast.BehaviorData.CombatModifiers = Beast.CombatModifiers;
			Beast.BehaviorData.Skills = Beast.Skills;
		}

		RflEditor.Rebuild();
		*/
	}

	if( !bForEditor )
	{
		m_Beasts.Clear();
		m_DebugMenuFinalSortOrder = m_DebugMenuSortOrder;

		for( const exBeastiaryCategory& Category : m_Categories )
		{
			for( const exBeastInfo& Beast : Category.Beasts )
			{
				m_Beasts.Append( &Beast );
				if( !m_DebugMenuFinalSortOrder.Contains( Beast.Id ) )
				{
					m_DebugMenuFinalSortOrder.Append( Beast.Id );
				}
			}
		}
	}
}

void ExBeastiary::ResetPropertiesForClass( const egRflEditor& ChangedProperty )
{
	for( exBeastiaryCategory& Category : m_Categories )
	{
		for( exBeastInfo& Beast : Category.Beasts )
		{
			if( &Beast.Class == ChangedProperty.GetData() )
			{
				Beast.ResetAttributesForClass();
			}
		}
	}
}

void ExBeastiary::RefreshVisibleProperties( egRflEditor& RootEditor )
{
	unused( RootEditor );
}
