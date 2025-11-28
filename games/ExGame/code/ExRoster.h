// ExRoster
// (c) 2016 Beem Media
#pragma once

#include "ExFighter.h"
#include "ExInventory.h"

struct exRosterCharacterCreationData
{
	eg_uint   RosterIndex;
	ExFighter Character = CT_Default;
};

class ExRoster
{
public:

	static const eg_size_t ROSTER_SIZE = 24;
	static const eg_size_t PARTY_INVALID_INDEX = ROSTER_SIZE;
	static const eg_size_t PARTY_SIZE  = 6;

public:

	void InitDefaultRoster();
	void SetFullRosterDirty();

	ExFighter* GetPartyMemberByIndex( eg_uint Index );
	const ExFighter* GetPartyMemberByIndex( eg_uint Index ) const;
	ExFighter* GetRosterCharacterByIndex( eg_uint Index );
	const ExFighter* GetRosterCharacterByIndex( eg_uint Index ) const;
	eg_uint GetRosterIndex( const ExFighter* PartyMember ) const;
	eg_uint GetPartyIndex( const ExFighter* PartyMember ) const;

	ExInventory& GetInventory(){ return m_Inventory; }
	const ExInventory& GetInventory() const { return m_Inventory; }

	void ReplicateDirtyData( class ExGame* Owner ); // Handles replication, etc...
	void RemovePartyMemberByIndex( eg_uint Index );
	void ExchangePartyMembers( eg_uint PartyIndex0 , eg_uint PartyIndex1 );
	void AddRosterMemberToParty( eg_uint RosterIndex , eg_uint PartyIndex );
	void DeleteRosterMember( eg_uint RosterIndex );
	void CreateRosterMember( const exRosterCharacterCreationData& CreateData );
	void OnItemReplicated( eg_size_t ItemIndex );
	void HandleEquipEvent( const exEquipEventData& Data );
	void HandleSwapEvent( const exSwapEventData& Data );
	void SortInventory();
	void ResolveReplicatedData();
	void SetBroadcastRosterUpdateOnNextReplication() { m_bBroadcastRosterUpdatedOnNextReplication = true; }
	void GetCharactersWithPortrait( const eg_string_crc& CrcId , EGArray<const ExFighter*>& Out ) const;

private:
	
	struct exFighterItem
	{
		ExFighter Fighter = CT_Preserve;
	};

private:

	exFighterItem     m_Characters[ROSTER_SIZE];
	eg_uint           m_PartyIndexes[PARTY_SIZE];

	ExInventory       m_Inventory;

	// Checksums for replication:
	eg_string_crc     m_RepChecksums[ROSTER_SIZE];
	eg_string_crc     m_PartyRepChecksum;

	eg_bool           m_bBroadcastRosterUpdatedOnNextReplication:1;
public:
	void ClientCreateRosterCharacter( class ExGame* Owner , eg_uint RosterIndex , const ExFighter& CreateCharacter );
	void ClientUpdateRosterCharacterByPartyIndex( class ExGame* Owner , eg_uint PartyIndex , const ExFighter& Character );
};
