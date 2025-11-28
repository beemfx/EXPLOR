// ExRoster
// (c) 2016 Beem Media

#include "ExRoster.h"
#include "ExGame.h"
#include "ExServer.h"
#include "ExGlobalData.h"

void ExRoster::InitDefaultRoster()
{
	for( eg_uint i=0; i<countof(m_PartyIndexes); i++ )
	{
		m_PartyIndexes[i] = PARTY_INVALID_INDEX;
	}

	auto InitCharater = [this]( ExFighter& Character , const eg_string_crc& Class , const eg_d_string& Name , eg_string_crc Portrait ) -> void
	{
		Character.InitAs( Class , false );
		Character.PortraitId = Portrait;

		EGString_Copy( Character.LocName , *Name , countof(Character.LocName) );
		Character.InitDefaultItemsAndSkills();
		Character.SetCreated( true );
	};

	const exCharacterClassGlobals& CharacterGlobals = ExGlobalData::Get().GetCharacterClassGlobals();
	for( eg_size_t i=0; i<CharacterGlobals.DefaultCharacters.Len(); i++ )
	{
		const exDefaultCharacter& DefaultCharacter = CharacterGlobals.DefaultCharacters[i];
		InitCharater( m_Characters[i].Fighter , DefaultCharacter.ClassId , DefaultCharacter.CharacterName , DefaultCharacter.PortraitId );
		m_PartyIndexes[i] = EG_To<eg_uint>(i);
	}

	m_PartyIndexes[0] = 0;
	m_PartyIndexes[1] = 1;
	m_PartyIndexes[2] = 2;
	m_PartyIndexes[3] = 3;
	m_PartyIndexes[4] = 4;
	m_PartyIndexes[5] = 5;
}

void ExRoster::SetFullRosterDirty()
{
	zero( &m_RepChecksums );
	zero( &m_PartyRepChecksum );
	m_Inventory.MarkAllDataDirty();
}

ExFighter* ExRoster::GetPartyMemberByIndex( eg_uint Index )
{
	ExFighter* Out = nullptr;

	if( 0 <= Index && Index < countof(m_PartyIndexes) )
	{
		eg_uint PartyIndex = m_PartyIndexes[Index];
		if( 0 <= PartyIndex && PartyIndex < countof(m_Characters) )
		{
			if( m_Characters[PartyIndex].Fighter.IsCreated() )
			{
				Out = &m_Characters[PartyIndex].Fighter;
			}
		}
	}

	return Out;
}

const ExFighter* ExRoster::GetPartyMemberByIndex( eg_uint Index ) const
{
	const ExFighter* Out = nullptr;

	if( 0 <= Index && Index < countof(m_PartyIndexes) )
	{
		eg_uint PartyIndex = m_PartyIndexes[Index];
		if( 0 <= PartyIndex && PartyIndex < countof(m_Characters) )
		{
			if( m_Characters[PartyIndex].Fighter.IsCreated() )
			{
				Out = &m_Characters[PartyIndex].Fighter;
			}
		}
	}

	return Out;
}

const ExFighter* ExRoster::GetRosterCharacterByIndex( eg_uint Index ) const
{
	const ExFighter* Out = nullptr;
	if( 0 <= Index && Index < countof(m_Characters) )
	{
		Out = &m_Characters[Index].Fighter;
	}
	return Out;
}

ExFighter* ExRoster::GetRosterCharacterByIndex( eg_uint Index )
{
	ExFighter* Out = nullptr;
	if( 0 <= Index && Index < countof(m_Characters) )
	{
		Out = &m_Characters[Index].Fighter;
	}
	return Out;
}

eg_uint ExRoster::GetRosterIndex( const ExFighter* PartyMember ) const
{
	for( eg_uint i=0; i<countof(m_Characters); i++ )
	{
		if( PartyMember == &m_Characters[i].Fighter )
		{
			return i;
		}
	}

	return PARTY_INVALID_INDEX;
}

eg_uint ExRoster::GetPartyIndex( const ExFighter* PartyMember ) const
{
	for( eg_uint i=0; i<PARTY_SIZE; i++ )
	{
		if( GetPartyMemberByIndex( i ) == PartyMember )
		{
			return i;
		}
	}

	return PARTY_INVALID_INDEX;
}

