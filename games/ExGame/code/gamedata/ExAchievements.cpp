// (c) 2020 Beem Media. All Rights Reserved.

#include "ExAchievements.h"
#include "EGPlatform.h"

EG_CLASS_DECL( ExAchievements )

ExAchievements* ExAchievements::s_Inst = nullptr;

void ExAchievements::Init( eg_cpstr Filename )
{
	assert( s_Inst == nullptr );
	s_Inst = EGDataAsset::LoadDataAsset<ExAchievements>( EGString_ToWide(Filename) );
	assert( nullptr != s_Inst ); // Will crash.
	Get();
}

void ExAchievements::Deinit()
{
	assert( nullptr != s_Inst );
	if( s_Inst )
	{
		EGDeleteObject( s_Inst );
		s_Inst = nullptr;
	}
}

void ExAchievements::Update( eg_real DeltaTime )
{
	unused( DeltaTime );
	
	EGArray<eg_string_crc> ToUnlock;
	m_DataLock.Lock();
	ToUnlock = std::move( m_UnlockQueue );
	m_UnlockQueue.Clear();
	m_DataLock.Unlock();

	for( const eg_string_crc& Id : ToUnlock )
	{
		if( !m_UnlockedAchievments.Contains( Id ) )
		{
			exAchievementInfo Info = GetAchievmentFromCrc( Id );
			if( Info.Id.Len() > 0 )
			{
				EGPlatform_GetPlatform().UnlockAchievement( *Info.Id );
			}
			else
			{
				assert( false ); // Bad achievement id make sure it is in database.
			}

			m_UnlockedAchievments.Append( Id );
		}
	}
}

void ExAchievements::UnlockAchievement( eg_string_crc Id )
{
	EGFunctionLock FnLock( &m_DataLock );
	
	m_UnlockQueue.Append( Id );
}

void ExAchievements::ResetAchievements()
{
	for( const exAchievementInfo& Data : m_Data )
	{
		EGPlatform_GetPlatform().ResetAchievement( *Data.Id );
	}

	m_UnlockedAchievments.Clear();
}

exAchievementInfo ExAchievements::GetAchievmentFromCrc( eg_string_crc Id ) const
{
	for( const exAchievementInfo& Data : m_Data )
	{
		if( Id == eg_string_crc(*Data.Id) )
		{
			return Data;
		}
	}
	return exAchievementInfo();
}
