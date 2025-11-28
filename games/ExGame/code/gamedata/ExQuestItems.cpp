// ExQuestItems
// (c) 2016 Beem Media

#include "ExQuestItems.h"
#include "EGCrcDb.h"
#include "ExGame.h"
#include "EGEngineConfig.h"

EG_CLASS_DECL( ExQuestItems )

void exQuestItemInfo::FormatText( eg_cpstr Flags, EGTextParmFormatter* Formatter ) const
{
	eg_string_crc BaseFlag = Formatter->GetNextFlag( &Flags );
	switch_crc( BaseFlag )
	{
		case_crc("NAME") :
		{
			eg_string_crc NameParm = Formatter->GetNextFlag( &Flags );
			if( NameParm != eg_crc("NOCOLOR") )
			{
				Formatter->SetText( EGFormat( L"|SC(EX_ITEM)|{0}|RC()|" , Name ).GetString() );
			}
			else
			{
				Formatter->SetText( eg_loc_text( Name ).GetString() );
			}
		} break;
		case_crc("DESC"):
		{
			eg_string_crc DescFlag = Formatter->GetNextFlag( &Flags );
			eg_loc_text RawDesc = eg_loc_text(Desc);

			if( DescFlag == eg_crc("ITEM_CARD") )
			{
				eg_string_crc ItemCardParm = Formatter->GetNextFlag( &Flags );
				eg_loc_text FullDesc = EGFormat( eg_loc("QuestItemsItemCardDesc","|SC(EX_ITEM)|Quest Item|RC()|\n{0}") , RawDesc );
				Formatter->SetText( FullDesc.GetString() );
			}
			else
			{
				Formatter->SetText( RawDesc.GetString() );
			}
		} break;
		case_crc("COST"):
		{
			eg_string_crc CostParm = Formatter->GetNextFlag( &Flags );
			switch_crc(CostParm)
			{
				case_crc("SELL"):
				case_crc("BUY"):
				case_crc("BUYBACK"):
				default:
					Formatter->SetText( eg_loc_text(eg_loc("QuestItemsCostText","|SC(EX_GOLD)|Not for sale|RC()|")).GetString() );
					break;
			}
		} break;
	}
}

///////////////////////////////////////////////////////////////////////////////

ExQuestItems* ExQuestItems::s_Inst = nullptr;

void ExQuestItems::Init( eg_cpstr Filename )
{
	assert( nullptr == s_Inst );
	s_Inst = EGDataAsset::LoadDataAsset<ExQuestItems>( EGString_ToWide(Filename) );
	assert( nullptr != s_Inst ); // Game will crash.
	Get();
}

void ExQuestItems::Deinit()
{
	assert( s_Inst );
	if( s_Inst )
	{
		EGDeleteObject( s_Inst );
	}
	s_Inst = nullptr;
}

void ExQuestItems::GetOwnedItems( const class ExGame* Game , EGArray<exQuestItemInfo>& Out ) const
{
	for( const exQuestItemInfo& Info : m_QuestItems)
	{
		if( Game->GetGameVar( Info.HasVarId ).as_bool() )
		{
			Out.Append( Info );
		}
	}
}

void ExQuestItems::SetHasQuestItem( class ExGame* Game , eg_string_crc ItemId , eg_bool bHas ) const
{
	assert( Game && Game->IsServer() );
	if( Game && Game->IsServer() )
	{
		for( const exQuestItemInfo& Info : m_QuestItems)
		{
			if( Info.Id == ItemId && Game->HasGameVar( Info.HasVarId ) )
			{
				Game->SmSetVar( Info.HasVarId , bHas );
			}
		}
	}
}

void ExQuestItems::UnlockAllQuestItems( class ExGame* Game ) const
{
	assert( Game && Game->IsServer() );
	if( Game && Game->IsServer() )
	{
		for( const exQuestItemInfo& Info : m_QuestItems)
		{
			Game->SmSetVar( Info.HasVarId , true );
		}
	}
}

exQuestItemInfo ExQuestItems::GetItemInfo( eg_string_crc ItemId ) const
{
	for( const exQuestItemInfo& Info : m_QuestItems)
	{
		if( Info.Id.IsNotNull() && Info.Id == ItemId )
		{
			return Info;
		}
	}

	return exQuestItemInfo();
}
