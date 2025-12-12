// (c) 2020 Beem Media. All Rights Reserved.

#include "ExLoadingHints.h"
#include "EGRandom.h"

EG_CLASS_DECL( ExLoadingHints )

ExLoadingHints* ExLoadingHints::s_Inst = nullptr;

void ExLoadingHints::Init( eg_cpstr Filename )
{
	assert( nullptr == s_Inst );
	s_Inst = EGDataAsset::LoadDataAsset<ExLoadingHints>( EGString_ToWide(Filename) );
	assert( nullptr != s_Inst ); // Will crash
	Get();
}

void ExLoadingHints::Deinit()
{
	EGDeleteObject( s_Inst );
	s_Inst = nullptr;
}

exLoadingHintInfo ExLoadingHints::GetRandomHint( EGRandom& Rand ) const
{
	exLoadingHintInfo Out;

	// We'll use a strategy to show hints at random but don't show a hint again until
	// all hints have been shown.

	if( m_UnviewedHints.IsEmpty() )
	{
		for( const exLoadingHintInfo& HintInfo : m_Hints )
		{
			m_UnviewedHints.Append( &HintInfo );
		}
	}

	if( m_UnviewedHints.HasItems() )
	{
		const eg_int RandomHintIdx = Rand.GetRandomRangeI( 0 , m_UnviewedHints.LenAs<eg_int>()-1 );
		if( m_Hints.IsValidIndex( RandomHintIdx ) && m_UnviewedHints[RandomHintIdx] )
		{
			Out = *m_UnviewedHints[RandomHintIdx];
			m_UnviewedHints.DeleteByIndex( RandomHintIdx );
		}
	}

	return Out;
}
