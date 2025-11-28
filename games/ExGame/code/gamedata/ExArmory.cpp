// (c) 2017 Beem Media

#include "ExArmory.h"
#include "ExCore.h"

ExArmory* ExArmory::s_Inst = nullptr;

EG_CLASS_DECL( ExArmory )

void ExArmory::Init( eg_cpstr Filename )
{
	assert( s_Inst == nullptr );
	s_Inst = EGDataAsset::LoadDataAsset<ExArmory>( EGString_ToWide(Filename) );
	assert( nullptr != s_Inst ); // Will crash.
	Get();
}

void ExArmory::Deinit()
{
	assert( nullptr != s_Inst );
	if( s_Inst )
	{
		EGDeleteObject( s_Inst );
		s_Inst = nullptr;
	}
}

const exArmoryItem& ExArmory::FindInfo( eg_string_crc ItemId ) const
{
	for( const exArmoryItem& Item : m_ArmoryItems )
	{
		if( Item.Id == ItemId )
		{
			return Item;
		}
	}
	return m_DefaultItem;
}

void ExArmory::PostLoad( eg_cpstr16 Filename, eg_bool bForEditor , egRflEditor& RflEditor )
{
	Super::PostLoad( Filename , bForEditor  , RflEditor );

	if( !bForEditor )
	{
		eg_size_t TotalNumItems = 0;
		for( const exArmoryCategory& Category : m_Categories )
		{
			TotalNumItems += Category.Items.Len();
		}

		m_ArmoryItems.Reserve( TotalNumItems );

		for( const exArmoryCategory& Category : m_Categories )
		{
			for( const exArmoryItem& Item : Category.Items )
			{
				m_ArmoryItems.Append( Item );
			}
		}
	}
}
