// (c) 2021 Beem Media. All Rights Reserved.

#include "ExNpcGossip.h"
#include "EGRandom.h"

EG_CLASS_DECL( ExNpcGossip )

ExNpcGossip* ExNpcGossip::s_Inst = nullptr;

void ExNpcGossip::Init( eg_cpstr Filename )
{
	assert( s_Inst == nullptr );
	s_Inst = EGDataAsset::LoadDataAsset<ExNpcGossip>( EGString_ToWide(Filename) );
	assert( nullptr != s_Inst ); // Will crash.
	Get();
	Get().TestUniqueness();
}

void ExNpcGossip::Deinit()
{
	assert( nullptr != s_Inst );
	if( s_Inst )
	{
		EGDeleteObject( s_Inst );
		s_Inst = nullptr;
	}
}

exNpcGossipDialog ExNpcGossip::GetRandomDialogForNpc( eg_string_crc NpcId , EGRandom& Rng ) const
{
	for( const exNpcGossipForNpc& NpcGossip : m_NpcGossips )
	{
		if( NpcGossip.Id == NpcId )
		{
			return NpcGossip.GetRandomDialog( Rng );
		}
	}

	return exNpcGossipDialog();
}

void ExNpcGossip::TestUniqueness()
{
	if( EX_CHEATS_ENABLED )
	{
		EGItemMap<eg_string_crc,eg_d_string> Keys( CT_Default );

		auto AddKey = [&Keys]( eg_string_crc Id , const eg_d_string& Str ) -> void
		{
			if( Id.IsNull() )
			{
				EGLogf( eg_log_t::Warning , "%s had no id", *Str );
				assert( false );
			}
			else if( Keys.Contains( Id ) )
			{
				EGLogf( eg_log_t::Warning , "%s duplicates %s" , *Str , *Keys[Id] );
				assert( false );
			}
			else
			{
				Keys.Insert( Id , Str );
			}
		};

		for( const exNpcGossipForNpc& NpcGossip : m_NpcGossips )
		{
			for( const exNpcGossipDialog& Dialog : NpcGossip.Dialogs )
			{
				AddKey( Dialog.CharacterDialog.Text , Dialog.CharacterDialog.Text_enus );
				AddKey( Dialog.PlayerResponse.Text , Dialog.PlayerResponse.Text_enus );
			}
		}
	}
}

exNpcGossipDialog exNpcGossipForNpc::GetRandomDialog( EGRandom& Rng ) const
{
	exNpcGossipDialog Out;

	if( Dialogs.Len() > 0 )
	{
		const eg_int RandIdx = Rng.GetRandomRangeI(0,Dialogs.LenAs<eg_int>()-1);
		Out = Dialogs[RandIdx];
	}

	return Out;
}