void ExRoster::ReplicateDirtyData( class ExGame* Owner )
{
	m_Inventory.ReplicateDirtyData( Owner );
	
	for( eg_uint i=0; i<countof(m_Characters); i++ )
	{
		m_Characters[i].Fighter.ClearPointers(); // Pointers may have been set for equipping items, etc, they are garbage on the server tho, so we clear them.
		eg_string_crc MemberChecksum = eg_string_crc::HashData( &m_Characters[i] , sizeof(m_Characters[i]) );
		if( MemberChecksum != m_RepChecksums[i] )
		{
			m_RepChecksums[i] = MemberChecksum;
			Owner->SDK_ReplicateToClient( &m_Characters[i] , sizeof(m_Characters[i]) );
			m_bBroadcastRosterUpdatedOnNextReplication = true;
		}
	}

	eg_string_crc PartyChecksum = eg_string_crc::HashData( &m_PartyIndexes , sizeof(m_PartyIndexes) );
	if( PartyChecksum != m_PartyRepChecksum )
	{
		m_PartyRepChecksum = PartyChecksum;
		Owner->SDK_ReplicateToClient( &m_PartyIndexes , sizeof(m_PartyIndexes) );
	}

	if( m_bBroadcastRosterUpdatedOnNextReplication )
	{
		m_bBroadcastRosterUpdatedOnNextReplication = false;
		Owner->SDK_RunClientEvent( eg_crc("OnPartyChanged") );
	}
}

void ExRoster::RemovePartyMemberByIndex( eg_uint Index )
{
	if( 0 <= Index && Index <countof(m_PartyIndexes) )
	{
		m_PartyIndexes[Index] = PARTY_INVALID_INDEX;
		/*
		for( eg_uint i=Index; i<countof(m_PartyIndexes)-1; i++ )
		{
			m_PartyIndexes[i] = m_PartyIndexes[i+1];
		}
		m_PartyIndexes[countof(m_PartyIndexes)-1] = PARTY_INVALID_INDEX;
		*/
	}

	m_bBroadcastRosterUpdatedOnNextReplication = true;
}

void ExRoster::ExchangePartyMembers( eg_uint PartyIndex0, eg_uint PartyIndex1 )
{
	if( 0 <= PartyIndex0 && PartyIndex0 < countof(m_PartyIndexes) && 0 <= PartyIndex1 && PartyIndex1 < countof(m_PartyIndexes) )
	{
		eg_uint OldOne = m_PartyIndexes[PartyIndex0];
		m_PartyIndexes[PartyIndex0] = m_PartyIndexes[PartyIndex1];
		m_PartyIndexes[PartyIndex1] = OldOne;
	}

	m_bBroadcastRosterUpdatedOnNextReplication = true;
}

void ExRoster::AddRosterMemberToParty( eg_uint RosterIndex, eg_uint PartyIndex )
{
	if( 0 <= RosterIndex && RosterIndex < countof(m_Characters) && 0 <= PartyIndex && PartyIndex < countof(m_PartyIndexes) )
	{
		// Make sure this guy isn't already in the party, and if he is, remove him.
		for( eg_uint i=0; i<countof(m_PartyIndexes); i++ )
		{
			if( m_PartyIndexes[i] == RosterIndex )
			{
				m_PartyIndexes[i] = 0;
			}
		}
		m_PartyIndexes[PartyIndex] = RosterIndex;
	}

	m_bBroadcastRosterUpdatedOnNextReplication = true;
}

void ExRoster::DeleteRosterMember( eg_uint RosterIndex )
{
	if( 0<= RosterIndex && RosterIndex < countof(m_Characters) )
	{
		m_Characters[RosterIndex].Fighter.SetCreated( false );

		// Also make sure this character isn't in the party.
		for( eg_size_t i=0; i<countof(m_PartyIndexes); i++ )
		{
			if( m_PartyIndexes[i] == RosterIndex )
			{
				m_PartyIndexes[i] = PARTY_INVALID_INDEX;
			}
		}
	}

	m_bBroadcastRosterUpdatedOnNextReplication = true;
}

void ExRoster::CreateRosterMember( const exRosterCharacterCreationData& CreateData )
{
	if( 0 <= CreateData.RosterIndex && CreateData.RosterIndex < countof(m_Characters) )
	{
		assert( !m_Characters[CreateData.RosterIndex].Fighter.IsCreated() ); // The roster member should not have been crated, this roster member will be overwritten
		m_Characters[CreateData.RosterIndex].Fighter = CreateData.Character;
		m_Characters[CreateData.RosterIndex].Fighter.SetCreated( true ); // Should have already been set, but just in case.

		m_bBroadcastRosterUpdatedOnNextReplication = true;
	}
	else
	{
		assert( false ); // Tried to create a roster member out of range.
	}
}

void ExRoster::OnItemReplicated( eg_size_t ItemIndex )
{
	m_Inventory.OnItemReplicated( ItemIndex );
}

