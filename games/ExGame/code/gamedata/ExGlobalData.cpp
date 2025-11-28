// (c) 2018 Beem Media

#include "ExGlobalData.h"

EG_CLASS_DECL( ExGlobalData );

ExGlobalData* ExGlobalData::s_Inst = nullptr;

void ExGlobalData::Init( eg_cpstr Filename )
{
	assert( nullptr == s_Inst );
	s_Inst = EGDataAsset::LoadDataAsset<ExGlobalData>( EGString_ToWide(Filename) );
	assert( nullptr != s_Inst ); // Game will crash.
	Get();
}

void ExGlobalData::Deinit()
{
	assert( s_Inst );
	if( s_Inst )
	{
		EGDeleteObject( s_Inst );
	}
	s_Inst = nullptr;
}

void ExGlobalData::GetWorldMapMenuMaps( EGArray<eg_string_crc>& MapsOut ) const
{
	for( const eg_string_crc& Crc : m_WorldMapMenuMaps )
	{
		MapsOut.Append( Crc );
	}
}

void ExGlobalData::GetDefaultInventory( EGArray<exRawItem>& ItemsOut ) const
{
	ItemsOut = m_DefaultInventory;
	if( IS_DEBUG )
	{
		ItemsOut.Append( m_DefaultDebugInventory );
	}
}

eg_real ExGlobalData::GetCombatAggroFactor( ex_class_t ClassType ) const
{
	eg_real Out = m_CombatAggroFactor;

	switch( ClassType )
	{
	case ex_class_t::Unknown:
		break;
	case ex_class_t::Warrior:
		Out = m_CombatAggroFactorWarrior;
		break;
	case ex_class_t::Thief:
		Out = m_CombatAggroFactorThief;
		break;
	case ex_class_t::Mage:
		Out = m_CombatAggroFactorMage;
		break;
	case ex_class_t::Cleric:
		Out = m_CombatAggroFactorCleric;
		break;
	case ex_class_t::EnemyMonster:
		break;
	}

	return Out;
}

eg_int ExGlobalData::GetCriticalPercent( ex_class_t ClassType ) const
{
	switch( ClassType )
	{
	case ex_class_t::Unknown:
		return 0;
	case ex_class_t::Warrior:
		return m_WarriorCriticalPercent;
	case ex_class_t::Thief:
		return m_ThiefCriticalPercent;
	case ex_class_t::Mage:
		return m_MageCriticalPercent;
	case ex_class_t::Cleric:
		return m_ClericCriticalPercent;
	case ex_class_t::EnemyMonster:
		return 0;
	}
	return 0;
}

void ExGlobalData::PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor )
{
	Super::PostLoad( Filename , bForEditor , RflEditor );

	if( bForEditor )
	{
		RefreshVisibleProperties( RflEditor );
	}

	if( !bForEditor )
	{
		m_CharacterClasses.PostLoad();
	}
}

void ExGlobalData::RefreshVisibleProperties( egRflEditor& RootEditor )
{
	unused( RootEditor );
}
