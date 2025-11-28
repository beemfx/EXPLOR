// ExPortraits
// (c) 2016 Beem Media

#include "ExPortraits.h"
#include "EGCrcDb.h"
#include "EGEngineConfig.h"

EG_CLASS_DECL( ExPortraits );

ExPortraits* ExPortraits::s_Inst = nullptr;

void ExPortraits::Init( eg_cpstr Filename )
{
	assert( nullptr == s_Inst );
	s_Inst = EGDataAsset::LoadDataAsset<ExPortraits>( EGString_ToWide(Filename) );
	assert( nullptr != s_Inst ); // Game will crash.
	Get();
}

void ExPortraits::Deinit()
{
	assert( s_Inst );
	if( s_Inst )
	{
		EGDeleteObject( s_Inst );
	}
	s_Inst = nullptr;
}

void ExPortraits::GetPlayerPortraitIds( EGArray<eg_string_crc>& Out )
{
	for( eg_size_t i=0; i<m_Portraits.Len(); i++ )
	{
		Out.Append( m_Portraits[i].Id );
	}
}

exPortraitInfo ExPortraits::FindInfo( eg_string_crc PortraitId ) const
{
	exPortraitInfo Out;

	for( eg_size_t i=0; i<m_Portraits.Len(); i++ )
	{
		if( m_Portraits[i].Id == PortraitId )
		{
			return m_Portraits[i];
		}
	}

	return exPortraitInfo();
}