void ExRoster::HandleEquipEvent( const exEquipEventData& Data )
{
	// The client should have done a sanity check to make sure this was allowed
	// so we're just going to go through with the change.
	eg_uint InventoryIndex = Data.GetInventoryIndex();
	eg_uint PartyMemberIndex = Data.GetPartyMemberIndex();
	ex_item_slot Slot = Data.GetItemSlot();

	ExFighter* PartyMember = GetPartyMemberByIndex( PartyMemberIndex );
	if( PartyMember )
	{
		exInventoryItem ItemToEquip( CT_Clear );
		if( InventoryIndex != exEquipEventData::UNEQUIP_INDEX )
		{
			ItemToEquip = m_Inventory.GetItemByIndex( InventoryIndex );
			m_Inventory.DeleteItemByIndex( InventoryIndex );
			ItemToEquip.ResolvePointers();
		}
	
		EGArray<exInventoryItem> ItemsRemoved;
		PartyMember->EquipInventoryItem( Slot , ItemToEquip , ItemsRemoved );

		eg_bool bFirstItem = true;
		for( const exInventoryItem& RemovedItem : ItemsRemoved )
		{
			if( bFirstItem && InventoryIndex != exEquipEventData::UNEQUIP_INDEX )
			{
				m_Inventory.AddItemAt( RemovedItem , InventoryIndex );
			}
			else
			{
				m_Inventory.AddItem( RemovedItem );
			}

			bFirstItem = false;
		}
	}

	m_bBroadcastRosterUpdatedOnNextReplication = true;
}

void ExRoster::HandleSwapEvent( const exSwapEventData& Data )
{
	if( Data.FromSlot != Data.ToSlot )
	{
		exInventoryItem SourceItem = m_Inventory.GetItemByIndex( Data.FromSlot );
		exInventoryItem DestItem = m_Inventory.GetItemByIndex( Data.ToSlot );
		m_Inventory.DeleteItemByIndex( Data.FromSlot );
		m_Inventory.DeleteItemByIndex( Data.ToSlot );
		m_Inventory.AddItemAt( SourceItem , Data.ToSlot );
		m_Inventory.AddItemAt( DestItem , Data.FromSlot );
		m_bBroadcastRosterUpdatedOnNextReplication = true;
	}
}

void ExRoster::SortInventory()
{
	m_Inventory.Sort();
	m_bBroadcastRosterUpdatedOnNextReplication = true;
}

void ExRoster::ResolveReplicatedData()
{
	m_Inventory.ResolveReplicatedData();

	for( exFighterItem& Fighter : m_Characters )
	{
		if( Fighter.Fighter.IsCreated() )
		{
			Fighter.Fighter.ResolveReplicatedData();
		}
	}
}

void ExRoster::GetCharactersWithPortrait( const eg_string_crc& CrcId , EGArray<const ExFighter*>& Out ) const
{
	for( const exFighterItem& Fighter : m_Characters )
	{
		if( CrcId.IsNotNull() && Fighter.Fighter.IsCreated() && Fighter.Fighter.PortraitId == CrcId )
		{
			Out.Append( &Fighter.Fighter );
		}
	}
}

void ExRoster::ClientCreateRosterCharacter( class ExGame* Owner, eg_uint RosterIndex, const ExFighter& CreateCharacter )
{
	if( 0 <= RosterIndex && RosterIndex < countof(m_Characters) )
	{
		m_Characters[RosterIndex].Fighter = CreateCharacter;
		m_Characters[RosterIndex].Fighter.InitDefaultItemsAndSkills();
		m_Characters[RosterIndex].Fighter.SetCreated( true );
		Owner->SDK_ReplicateToServer( &m_Characters[RosterIndex] , sizeof(m_Characters[RosterIndex]) );
		Owner->SDK_RunClientEvent( eg_crc( "OnPartyChanged" ) );
	}
	else
	{
		assert( false ); // Tried to create a roster member out of range.
	}
}

void ExRoster::ClientUpdateRosterCharacterByPartyIndex( class ExGame* Owner , eg_uint PartyIndex , const ExFighter& Character )
{
	if( 0 <= PartyIndex && PartyIndex < countof(m_PartyIndexes) )
	{
		eg_uint RosterIndex = m_PartyIndexes[PartyIndex];

		if( 0 <= RosterIndex && RosterIndex < countof( m_Characters ) )
		{
			m_Characters[RosterIndex].Fighter = Character;
			assert( m_Characters[RosterIndex].Fighter.IsCreated() ); // This shouldn't have been unset.
			m_Characters[RosterIndex].Fighter.SetCreated( true );
			m_Characters[RosterIndex].Fighter.SetCombatBoosts( nullptr );
			Owner->SDK_ReplicateToServer( &m_Characters[RosterIndex], sizeof( m_Characters[RosterIndex] ) );
			Owner->SDK_RunClientEvent( eg_crc( "OnPartyChanged" ) );
		}
		else
		{
			assert( false );
		}
	}
	else
	{
		assert( false ); // Not a valid party index.
	}
}
