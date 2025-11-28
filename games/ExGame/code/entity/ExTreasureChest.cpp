/******************************************************************************
ExTreasureChest - Treasure Chest AI

(c) 2016 Beem Media
******************************************************************************/

#include "ExTreasureChest.h"
#include "ExGame.h"
#include "EGBoolList.h"
#include "ExServer.h"
#include "ExPlayerController.h"
#include "ExQuestItems.h"

EG_CLASS_DECL( ExTreasureChest )

void ExTreasureChest::OnCreate( const egEntCreateParms& Parms )
{
	Super::OnCreate( Parms );

	m_EncounterPriority = 100;

	if( GetRole() == eg_ent_role::Authority )
	{		
		if( Parms.Reason != eg_spawn_reason::GameLoad )
		{
			m_bIsUnique = true;
		}

		if( m_bIsUnique && Parms.Reason != eg_spawn_reason::GameLoad )
		{
			// The game will know if this chest is open.
			m_bIsOpen = GetGameData()->IsChestOpen( m_InitId );
		}

		eg_transform NewPose = GetPose();
		NewPose.TranslateThis( eg_vec3( 0.f , -m_CurentNavVertex.GetPosition().y , 0.f ) );
		SetPose( NewPose );

		RunEvent( m_bIsOpen ? eg_crc( "SetOpened" ) : eg_crc( "SetClosed" ) );
	}
}

void ExTreasureChest::OnEnterWorld()
{
	Super::OnEnterWorld();

	if( m_bIsFadingOut )
	{
		FadeOut();
	}
}

void ExTreasureChest::OnEncounter()
{
	ExGame* GameData = GetGameData();
	if( GameData->IsIgnoringNextChestEncounter() )
	{
		GameData->SetIgnoreNextScriptEncounter();
		return;
	}

	if( !m_bIsOpen )
	{
		EGLogf( eg_log_t::Verbose, "Encountered Chest!" );
		GetGameData()->SmSetVar( eg_crc("LastGoldAmountFound") , m_Contents.GoldAmount );
		SDK_GetGame()->SDK_RunServerEvent( eg_crc( "BeginTreasureChestInteraction" ), GetID() );

		HandlePlayerLookAt( GameData->GetPlayerEnt() );
	}
}

void ExTreasureChest::OnUpdate( eg_real DeltaTime )
{
	Super::OnUpdate( DeltaTime );

	if( GetRole() == eg_ent_role::Authority )
	{
		if( m_bIsFadingOut )
		{
			m_FadeOutTime += DeltaTime;
			if( m_FadeOutTime >= 2.f )
			{
				DeleteFromWorld();
			}
		}
	}
}

void ExTreasureChest::OpenTreasureChest()
{
	RunEvent( eg_crc("Open") );
}

void ExTreasureChest::CloseTreasureChest()
{
	RunEvent( eg_crc("Close") );
}

void ExTreasureChest::RunTreasureScript()
{
	assert( IsServer() );

	if( m_Contents.HasContents() )
	{
		if( !m_bIsOpen )
		{
			ExGame* Game = GetGameData();
			ExQuestItems& QuestItems = ExQuestItems::Get();
			if( Game )
			{
				ExRoster& Roster = Game->GetRoster();
				ExInventory& Inventory = Roster.GetInventory();

				exTreasureChestResults ReplicatedContents = m_Contents;

				if( m_Contents.InventoryItems.Len() > Inventory.GetFreeSpace() )
				{
					ReplicatedContents.bWasAbleToOpen = false;
				}
				else
				{
					m_bIsOpen = true;	
					if( m_bIsUnique )
					{
						GetGameData()->SetChestOpen( m_InitId );
					}

					ReplicatedContents.bWasAbleToOpen = true;

					Game->SetPartyGold( Game->GetPartyGold() + m_Contents.GoldAmount );
					ReplicatedContents.GoldAmount = m_Contents.GoldAmount;
					for( const exRawItem& Item : m_Contents.InventoryItems )
					{
						Inventory.AddItem( exInventoryItem( Item ) );
					}
					for( const eg_string_crc& ItemId : m_Contents.QuestItems )
					{
						QuestItems.SetHasQuestItem( Game , ItemId , true );
					}
				}

				Game->SetLastTreasureChestContents( ReplicatedContents );
				Roster.SetBroadcastRosterUpdateOnNextReplication();
			}
		}
	}
}

void ExTreasureChest::AddGold( eg_int GoldAmount )
{
	m_Contents.GoldAmount += GoldAmount;
	if( m_bIsOpen || m_bIsFadingOut )
	{
		m_bIsOpen = false;

		RunEvent( eg_crc("SetClosed") ); // Could also be Close but looks weird to see a treasure chest close from open.
		RunEvent( eg_crc("ClearFade") );
		m_bIsFadingOut = false;
		m_FadeOutTime = 0.f;
		SetActive( false );
	}
}

void ExTreasureChest::FadeOut()
{
	assert( m_bIsOpen ); // Should only delete treasures that are open.
	assert( !m_bIsUnique ); // Should not fade out unique chests.
	RunEvent( eg_crc("FadeOut") );
	m_bIsFadingOut = true;
	m_FadeOutTime = 0.f;
	m_bEncountersEnabled = false;
	SetActive( true );
}

void ExTreasureChest::ParseInitString( const eg_d_string& InitString )
{
	m_InitId = eg_string_crc(*InitString);

	EGParse_ProcessFnCallScript( *InitString , InitString.Len() , [this]( const egParseFuncInfo& InfoStr )->void
	{
		if( EGString_EqualsI(InfoStr.FunctionName , "AddGold") && InfoStr.NumParms >= 1 )
		{
			m_Contents.GoldAmount += EGString_ToInt( InfoStr.Parms[0] );
		}
		else if( EGString_EqualsI( InfoStr.FunctionName , "AddItem" ) && InfoStr.NumParms >= 2 )
		{
			exRawItem AddedItem;
			AddedItem.ItemId = eg_string_crc(InfoStr.Parms[0]);
			AddedItem.Level = EGString_ToInt(InfoStr.Parms[1]);
			if( !m_Contents.InventoryItems.IsFull() )
			{
				m_Contents.InventoryItems.Append( AddedItem );
			}
			else
			{
				assert( false );
				EGLogf( eg_log_t::Warning , "Too many inventory items in a chest." );
			}
		}
		else if( EGString_EqualsI( InfoStr.FunctionName , "AddQuestItem" ) && InfoStr.NumParms >= 1 )
		{
			if( !m_Contents.QuestItems.IsFull() )
			{
				m_Contents.QuestItems.Append( eg_string_crc(InfoStr.Parms[0]) );
			}
			else
			{
				assert( false );
				EGLogf( eg_log_t::Warning , "Too many quest items in a chest." );
			}
		}
		else if( EGString_EqualsI( InfoStr.FunctionName , "SetId" ) && InfoStr.NumParms >= 1 )
		{
			m_InitId = eg_string_crc(InfoStr.Parms[0]);
		}
	} );
}
