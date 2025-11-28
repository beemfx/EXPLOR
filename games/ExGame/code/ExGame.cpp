// (c) 2016 Beem Media. All Rights Reserved.

#include "ExGame.h"
#include "ExEnt.h"
#include "ExMapNavBase.h"
#include "ExIoEnt.h"
#include "ExDialogMenu.h"
#include "EGParse.h"
#include "EGBase64.h"
#include "ExMapInfos.h"
#include "ExQuestItems.h"
#include "ExSaveMgr.h"
#include "ExServer.h"
#include "ExClient.h"
#include "ExPlayerController.h"
#include "ExTreasureChest.h"
#include "ExConversationMenu.h"
#include "EGSmMgr.h"
#include "ExScriptMenu.h"
#include "ExEnemyController.h"
#include "EGMenuStack.h"
#include "ExSkills.h"
#include "ExMapping.h"
#include "EGEntWorld.h"
#include "ExShopData.h"
#include "ExDoor.h"
#include "ExGlobalData.h"
#include "EGConDraw.h"
#include "ExCombatAI.h"
#include "ExCombatEncounter.h"
#include "ExBeastiary.h"
#include "ExGameSettings.h"
#include "ExCombat.h"
#include "ExRestMenu.h"
#include "ExMapTransitionEnt.h"
#include "ExProfileData.h"
#include "ExCameraSplineNodeEnt.h"
#include "EGDebugText.h"
#include "ExArmory.h"
#include "ExAchievements.h"
#include "ExNpcGossip.h"

EG_CLASS_DECL( ExGame )

const eg_uint32 ExGame::SAVE_VERSION = 107; // Increment this when format changes (and size did not, e.g. serialized entity changed)

static EGSettingsBool ExGame_SimulateRandomCombat( "ExGame.SimulateRandomCombat" , eg_loc("DebugSimulateCombatText","DEBUG: Simulate Random Combat") , false , EGS_F_DEBUG_EDITABLE|EGS_F_USER_SAVED );
static EGSettingsBool ExGame_AlwaysDoCombatAfterResting( "ExGame.AlwaysDoCombatAfterResting" , eg_loc("DebugCombatAfterRester","DEBUG: Force Ambushes While Resting") , false , EGS_F_DEBUG_EDITABLE|EGS_F_USER_SAVED );

ExGame::ExGame()
: m_PlayerId( INVALID_ID )
, m_PartyStatusChecksum( CT_Clear ) // Always want this cleared since on load we want replication
, m_MonsterList( CT_Preserve )
, m_TrackedIoEnts() // Map load fills this out
, m_SafeSpots() // Map load fills this out
, m_QueuedWorldMapTransition( CT_Clear )
, m_GameStateVars( CT_Preserve )
, m_CurScriptState( CT_Preserve )
, m_TreasureState( CT_Preserve )
, m_TransitTarget( CT_Preserve )
, m_Time( CT_Preserve )
, m_BuybackItems( CT_Preserve )
, m_ShopInventory( CT_Preserve )
, m_bHasPropperSpawnLocation( false )
, m_bRecallTownPortalQueued( false )
, m_bDontShowPopupForNextSave( false )
{
	#define REG_BH( _bh_ ) m_BhHandler.RegisterEvent( eg_crc(#_bh_) , &ExGame::Bh##_bh_ );
	REG_BH( DoSave )
	REG_BH( OnSendInitialRepData )
	REG_BH( OnTurnStart )
	REG_BH( OnPlayerRotated )
	REG_BH( OnTurnEnd )
	REG_BH( OnCombatResults )
	REG_BH( BeginTreasureChestInteraction )
	REG_BH( DoTreasureChestOpening )
	REG_BH( EndTreasureChestIntraction )
	REG_BH( OnConversationChoice )
	REG_BH( SaveFromInn )
	REG_BH( RosterMenuRemovePartyMember )
	REG_BH( RosterMenuExchangePartyMembers )
	REG_BH( RosterMenuAddPartyMemberToSlot )
	REG_BH( RosterMenuDeleteRosterMember )
	REG_BH( BeginRepFullGameState )
	REG_BH( ReplicateGameVar )
	REG_BH( ReplicateUnlockedSkill )
	REG_BH( EndRepFullGameState )
	REG_BH( OnStartConversation )
	REG_BH( OnConversationChoiceProcessed )
	REG_BH( OnConversationClear )
	REG_BH( OnScriptDialogBox )
	REG_BH( OnScriptDialogBoxChoiceMade )
	REG_BH( Rest )
	REG_BH( OpenScriptMenu )
	REG_BH( CloseScriptMenu )
	REG_BH( OnScriptMenuChoice )
	REG_BH( RefreshConversationNameplate )
	REG_BH( OnScriptMenuChoiceProcessed )
	REG_BH( DropGold )
	REG_BH( OpenGameMenu )
	REG_BH( OnGameMenuClosed )
	REG_BH( TrainPartyMember )
	REG_BH( RefreshGameMenu )
	REG_BH( OnItemReplicated )
	REG_BH( OnBeginCombat )
	REG_BH( BeginRestMenu )
	REG_BH( OnRosterRequestComplete )
	REG_BH( OnPartyChanged )
	REG_BH( OnRosterChanged )
	REG_BH( SetSelectedPartyMember )
	REG_BH( EquipInventoryItem )
	REG_BH( SwapInventoryItem )
	REG_BH( AttuneSkill )
	REG_BH( RevealMapSquare )
	REG_BH( SellInventoryItem )
	REG_BH( MakePurchase )
	REG_BH( SetHandleEncounterAfterScript )
	REG_BH( UnlockDoor )
	REG_BH( QueueWorldMapTransition )
	REG_BH( ClientHandlePartyMemberSwitched )
	REG_BH( ServerOpenContextMenu )
	REG_BH( ClientNotifyWait )
	REG_BH( ClientNotifyTurnStart )
	REG_BH( ClientNotifyTurnEnd )
	REG_BH( ServerDoTownPortal )
	REG_BH( QuitGame )
	REG_BH( ClientNotifyEvent )
	REG_BH( ServerNotifyEvent )
	REG_BH( DropInventoryItem )
	REG_BH( ClosedTreasureDialog )
	REG_BH( PlayCameraSpline )
	REG_BH( PlaySound )
	#undef REG_BH
}

void ExGame::Init( EGEntWorld* EntWorldOwner )
{
	Super::Init( EntWorldOwner );

	m_Skills = EGNewObject<ExSkills>( eg_mem_pool::System );
	m_Mapping = EGNewObject<ExMapping>( eg_mem_pool::System );

	if( m_Mapping )
	{
		m_Mapping->SetOwner( this );
	}

	if( IsClient() )
	{
		ExHUD* Hud = GetHUD();
		if( Hud )
		{
			Hud->InitGameComponents();
		}
	}

	if( IsServer() )
	{
		eg_bool bRandomizeFinalPuzzle = false;

		// The ExGame constructor is called under a few circumstances:
		// 1) New Game. All data will be zeroed out.
		// 2) Loading a Save Game. Data will be whatever it was when the game
		//    was saved, but any data with constructors will have those
		//    constructors run (eg_string, etc...) so all game data should stay
		//    aware of the fact that some of it's data (obviously not pointers)
		//    may still be valid. Lots of entity ids will be valid, but entities
		//    that are not serialized won't be (such as player controllers).
		// 3) Transitioning levels. Lots of data is probably still valid, but
		//    be wary that any entity ids would now be meaningless.
		if( m_SaveId == SAVE_ID && m_SaveVersion == SAVE_VERSION && m_ExGameSize == sizeof( ExGame ) )
		{
			// We are either loading or transitioning maps...
			// We'll get a OnMapLoad event soon enough and can handle
			// changing lots of data there.
			EGLogf( eg_log_t::GameLib, __FUNCTION__ ": Loaded: %s", m_bIsSave ? "Save Game" : "Map Transition" );
		}
		else if( m_SaveId == 0 && m_SaveVersion == 0 && m_ExGameSize == 0 )
		{
			EGLogf( eg_log_t::GameLib, __FUNCTION__ ": Created New Game" );
			// In this case everything should have been zeroed out, so we'll
			// do some checks to make sure...
			assert( ex_game_s::NONE == GetGameState() );
			assert( !m_bHasInnSave );
			assert( 0.f == m_TurnTime );
			assert( m_MonsterList.IsEmpty() );

			m_SaveId = SAVE_ID;
			m_SaveVersion = SAVE_VERSION;
			m_ExGameSize = sizeof( ExGame );
			EGString_Copy( m_SaveName , *ExSaveMgr::GetNextSaveName() , countof(m_SaveName) );
			m_Time.Day = (ExGlobalData::Get().GetStartingYear()-1)*exTime::DAYS_IN_YEAR + ExGlobalData::Get().GetStartingDay();
			m_Time.AddHours( ExGlobalData::Get().GetStartingHour() );
			m_PlayerSpawnLocation = CT_Default;
			m_EncounterDisposition = ex_encounter_disposition::Default;
			m_FrontLineSize = ExGlobalData::Get().GetCombatPlayerTeamFrontLineDefaultSize();

			m_Roster.InitDefaultRoster();
			m_Roster.GetInventory().SetBagSize( ExGlobalData::Get().GetStartingBagSize() );
			m_Roster.GetInventory().CreateDefaultInventory();

			m_Rng.ReSeed( m_Rng.CreateSeed() );
			m_SafeTurns = 0;

			bRandomizeFinalPuzzle = true;
		}
		else
		{
			EGLogf( eg_log_t::GameLib, __FUNCTION__ ": Loaded an out of date save." );
			if( IsServer() )
			{
				ExServer* Server = EGCast<ExServer>(GetServerOwner());
				Server->OnSaveIsOutOfDate();
			}
		}

		InitGameVars(); // preserves existing vars, clears non existing ones, creates new ones.

		if( bRandomizeFinalPuzzle )
		{
			SmSetVar( eg_crc("FinalPuzzleLever1Solution") , m_Rng.GetRandomRangeI( 1 , 2 ) );
			SmSetVar( eg_crc("FinalPuzzleLever2Solution") , m_Rng.GetRandomRangeI( 1 , 2 ) );
			SmSetVar( eg_crc("FinalPuzzleLever3Solution") , m_Rng.GetRandomRangeI( 1 , 2 ) );
			SmSetVar( eg_crc("FinalPuzzleLever4Solution") , m_Rng.GetRandomRangeI( 1 , 2 ) );
		}
	}
}

void ExGame::OnDestruct()
{
	if( IsClient() )
	{
		if( m_GameMenu.IsValid() )
		{
			// It's possible the game terminated while a game menu was open, 
			// in which case get rid of it.
			GetClientOwner()->SDK_GetMenuStack()->Clear();
		}
		ExHUD* Hud = GetHUD();
		if( Hud )
		{
			Hud->DeinitGameComponents();
		}
	}

	EGDeleteObject( m_Mapping );
	EGDeleteObject( m_Skills );

	m_CameraSplines.Clear();
}

void ExGame::LoadMap( eg_string_crc MapId )
{
	assert( IsServer() );

	m_CameraSplines.Clear();

	exMapInfoData Info = ExMapInfos::Get().FindInfo( MapId );
	
	if( m_EntWorldOwner )
	{
		m_PlayerId = INVALID_ID;
		m_ClientPCs.Clear();
		m_EntWorldOwner->ClearWorld();
		SpawnClients();
		if( Info.Id == MapId )
		{
			m_CurrentMapId = MapId;
		}
		else
		{
			EGLogf( eg_log_t::Warning, "An invalid map transition was specified, getting lost!" );
			Info = ExMapInfos::Get().FindInfo( ExGlobalData::Get().GetLostMapTravelTarget().MapId );
			m_CurrentMapId = Info.Id;
		}
		m_EntWorldOwner->LoadWorld( *Info.World.FullPath );
	}
}

void ExGame::DebugLoadMapByWoldFilename( eg_cpstr WorldFilename )
{
	assert( IsServer() );

	if( IsServer() && GetGameState() == ex_game_s::NONE )
	{
		SetGameState( ex_game_s::TRANS_LOADING );
		m_TransitTarget = exTransitTarget( CT_Clear );
		exMapInfoData Info = ExMapInfos::Get().FindInfoByWorldFilename( WorldFilename );
		AttemptAutoSave( true ); // Save right before leaving map
		m_TransitTarget.TargetMapId = Info.Id;
		m_TransitTarget.TargetEntranceId = eg_crc("TownPortal");
		LoadMap( m_TransitTarget.TargetMapId );
	}
	else
	{
		EGLogf( eg_log_t::Warning , "Tried to load \"%s\" while the game was doing something." , WorldFilename );
	}
}

void ExGame::BeginNewGame( const exPackedCreateGameInfo& NewGameData )
{
	SetGameState( ex_game_s::CREATED_NEW_GAME );
	m_SaveSlot = NewGameData.Slot;
	m_bNeedsInitialSave = true;
	m_TransitTarget = exTransitTarget( CT_Clear );
	m_TransitTarget.TargetMapId = ExGlobalData::Get().GetStartingMapTravelTarget().MapId;
	m_TransitTarget.TargetEntranceId = ExGlobalData::Get().GetStartingMapTravelTarget().EntranceId;
	LoadMap( m_TransitTarget.TargetMapId );
}

void ExGame::BeginLostGame()
{
	SetGameState( ex_game_s::CREATED_NEW_GAME );
	m_SaveSlot = 0;
	m_bNeedsInitialSave = false;
	m_TransitTarget = exTransitTarget( CT_Clear );
	m_TransitTarget.TargetMapId = ExGlobalData::Get().GetLostMapTravelTarget().MapId;
	m_TransitTarget.TargetEntranceId = ExGlobalData::Get().GetLostMapTravelTarget().EntranceId;
	LoadMap( m_TransitTarget.TargetMapId );
}

void ExGame::SaveGame( eg_bool bSaveFromInn /* = false */ )
{
	const eg_bool bDontShowPopup = m_bDontShowPopupForNextSave;
	m_bDontShowPopupForNextSave = false;

	assert( IsServer() );
	assert( GetGameState() == ex_game_s::NONE );
	assert( m_SaveSlot != 0 ); // How'd we create a new game without putting it in a save slot.
	assert( !m_bHandleEncountersAfterScript );
	assert( m_QueuedWorldMapTransition.IsNull() );
	assert( !m_bSaveOnNextUpdate );
	assert( !m_bMapScriptIsInConversation );

	SmSetVar( eg_crc("CombatsSinceLastSave") , 0 );

	m_Roster.GetInventory().ClearPointers();

	ExMapNavBase* PlayerEnt = static_cast<ExMapNavBase*>(GetEnt( m_PlayerId ));
	if( PlayerEnt )
	{
		assert( PlayerEnt->IsIdle() );
		ex_face FaceDir = PoseToFace( PlayerEnt->GetPose() );
		m_bIsSave = true;
		m_PlayerSpawnLocation = PlayerEnt->GetPose();
		if( bSaveFromInn )
		{
			eg_bool bHasLoadFromInnLocation = false;
			for( const exMapTransitionEnt& TransEnt : m_MapTransitionEnts )
			{
				if( TransEnt.Ent && TransEnt.Ent->GetId() == eg_crc("LoadFromInn") )
				{
					m_PlayerSpawnLocation = FaceToTransform( TransEnt.SpawnFace );
					m_PlayerSpawnLocation.TranslateThis( TransEnt.NavVertex.GetPosition().ToVec3() );
					bHasLoadFromInnLocation = true;
				}
			}

			if( !bHasLoadFromInnLocation )
			{
				const eg_vec3 T = m_PlayerSpawnLocation.GetTranslation();
				m_PlayerSpawnLocation.SetTranslation( eg_vec3(0.f,0.f,0.f) );
				m_PlayerSpawnLocation.RotateYThis( EG_Deg(180.f) );
				m_PlayerSpawnLocation.SetTranslation( T );
			}
		}
		m_SaveTime = EGTimer::GetSysTime();
		ExSaveMgr::Get().SaveSave( this , m_SaveSlot );
		if( !bDontShowPopup )
		{
			SDK_RunClientEvent( eg_crc("ProgressSaved") );
		}
		m_bIsSave = false;
	}
}

void ExGame::AttemptAutoSave( eg_bool bImmediate )
{
	EGLogf( eg_log_t::General , "Attempting auto save..." );
	if( GetCurrentMapInfo().MapType == ex_map_info_map_t::Town )
	{
		EGLogf( eg_log_t::General , "In town, saving..." );
		if( bImmediate )
		{
			const ex_game_s PrevGameState = GetGameState();
			if( PrevGameState == ex_game_s::NONE || PrevGameState == ex_game_s::TRANS_LOADING )
			{
				m_GameStateInternal = ex_game_s::NONE;
				SaveGame();
				m_GameStateInternal = PrevGameState;
			}
			else
			{
				assert( false ); // Can't autosave right now.
			}
		}
		else
		{
			m_bSaveOnNextUpdate = true;
		}
	}
	else
	{
		EGLogf( eg_log_t::General , "In a dungeon, save not allowed." );
	}
}

void ExGame::AddMonster( eg_ent_id MonsterId )
{
	m_MonsterList.Push( MonsterId );
}

void ExGame::Update( eg_real DeltaTime )
{
	Super::Update( DeltaTime );

	if( IsServer() )
	{
		Update_AsServer( DeltaTime );
	}

	if( IsClient() )
	{
		Update_AsClient( DeltaTime );
	}
}

eg_bool ExGame::IsFacing( ex_face Face1, ex_face Face2 )
{
	if( Face1 == ex_face::ALL || Face2 == ex_face::ALL )
	{
		return true;
	}

	if( Face1 == ex_face::NONE || Face2 == ex_face::NONE )
	{
		return false;
	}

	eg_bool Out = false;
	switch( Face1 )
	{
	default:
		assert( false );
		// fallthru:
	case ex_face::NORTH: Out = Face2 == ex_face::SOUTH; break;
	case ex_face::EAST: Out = Face2 == ex_face::WEST; break;
	case ex_face::SOUTH: Out = Face2 == ex_face::NORTH; break;
	case ex_face::WEST: Out = Face2 == ex_face::EAST; break;
	}
	return Out;
}

const ExFighter* ExGame::GetPartyMemberInCombatByIndex( eg_uint Index ) const
{
	assert( IsClient() );
	assert( IsInCombat() );

	if( m_CurrentCombat )
	{
		return m_CurrentCombat->GetPartyMemberByIndex( 0 , Index );
	}

	return nullptr;
}

ex_attr_value ExGame::GetPartyMinLevel() const
{
	ex_attr_value Out = 0;
	eg_bool bFoundOne = false;

	for( eg_int i=0; i<ExRoster::PARTY_SIZE; i++ )
	{
		const ExFighter* PartyMember = m_Roster.GetPartyMemberByIndex( i );
		if( PartyMember )
		{
			const ex_attr_value Lvl = PartyMember->GetAttrValue( ex_attr_t::LVL );
			if( !bFoundOne )
			{
				bFoundOne = true;
				Out = Lvl;
			}
			else
			{
				Out = EG_Min( Out , Lvl );
			}
		}
	}
	
	return Out;
}

ex_attr_value ExGame::GetPartyMaxLevel() const
{
	ex_attr_value Out = 0;
	eg_bool bFoundOne = false;

	for( eg_int i=0; i<ExRoster::PARTY_SIZE; i++ )
	{
		const ExFighter* PartyMember = m_Roster.GetPartyMemberByIndex( i );
		if( PartyMember )
		{
			const ex_attr_value Lvl = PartyMember->GetAttrValue( ex_attr_t::LVL );
			if( !bFoundOne )
			{
				bFoundOne = true;
				Out = Lvl;
			}
			else
			{
				Out = EG_Max( Out , Lvl );
			}
		}
	}

	return Out;
}

ex_xp_value ExGame::GetPartyMinXP() const
{
	ex_xp_value Out = 0;
	eg_bool bFoundOne = false;

	for( eg_int i=0; i<ExRoster::PARTY_SIZE; i++ )
	{
		const ExFighter* PartyMember = m_Roster.GetPartyMemberByIndex( i );
		if( PartyMember )
		{
			const ex_xp_value Value = PartyMember->GetXP();
			if( !bFoundOne )
			{
				bFoundOne = true;
				Out = Value;
			}
			else
			{
				Out = EG_Min( Out , Value );
			}
		}
	}

	return Out;
}

ex_xp_value ExGame::GetPartyMaxXP() const
{
	ex_xp_value Out = 0;
	eg_bool bFoundOne = false;

	for( eg_int i=0; i<ExRoster::PARTY_SIZE; i++ )
	{
		const ExFighter* PartyMember = m_Roster.GetPartyMemberByIndex( i );
		if( PartyMember )
		{
			const ex_xp_value Value = PartyMember->GetXP();
			if( !bFoundOne )
			{
				bFoundOne = true;
				Out = Value;
			}
			else
			{
				Out = EG_Max( Out , Value );
			}
		}
	}

	return Out;
}

ex_attr_value ExGame::GetPartyMinTrainToLevel() const
{
	return ExCore_GetMaxLevelToTrainTo( GetPartyMinXP() );
}

ex_attr_value ExGame::GetPartyMaxTrainToLevel() const
{
	return ExCore_GetMaxLevelToTrainTo( GetPartyMaxXP() );
}

eg_int ExGame::GetNumMapSquaresRevealed() const
{
	return m_Mapping ? m_Mapping->GetNumSquaresRevealed() : 0;
}

void ExGame::SetCurrentCombat( ExCombat* InCombat )
{
	assert( IsClient() );
	assert( (m_CurrentCombat && nullptr == InCombat) || !m_CurrentCombat );
	m_CurrentCombat = InCombat;
}

const ExCombat* ExGame::GetCombat() const
{
	assert( IsClient() );
	assert( IsInCombat() );

	return m_CurrentCombat.GetObject();
}

egsm_var& ExGame::GetGameVar( eg_string_crc Id )
{
	assert( m_GameStateVars.Contains( Id ) );
	return m_GameStateVars[Id];
}

const egsm_var& ExGame::GetGameVar( eg_string_crc Id ) const
{
	assert( m_GameStateVars.Contains( Id ) );
	return m_GameStateVars[Id];
}

eg_bool ExGame::HasGameVar( eg_string_crc Id ) const
{
	return m_GameStateVars.Contains( Id );
}

void ExGame::GetCurScriptChoices( EGSmBranchOptions& OptionsOut ) const
{
	OptionsOut = m_CurConvState.Choices;
}

const exMapInfoData& ExGame::GetCurrentMapInfo() const
{
	return ExMapInfos::Get().FindInfo( GetCurrentMapId() );
}

ExMenu* ExGame::GetGameMenu()
{
	assert( IsClient() );
	return m_GameMenu.GetObject();
}

ExMenu* ExGame::SwitchGameMenu( eg_string_crc MenuId )
{
	assert( IsClient() );
	assert( m_GameMenu.IsValid() );
	if( m_GameMenu.IsValid() )
	{
		// assert( m_GameMenu->IsActive() );
		m_GameMenu = EGCast<ExMenu>(GetClientOwner()->SDK_GetMenuStack()->PopToSwitchTo( m_GameMenu.GetObject() , MenuId ));
	}
	return m_GameMenu.GetObject();
}

const EGArray<exDayNightCycleState>& ExGame::GetCurrentDayNightCycle() const
{
	assert( IsClient() );
	return m_DayNightCycle;
}

void ExGame::GetInventoryItems( EGArray<exInventoryItem>& Out ) const
{
	m_Roster.GetInventory().GetItems( Out );
}

eg_angle ExGame::GetPlayerRotation() const
{
	eg_angle Rot = EG_Rad(0.f);
	const ExEnt* Ent = GetPlayerEnt();
	if( Ent )
	{
		eg_transform EntPose = Ent->GetPose();
		eg_vec4 FaceDir( 0 , 0 , 1.f , 0.f );
		eg_transform EntPoseM( EntPose );
		FaceDir *= EntPoseM;
		const eg_real& y = FaceDir.x;
		const eg_real& x = FaceDir.z;

		Rot = EGMath_atan2( y , x );
	}

	return Rot;
}

ex_attr_value ExGame::GetPartyGold() const
{
	egsm_var PartyGold = SmGetVar( eg_crc("PartyGold") );
	return PartyGold.as_int();
}

void ExGame::SetPartyGold( ex_attr_value NewValue )
{
	assert( IsServer() );

	SmSetVar( eg_crc("PartyGold") , NewValue );
}

ex_can_buy_t ExGame::CanBuyItem( const exInventoryItem& Item, eg_bool bIsBuyback ) const
{
	ex_attr_value PartyGold = GetPartyGold();
	assert( Item.ArmoryItem );
	ex_attr_value Cost = bIsBuyback ? Item.GetSellValue() : Item.GetBuyValue();

	if( Cost > PartyGold )
	{
		return ex_can_buy_t::TooExpensive;
	}

	if( m_Roster.GetInventory().IsBagFull() )
	{
		return ex_can_buy_t::BagsFull;
	}

	return ex_can_buy_t::CanBuy;
}

void ExGame::UnlockTargetedDoor()
{
	SDK_RunServerEvent( eg_crc("UnlockDoor") );
}

void ExGame::QueueWorldMapTransitionFromClient( eg_string_crc TargetMapId )
{
	SDK_RunServerEvent( eg_crc("QueueWorldMapTransition") , eg_event_parms(TargetMapId) );
}

eg_int ExGame::CalculateCostToHealParty() const
{
	eg_int TotalCost = 0;

	for( eg_int i=0; i<ExRoster::PARTY_SIZE; i++ )
	{
		const ExFighter* PartyMember = m_Roster.GetPartyMemberByIndex( i );
		if( PartyMember )
		{
			ex_attr_value MemberBaseCost = ExGlobalData::Get().GetBaseHealerHealCost();
			const ex_attr_value Level = PartyMember->GetAttrValue( ex_attr_t::LVL );
			if( PartyMember->IsDead() )
			{
				MemberBaseCost = EG_Max( MemberBaseCost , ExGlobalData::Get().GetBaseHealerResurrectCost() );
			}
			if( PartyMember->IsCursed() )
			{
				MemberBaseCost = EG_Max( MemberBaseCost , ExGlobalData::Get().GetBaseHealerUnCurseCost() );
			}
			if( PartyMember->IsPetrified() )
			{
				MemberBaseCost = EG_Max( MemberBaseCost , ExGlobalData::Get().GetBaseHealerUnPetrifyCost() );
			}

			ex_attr_value HealCost = ExCore_GetRampedValue_Level( MemberBaseCost , Level , EX_LEVEL_GROWTH );
			TotalCost += HealCost;
		}
	}

	return TotalCost;
}

eg_bool ExGame::IsSaveAndQuit() const
{
	return GetCurrentMapInfo().MapType == ex_map_info_map_t::Town;
}

void ExGame::DebugBalanceParty( ex_attr_value Level )
{
	if( Level == 0 )
	{
		const exMapInfoMonsterData& EncounterData = 	GetCurrentMapInfo().EncounterData;
		if( EncounterData.RandomEncounterThresholdPercents.IsValidIndex( 0 ) )
		{
			Level = EncounterData.RandomEncounterThresholdPercents[0].Level;
		}
		else
		{
			Level = 1;
		}
	}

	Level = EG_Clamp<ex_attr_value>( Level , 1 , EX_MAX_LEVEL );

	EGLogf( eg_log_t::General , "Balancing party to LVL %d" , Level );

	const ex_xp_value XpForLevel = ExCore_GetXpNeededForLevel( Level );

	for( eg_uint i=0; i<ExRoster::PARTY_SIZE; i++ )
	{
		ExFighter* PartyMember = GetPartyMemberByIndex( i );
		if( PartyMember )
		{
			const ex_xp_value XpNeeded = EG_Max<ex_xp_value>( 0 , XpForLevel - PartyMember->GetXP() );

			PartyMember->AwardXP( XpNeeded );
			PartyMember->SetAttrValue( ex_attr_t::LVL , Level );
			PartyMember->RestoreAllHP();
			PartyMember->RestoreAllMP();
			PartyMember->SetUnconcious( false );
			PartyMember->SetDead( false );
			PartyMember->SetCursed( false );
			PartyMember->SetPetrified( false );

			const exCharacterClassGlobals& CharacterGlobals = ExGlobalData::Get().GetCharacterClassGlobals();
			const exCharacterClass& ClassData = CharacterGlobals.GetClass( ExCore_ClassEnumToClassId( PartyMember->GetClass() ) );
			const exCharacterClassBalanceData* BalanceDataToUse = nullptr;
			for( const exCharacterClassBalanceData& BalanceData : ClassData.BalanceData )
			{
				if( BalanceData.Level <= Level )
				{
					BalanceDataToUse = &BalanceData;
				}
				else
				{
					break;
				}
			}

			if( BalanceDataToUse )
			{
				EGArray<exInventoryItem> ItemsRemoved;
				PartyMember->EquipInventoryItem( ex_item_slot::WEAPON2 , exInventoryItem( BalanceDataToUse->LhWeapon , true ) , ItemsRemoved ); // Weapon 2 first in case weapon 1 is two handed and would remove weapon 2 (even if weapon 2 is empty)
				PartyMember->EquipInventoryItem( ex_item_slot::WEAPON1 , exInventoryItem( BalanceDataToUse->RhWeapon , true ) , ItemsRemoved );
				PartyMember->EquipInventoryItem( ex_item_slot::ARMOR , exInventoryItem( BalanceDataToUse->Armor , true ) , ItemsRemoved );
				PartyMember->EquipInventoryItem( ex_item_slot::HELM , exInventoryItem( BalanceDataToUse->Helm , true ) , ItemsRemoved );
			}
		}
	}

	m_Roster.SetBroadcastRosterUpdateOnNextReplication();
}

void ExGame::DebugSimulateCombatEncounters( eg_int NumEncounters , eg_int NumBosses )
{
	EGLogf( eg_log_t::General , "Simulating %d encounters..." , NumEncounters );

	ex_xp_value TotalXpEarned = 0;
	ex_attr_value TotalGoldMade = 0;

	auto ProcessEncounter = [this,&TotalGoldMade,&TotalXpEarned]( eg_int Index , eg_string_crc EncounterId ) -> void
	{
		ex_xp_value EncounterXpEarned = 0;
		ex_attr_value EncounterGoldMade = 0;

		DebugProcessEncounter( EncounterId , &EncounterGoldMade , &EncounterXpEarned );

		if( Index >= 0 )
		{
			eg_d_string8 ResultText = EGSFormat8( "Encounter {0}: XP Earned: {1:PRETTY} ({2:PRETTY}) , Gold Earned: {3:PRETTY}" , Index+1 , EG_To<eg_int>(EncounterXpEarned) , EG_To<eg_int>(EncounterXpEarned)/ExRoster::PARTY_SIZE , EncounterGoldMade );
			EGLogf( eg_log_t::General , *ResultText );
		}
		else
		{
			eg_d_string8 ResultText = EGSFormat8( "Encounter {0}: XP Earned: {1:PRETTY} ({2:PRETTY}) , Gold Earned: {3:PRETTY}" , EG_Abs( Index ) , EG_To<eg_int>(EncounterXpEarned) , EG_To<eg_int>(EncounterXpEarned)/ExRoster::PARTY_SIZE , EncounterGoldMade );
			EGLogf( eg_log_t::General , *ResultText );
		}

		TotalXpEarned += EncounterXpEarned;
		TotalGoldMade += EncounterGoldMade;
	};

	for( eg_int i=0; i<NumEncounters; i++ )
	{
		eg_string_crc EncounterId = eg_crc("DefaultEncounter");
		const eg_string_crc RandomEncounterId = GetCurrentMapInfo().EncounterData.RandomEncouter.GetEncounter( m_Rng );
		if( RandomEncounterId )
		{
			EncounterId = RandomEncounterId;
		}
	
		ProcessEncounter( i , EncounterId );
	}

	for( eg_int i=0; i<NumBosses; i++ )
	{
		ProcessEncounter( -1 , GetCurrentMapInfo().EncounterData.Boss );
	}

	EGLogf( eg_log_t::General , *EGSFormat8( "XP Earned: {0:PRETTY} ({1:PRETTY}) , Gold Earned: {2:PRETTY}" , EG_To<eg_int>(TotalXpEarned) , EG_To<eg_int>(TotalXpEarned)/ExRoster::PARTY_SIZE , TotalGoldMade ) );
}

void ExGame::DebugProcessEncounter( eg_string_crc EncounterId , ex_attr_value* GoldOut , ex_xp_value* XpOut )
{
	if( const exCombatEncounterInfo* Encounter = ExCombatEncounter::Get().FindCombatEncounter( EncounterId ) )
	{
		ex_xp_value EncounterXpEarned = 0;
		ex_attr_value EncounterGoldMade = 0;

		EGArray<exCombatEncounterMonster> MonsterList;
		Encounter->CreateMonsterList( m_Rng , MonsterList );

		for( const exCombatEncounterMonster& MonsterInfo : MonsterList )
		{
			if( MonsterInfo.BeastInfo && MonsterInfo.Level > 0 )
			{
				ex_xp_value XPReward = MonsterInfo.BeastInfo->GetXPReward( MonsterInfo.Level ); 
				ex_attr_value GoldDropped = MonsterInfo.BeastInfo->GetGoldDropAmount( MonsterInfo.Level , m_Rng );
				EncounterXpEarned += XPReward;
				EncounterGoldMade += GoldDropped;
			}
		}

		*GoldOut = EncounterGoldMade;
		*XpOut = EncounterXpEarned;
	}
}

void ExGame::SetTrackedMapIcon( ExMapIconComponent* InTrackedIcon , eg_bool bTrack )
{
	if( bTrack )
	{
		assert( !m_TrackedMapIcons.Contains( InTrackedIcon ) );
		m_TrackedMapIcons.AppendUnique( InTrackedIcon );
	}
	else
	{
		assert( m_TrackedMapIcons.Contains( InTrackedIcon ) );
		m_TrackedMapIcons.DeleteByItem( InTrackedIcon );
	}

	OnTrackedMapIconsChanged.Broadcast();
}

void ExGame::SetTrackedMapTransitionEnt( ExMapTransitionEnt* InEnt , eg_bool bTrack )
{
	if( InEnt )
	{
		eg_uint ClosestVertexIdx = m_NavGraph.GetVertexIndexClosestTo( this , eg_vec4(InEnt->GetPose().GetTranslation(),1.f) );
		EGNavGraphVertex ClosestVertex( CT_Clear );
		eg_bool bGotVertex = m_NavGraph.GetVertex( this , ClosestVertexIdx , &ClosestVertex );
		assert( bGotVertex ); // Couldn't associate this function call with a vertex...

		exMapTransitionEnt TransitionEnt;
		TransitionEnt.Ent = InEnt;
		TransitionEnt.NavVertex = ClosestVertex;
		TransitionEnt.InteractFace = PoseToFace( InEnt->GetPose() );
		TransitionEnt.SpawnFace = GetReverseFace( TransitionEnt.InteractFace );

		if( bTrack )
		{
			assert( !m_MapTransitionEnts.Contains( TransitionEnt ) );
			m_MapTransitionEnts.AppendUnique( TransitionEnt );

			if( !m_bHasPropperSpawnLocation && m_TransitTarget.TargetEntranceId == InEnt->GetId() )
			{
				m_bHasPropperSpawnLocation = true;
				m_PlayerSpawnLocation = FaceToTransform( TransitionEnt.SpawnFace );
				m_PlayerSpawnLocation.TranslateThis( TransitionEnt.NavVertex.GetPosition().ToVec3() );

				// Teleport player here
				ExPlayerController* Player = GetPlayerEnt();
				if( Player )
				{
					Player->RestablishNavGraph( m_PlayerSpawnLocation );
				}
				AttemptAutoSave( false );
			}
		}
		else
		{
			assert( m_MapTransitionEnts.Contains( TransitionEnt ) );
			m_MapTransitionEnts.DeleteByItem( TransitionEnt );
		}
	}
	else
	{
		assert( false );
	}
}

void ExGame::SetCameraSplineNode( ExCameraSplineNodeEnt* InSplineNode , eg_bool bAdd )
{
	if( InSplineNode )
	{
		const eg_int Index = InSplineNode->GetSplineNodeIndex();
		const eg_string_crc SplineId = InSplineNode->GetSplineId();
		if( SplineId.IsNull() )
		{
			assert( false ); // Should set id on node.
			return;
		}

		EGTransformSpline* SplineToChange = nullptr;

		for( EGTransformSpline& Spline : m_CameraSplines )
		{
			if( SplineId == Spline.GetId() )
			{
				SplineToChange = &Spline;
			}
		}

		if( nullptr == SplineToChange && bAdd )
		{
			// No such spline. Create a new one.
			m_CameraSplines.AppendMove( EGTransformSpline() );
			SplineToChange = m_CameraSplines.HasItems() ? &m_CameraSplines[m_CameraSplines.Len()-1] : nullptr;

			// Make sure we created a new one.
			if( SplineToChange && SplineToChange->GetId().IsNull() )
			{
				SplineToChange->SetId( SplineId );
			}
			else
			{
				SplineToChange = nullptr;
			}
		}

		if( SplineToChange && SplineToChange->GetId() == SplineId )
		{
			if( bAdd )
			{
				SplineToChange->InsertSplineNode( Index , InSplineNode->GetPose() );
			}
			else
			{
				SplineToChange->RemoveSplineNode( Index );
			}
		}
	}
}

void ExGame::PlayCameraSpline( eg_string_crc SplineDataId )
{
	// We could be switching from one player to spline, or we could be switching from
	// another spline, both cases should be covered.

	if( SplineDataId.IsNull() )
	{
		return;
	}

	const exCameraSplineData* DataToPlay = nullptr;
	const EGArray<exCameraSplineData>& CameraSplineDatas = ExGlobalData::Get().GetCameraSplines();
	for( const exCameraSplineData& Data : CameraSplineDatas )
	{
		if( Data.Id == SplineDataId )
		{
			DataToPlay = &Data;
		}
	}

	if( nullptr == DataToPlay )
	{
		return;
	}

	m_ActiveCameraSplineIndex = -1;
	const eg_transform CurCameraPose = GetCameraPose();

	for( eg_int i=0; m_ActiveCameraSplineIndex == -1 && i<m_CameraSplines.LenAs<eg_int>(); i++ )
	{
		if( m_CameraSplines[i].GetId() == DataToPlay->SplineId )
		{
			m_ActiveCameraSplineIndex = i;
		}
	}
	
	if( m_CameraSplines.IsValidIndex( m_ActiveCameraSplineIndex ) )
	{
		m_CameraMode = ex_camera_mode::CameraSpline;
		const eg_transform StartingPose = DataToPlay->bJumpToStart ? m_CameraSplines[m_ActiveCameraSplineIndex].GetFirstPose() : CurCameraPose;
		egTransformSplineInitData InitData;
		InitData.Acceleration = DataToPlay->Acceleration;
		InitData.StartingSpeed = DataToPlay->StartingSpeed;
		InitData.MaxSpeed = DataToPlay->MaxSpeed;
		InitData.bLoop = DataToPlay->bLooping;
		InitData.InitialPose = StartingPose;
		InitData.SplineType = ex_transform_spline_t::CatmullRom;
		m_CameraSplines[m_ActiveCameraSplineIndex].BeginSpline( InitData );
	}
	else
	{
		m_CameraMode = ex_camera_mode::PlayerView;
	}

	SDK_ReplicateToClient( &m_CameraMode , sizeof(m_CameraMode) );
}

void ExGame::StopCameraSpline()
{
	if( m_CameraMode == ex_camera_mode::CameraSpline )
	{
		m_CameraMode = ex_camera_mode::PlayerView;

		if( m_CameraSplines.IsValidIndex( m_ActiveCameraSplineIndex ) )
		{
			m_CameraSplines[m_ActiveCameraSplineIndex].StopSpline();
			m_CameraSplineRepPose = m_CameraSplines[m_ActiveCameraSplineIndex].GetCurrentPose();
		}
	}

	SDK_ReplicateToClient( &m_CameraMode , sizeof(m_CameraMode) );
}

eg_transform ExGame::GetCameraPose() const
{
	eg_transform Out = CT_Default;
	if( m_CameraMode == ex_camera_mode::PlayerView )
	{
		const ExPlayerController* MyPC = GetPlayerEnt();
		if( MyPC )
		{
			Out = MyPC->GetCameraPose();
		}
		else
		{
			// This is in case the player controller is not an ExPlayerController (using ExFreeMovePlayerController for example)
			const ExEnt* DefEnt = EGCast<ExEnt>( SDK_GetEnt( GetPrimaryPlayer() ) );
			if( DefEnt )
			{
				Out = DefEnt->GetPose();
			}
		}
	}
	else if( m_CameraMode == ex_camera_mode::CameraSpline )
	{
		Out = m_CameraSplineRepPose;
	}

	return Out;
}

bool ExGame::IsBossDefeated( const exDefeatedBoss& BossInfo ) const
{
	return m_DefeatedBossList.Contains( BossInfo );
}

void ExGame::SetGameState( ex_game_s NewGameState )
{
	assert( IsServer() );
	if( m_GameStateInternal != NewGameState )
	{
		m_GameStateInternal = NewGameState;
		SDK_ReplicateToClient( &m_GameStateInternal , sizeof(m_GameStateInternal) );
	}
}

void ExGame::InitGameVars()
{
	// Basically variables may have been deleted or added, so we're going to
	// make a whole new list. That way the list that is serialized is always
	// in the correct order in memory with no gaps.

	ExGameVarSearchMap NewVars( CT_Clear );

	EGArray<egsmVarPair> AllVars;
	EGSmMgr_GetVars( AllVars );

	for( const egsmVarPair& VarPair : AllVars )
	{
		if( m_GameStateVars.Contains( VarPair.Id ) )
		{
			NewVars.Insert( VarPair.Id , m_GameStateVars[VarPair.Id] );
		}
		else
		{
			NewVars.Insert( VarPair.Id , VarPair.Value );
		}
	}

	m_GameStateVars.Clear();

	assert( NewVars.Len() == AllVars.Len() );

	for( eg_uint i=0; i<NewVars.Len(); i++ )
	{
		m_GameStateVars.Insert( NewVars.GetKeyByIndex(i) , NewVars.GetByIndex(i) );
	}

	assert( m_GameStateVars.Len() == AllVars.Len() );

	if( IS_DEBUG )
	{
		eg_string_crc DebugFeaturesVar = eg_crc("bDebugFeaturesEnabled");
		if( m_GameStateVars.Contains( DebugFeaturesVar ) )
		{
			GetGameVar( DebugFeaturesVar ) = true;
		}
	}
}

void ExGame::GeneratedNewSeeds()
{
	assert( IsServer() );

	m_SeedDay = m_Time.Day;

	//
	// Shop Seed
	//

	m_ShopSeed = EG_Max<eg_uint>( 1 , EGRandom::CreateSeed() );
	m_ShopSeedPartyLevel = GetPartyMaxLevel();
	SDK_ReplicateToClient( &m_ShopSeed , sizeof(m_ShopSeed) );
	SDK_ReplicateToClient( &m_ShopSeedPartyLevel , sizeof(m_ShopSeedPartyLevel) );

	m_ShopInventory.Clear();
	EGArray<exRawItem> InventoryItems;
	ExShopData::Get().GetInventory( ExGlobalData::Get().GetDefaultShopKeeperInventory() , InventoryItems , m_ShopSeedPartyLevel , m_ShopSeed );
	for( const exRawItem& Item : InventoryItems )
	{
		m_ShopInventory.Append( Item );
	}
	SDK_ReplicateToClient( &m_ShopInventory , sizeof(m_ShopInventory) );

	//
	// NPC Gossip Seed
	//

	m_NpcGossipSeed = EG_Max<eg_uint>( 1 , EGRandom::CreateSeed() );
}

void ExGame::Update_AsServer( eg_real DeltaTime )
{
	assert( IsServer() );
	assert( !m_bIsSave ); // We should not update until after the fact that the game was loaded from a save is processed.

	ExPlayerController* PlayerEnt = GetPlayerEnt();

	if( m_CameraMode == ex_camera_mode::CameraSpline && m_CameraSplines.IsValidIndex( m_ActiveCameraSplineIndex ) )
	{
		EGTransformSpline& ActiveSpline = m_CameraSplines[m_ActiveCameraSplineIndex];
		ActiveSpline.Update( DeltaTime );
		m_CameraSplineRepPose = ActiveSpline.GetCurrentPose();
		m_CameraSplineRepPose.RecPose.TimeMs = m_EntWorldOwner ? m_EntWorldOwner->GetGameTimeMs() : 0;
		SDK_ReplicateToClient( &m_CameraSplineRepPose.RecPose , sizeof(m_CameraSplineRepPose.RecPose) );
	}

	if( GetGameState() == ex_game_s::NONE && m_bRecallTownPortalQueued )
	{
		m_bRecallTownPortalQueued = false;
		SDK_RunServerEvent( eg_crc("ServerDoTownPortal") , true ); // Treat it same as casting spell from client.
	}

	if( GetGameState() == ex_game_s::NONE && m_bHandleEncountersAfterScript )
	{
		m_bHandleEncountersAfterScript = false;
		HandleHighestPriorityEncounter( false );
	}

	if( GetGameState() == ex_game_s::NONE && m_QueuedWorldMapTransition.IsNotNull() )
	{
		if( m_QueuedWorldMapTransition != GetCurrentMapId() )
		{
			m_TransitTarget = exTransitTarget( CT_Clear );
			m_TransitTarget.TargetMapId = m_QueuedWorldMapTransition;
			m_TransitTarget.TargetAutoMarker = gcx_marker_t::EXIT;
			SetGameState( ex_game_s::TRANS_MAP );
		}

		m_QueuedWorldMapTransition = CT_Clear;
	}

	if( GetGameState() == ex_game_s::TRANS_MAP )
	{
		SetGameState( ex_game_s::TRANS_LOADING );
		SDK_RunClientEvent( eg_crc( "ShowFullScreenLoading" ) ); // Call this before the load, that way the client starts the loading screen before the map load.
		AttemptAutoSave( true ); // Save right before leaving map
		if( m_TownPortalRecallMarker.bClearTownPortalAfterSave )
		{
			m_TownPortalRecallMarker.bClearTownPortalAfterSave = false;
			m_TownPortalRecallMarker.bRecallWasCast = true;
			SmSetVar( eg_crc("LloydsBeaconOpenControl") , 0 );
		}
		LoadMap( m_TransitTarget.TargetMapId );
		return;
	}

	if( GetGameState() == ex_game_s::DEAD )
	{
		SetGameState( ex_game_s::NONE );
		SDK_LoadLevel( "" ); // This will unload...
		SDK_RunClientEvent( eg_crc( "OnReturnToMainMenu" ) );
		return;
	}

	egLockstepCmds Cmds;
	SDK_GetCommandsForPlayer( eg_lockstep_id::IndexToId( 0 ), &Cmds );

	if( GetGameState() == ex_game_s::RESTING )
	{
		assert( m_RestHoursLeft > 0 );

		eg_int PartyFood = SmGetVar( eg_crc("PartyFood") ).as_int();

		m_RestTimer += DeltaTime;
		if( m_RestTimer >= ExGlobalData::Get().GetRestTimePerHour() )
		{
			m_RestTimer = 0.f;
			m_RestHoursLeft--;
			m_RestHoursUntilAmbush--;
			m_Time.AddHours( 1 );
			SDK_ReplicateToClient( &m_Time , sizeof(m_Time) );
			for( eg_uint i=0; i<ExRoster::PARTY_SIZE; i++ )
			{
				ExFighter* PartyMember = GetPartyMemberByIndex( i );
				if( PartyMember && PartyFood > 0 )
				{
					PartyMember->ResolveReplicatedData();
					if( !PartyMember->IsDead() )
					{
						PartyMember->ApplyRawHeal( m_PartyMemberRestHPPerHour[i] );
						PartyMember->ApplyManaRestoration( m_PartyMemberRestMPPerHour[i] );
					}
				}
			}
		}

		if( m_bCombatEncounterAfterRest && m_RestHoursUntilAmbush == 0 )
		{
			if( PartyFood > 0 )
			{
				SmSetVar( eg_crc("PartyFood") , PartyFood - 1 );
			}
			m_bIgnoreTreasureUntilNextTurn = false;
			SetGameState( ex_game_s::NONE );
			BeginRandomCombat();
		}
		else if( m_RestHoursLeft == 0 )
		{
			for( eg_uint i=0; i<ExRoster::PARTY_SIZE; i++ )
			{
				ExFighter* PartyMember = GetPartyMemberByIndex( i );
				if( PartyMember && PartyFood > 0 && !PartyMember->IsDead() )
				{
					PartyMember->RestoreAllHP();
					PartyMember->RestoreAllMP();
				}
			}

			if( PartyFood > 0 )
			{
				SmSetVar( eg_crc("PartyFood") , PartyFood - 1 );
			}

			SetGameState( ex_game_s::NONE );
		}
	}

	if( EX_CHEATS_ENABLED && Cmds.WasPressed( CMDA_QSAVE ) && ExGameSettings_DebugSaveAllowed.GetValueThreadSafe() )
	{
		m_bSaveOnNextUpdate = true;
	}

	if( GetGameState() == ex_game_s::NONE && (PlayerEnt == nullptr || PlayerEnt->IsIdle()) )
	{		
		if( m_bSaveOnNextUpdate && PlayerEnt && PlayerEnt->IsIdle() && m_EntWorldOwner.IsValid() && !m_EntWorldOwner->IsLoadingWorlds() )
		{
			assert( m_PlayerId != INVALID_ID );
			m_bSaveOnNextUpdate = false;
			SaveGame();
		}

		if( m_bRestOnNextUpdate )
		{
			m_bRestOnNextUpdate = false;
			BeginResting();
			return;
		}

		if( EX_CHEATS_ENABLED && Cmds.WasPressed( CMDA_QLOAD ) && ExGameSettings_DebugSaveAllowed.GetValueThreadSafe() )
		{
			SDK_RunClientEvent( eg_crc( "ShowFullScreenLoading" ) ); // Call this before the load, that way the client starts the loading screen before the map load.
			SDK_LoadLevel( *ExSaveMgr::GetSaveSlotFilename( m_SaveSlot ) , true );
		}

		if( Cmds.WasMenuPressed( CMDA_GAMEMENU ) )
		{
			SDK_RunClientEvent( eg_crc( "OpenPauseMenu" ) );
			// Probably should set a flag stating we are waiting for the game menu to end.
		}

		if( Cmds.WasMenuPressed( CMDA_QUESTLOG ) )
		{
			BeginStateMachine( eg_crc("MenuLib") , eg_crc("OpenQuestLog") );
		}

		if( Cmds.WasMenuPressed( CMDA_MAPMENU ) )
		{
			BeginStateMachine( eg_crc("MenuLib") , eg_crc("OpenMapMenu") );
		}

		if( Cmds.WasMenuPressed( CMDA_QUICK_INVENTORY ) )
		{
			BeginStateMachine( eg_crc("MenuLib") , eg_crc("OpenInventory") );
		}

		if( Cmds.WasMenuPressed( CMDA_CONTEXTMENU ) )
		{
			BeginStateMachine( eg_crc("MenuLib") , eg_crc("OpenContextMenu") );
		}

		if( Cmds.WasMenuPressed( CMDA_VIEWCHARMENU ) )
		{
			BeginStateMachine( eg_crc("MenuLib") , eg_crc("OpenViewCharacterMenu") );
		}

		// DEBUG ONLY
		if( EX_CHEATS_ENABLED )
		{
			if( Cmds.WasPressedOrRepeated( CMDA_DEBUG_NEXT ) )
			{
				if( Cmds.IsActive( CMDA_KBMENU_BKSP ) )
				{
					m_Time.AddDays( 1 );
				}
				else if( Cmds.IsActive( CMDA_DEBUG_SPEED ) )
				{
					m_Time.AddHours( 1 );
				}
				else
				{
					m_Time.AddSeconds( 1 );
				}
				SDK_ReplicateToClient( &m_Time , sizeof(m_Time) );
			}
			if( Cmds.WasPressedOrRepeated( CMDA_DEBUG_PREV ) )
			{
				if( Cmds.IsActive( CMDA_KBMENU_BKSP ) )
				{
					m_Time.AddDays( -1 );
				}
				else if( Cmds.IsActive( CMDA_DEBUG_SPEED ) )
				{
					m_Time.AddHours( -1 );
				}
				else
				{
					m_Time.AddSeconds( -1 );
				}
				SDK_ReplicateToClient( &m_Time , sizeof(m_Time) );
			}
		}
		// DEBUG ONLY
	}

	if( GetGameState() == ex_game_s::MOVING || GetGameState() == ex_game_s::NONE )
	{
		if( Cmds.WasPressed( CMDA_GAME_NEXTPMEMBER ) || Cmds.WasPressed( CMDA_GAME_NEXTPMEMBER2 ) || Cmds.WasPressed( CMDA_GAME_PREVPMEMBER ) )
		{
			// This weird loop is basically in case a party member slot is empty:
			const eg_int Direction = Cmds.WasPressed( CMDA_GAME_PREVPMEMBER ) ? -1 : 1;
			for( eg_uint i = 0; i<ExRoster::PARTY_SIZE; i++ )
			{
				const eg_int Offset = Direction*(i+1);
				const eg_int PartyIndex = ( ExRoster::PARTY_SIZE + m_PartyStatus.SelectedPartyMember + Offset ) % ExRoster::PARTY_SIZE;
				if( GetPartyMemberByIndex( PartyIndex ) )
				{
					BhSetSelectedPartyMember( PartyIndex );
					SDK_RunClientEvent( eg_crc("ClientHandlePartyMemberSwitched") );
					break;
				}
			}
			// Party status gets replicated whenever it changes so we don't need to explicitly replicate it here.
		}
	}

	if( GetGameState() == ex_game_s::MOVING )
	{
		m_TurnTime += DeltaTime;
	}

	//DebugText_MsgOverlay_AddText( DEBUG_TEXT_OVERLAY_SERVER , EGString_Format( "Turn Progress: %.4f" , m_TurnTime ) );
	//DebugText_MsgOverlay_AddText( DEBUG_TEXT_OVERLAY_SERVER , EGString_Format( "Monster Count: %u" , m_MonsterList.Len() ) );

	// Replicate an changed data.

	m_Roster.ReplicateDirtyData( this );

	if( m_SeedDay != m_Time.Day )
	{
		GeneratedNewSeeds();
	}

	eg_string_crc NewChecksum = eg_string_crc::HashData( &m_PartyStatus, sizeof( m_PartyStatus ) );
	if( NewChecksum != m_PartyStatusChecksum )
	{
		m_PartyStatusChecksum = NewChecksum;
		SDK_ReplicateToClient( &m_PartyStatus, sizeof( m_PartyStatus ) );
	}
}

void ExGame::Update_AsClient( eg_real DeltaTime )
{
	unused( DeltaTime );

	m_TimeSmoother.Update( DeltaTime );

	if( m_CameraMode == ex_camera_mode::CameraSpline )
	{
		m_CameraSplineRepPose.Smooth( DeltaTime , m_EntWorldOwner ? m_EntWorldOwner->GetEntUpdateRate() : 0.f );
	}

	CheckForGameMenuClosed();
}

void ExGame::FormatText( eg_cpstr Flags, class EGTextParmFormatter* Formatter ) const
{
	eg_string_crc FormatType = Formatter->GetNextFlag( &Flags );

	switch_crc( FormatType )
	{
		case_crc("DATE"):
		{
			Formatter->SetText( EGFormat( L"{0}" , &ExTimeFormatter( &m_Time , Flags ) ).GetString() );
		} break;
		case_crc("ITEMNAME"):
		{
			exInventoryItem NewItem;
			NewItem.RawItem.ItemId = Formatter->GetNextFlag( &Flags );
			NewItem.RawItem.Level = Formatter->GetNextFlagAsInt( &Flags );
			NewItem.ResolvePointers();
			Formatter->SetText( EGFormat( L"{0:NAME}" , &NewItem ).GetString() );
		} break;
		case_crc("QUESTITEMNAME"):
		{
			exQuestItemInfo ItemInfo = ExQuestItems::Get().GetItemInfo( Formatter->GetNextFlag( &Flags ) );
			Formatter->SetText( EGFormat( L"{0:NAME}" , &ItemInfo ).GetString() );
		} break;
		case_crc("MAP"):
		{
			const eg_string_crc MapProp = Formatter->GetNextFlag( &Flags );
			if( MapProp == eg_crc("NAME") )
			{
				Formatter->SetText( eg_loc_text(GetCurrentMapInfo().NameCrc).GetString() );
			}
		} break;
		case_crc("VARS"):
		{
			eg_string_crc VarName = Formatter->GetNextFlag( &Flags );
			if( m_GameStateVars.Contains( VarName ) )
			{
				const egsm_var& Var = m_GameStateVars.Get( VarName );
				egsm_var_t VarType = EGSmMgr_GetVarType( VarName );
				switch( VarType )
				{
				case egsm_var_t::UNK:
				case egsm_var_t::RETURN_VOID:
				case egsm_var_t::TERMINAL:
				case egsm_var_t::NUMBER:
					Formatter->SetText( L"(UNK)" );
					break;
				case egsm_var_t::INT:
					Formatter->SetText( EGFormat( L"{0}" , Var.as_int() ).GetString() );
					break;
				case egsm_var_t::REAL:
					Formatter->SetText( EGFormat( L"{0}" , Var.as_real() ).GetString() );
					break;
				case egsm_var_t::BOOL:
					Formatter->SetText( EGFormat(L"{0}" , Var.as_bool() ).GetString() );
					break;
				case egsm_var_t::CRC:
					Formatter->SetText( EGFormat( L"{0}" , Var.as_int() ).GetString() );
					break;
				case egsm_var_t::LOC_TEXT:
					Formatter->SetText( EGFormat( Var.as_crc() ).GetString() );
					break;
				}
			}
		} break;
	}
}

void ExGame::CalmHostileEnemies()
{
	for( exTrackedIoEnt& TrackedIo : m_TrackedIoEnts )
	{
		ExEnemyController* EnemyEnt = EGCast<ExEnemyController>( TrackedIo.IoEnt.GetObject() );
		if( EnemyEnt )
		{
			EnemyEnt->CalmHostility();
		}
	}
}

void ExGame::MovePlayerToEscapeSquare()
{
	assert( GetGameState() == ex_game_s::NONE );

	// Find the transition square we are going to
	eg_bool bHasSafeSquare = false;
	eg_bool bHasTownPortal = false;
	eg_bool bHasAnyEntrance = false;
	exMapTransitionEnt WantedEntrance;

	for( const exMapTransitionEnt& Trans : m_MapTransitionEnts )
	{
		if( Trans.Ent )
		{
			if( Trans.Ent->GetId() == eg_crc("SafeLocation") )
			{
				bHasSafeSquare = true;
				bHasAnyEntrance = true;
				WantedEntrance = Trans;
			}
			if( Trans.Ent->GetId() == eg_crc("TownPortal") )
			{
				if( !bHasSafeSquare )
				{
					bHasTownPortal = true;
					bHasAnyEntrance = true;
					WantedEntrance = Trans;
				}
			}
			if( !bHasSafeSquare && !bHasTownPortal )
			{
				bHasAnyEntrance = true;
				WantedEntrance = Trans;	
			}
		}
	}

	eg_transform EscapeToLocation( CT_Default );

	if( bHasAnyEntrance )
	{
		EscapeToLocation = FaceToTransform( WantedEntrance.SpawnFace );
		EscapeToLocation.TranslateThis( (WantedEntrance.NavVertex.GetPosition() - eg_vec4(0,0,0,1)).ToVec3() );
		ExPlayerController* Player = GetPlayerEnt();
		if( Player )
		{
			Player->TeleporTo( EscapeToLocation );
		}
	}
}

ExHUD* ExGame::GetHUD()
{
	ExHUD* Out = nullptr;
	assert( IsClient() );
	if( IsClient() )
	{
		ExClient* Client = EGCast<ExClient>(GetClientOwner());
		if( Client )
		{
			Out = Client->GetHUD();
		}
	}
	return Out;
}

void ExGame::SpawnClients()
{
	assert( m_ClientPCs.IsEmpty() );

	for( const eg_lockstep_id& ClientId : m_ClientsInWorld )
	{
		eg_bool bMainController = m_ClientPCs.IsEmpty();
		EGEnt* SpawnedEnt = m_EntWorldOwner->SpawnEnt( bMainController ? eg_crc( "PlayerController" ) : eg_crc( "FreeMovePlayerController" ), CT_Default );
		ExEnt* SpawnedPC = EGCast<ExEnt>( SpawnedEnt );
		if( SpawnedPC )
		{
			SpawnedPC->AssignToPlayer( ClientId );
			m_ClientPCs.Append( SpawnedPC );
		}
	}
}

void ExGame::ClientCreateRosterCharacter( eg_uint RosterIndex, const ExFighter& CreateCharacter )
{
	assert( IsClient() );
	m_Roster.ClientCreateRosterCharacter( this , RosterIndex , CreateCharacter );
}

void ExGame::ClientUpdateRosterCharacterByPartyIndex( eg_uint PartyIndex, const ExFighter& Character )
{
	assert( IsClient() );
	m_Roster.ClientUpdateRosterCharacterByPartyIndex( this , PartyIndex , Character );
}

void ExGame::CheckForGameMenuClosed()
{
	if( m_GameMenu.IsStale() )
	{
		m_GameMenu = nullptr;
		assert( !m_GameMenu.IsStale() );
		SDK_RunServerEvent( eg_crc( "OnGameMenuClosed" ) );
	}
}

void ExGame::SmOnError( eg_cpstr ErrorString )
{
	EGLogf( eg_log_t::Error , __FUNCTION__ ": Script Failure: %s" , ErrorString );
	assert( false );
}

egsm_var ExGame::SmGetVar( eg_string_crc VarId ) const
{
	return GetGameVar( VarId );
}

eg_bool ExGame::SmDoesVarExist( eg_string_crc VarName ) const
{
	return m_GameStateVars.Contains( VarName );
}

void ExGame::SmSetVar( eg_string_crc VarId, const egsm_var& Value )
{
	assert( IsServer() );
	
	if( m_GameStateVars.Contains( VarId ) )
	{
		egsm_var& Var = GetGameVar( VarId );
		egsm_var OldValue = Var;
		Var.SetToButPreserveType( Value );
		if( OldValue != Var )
		{
			OnServerGameVarChanged.Broadcast( VarId , Var );
		}
		SDK_ReplicateToClient( &Var , sizeof(Var) );
	}
	else
	{
		assert( false ); // Tried to set a var that didn't exist.
	}
}

ISmRuntimeHandler::egsmNativeRes ExGame::SmOnNativeEvent( eg_string_crc EventName, const EGArray<egsm_var>& Parms, const EGArray<eg_string_crc>& Branches )
{
	assert( m_MapScriptYieldReason == ex_script_yield_reason::NONE ); // Should not be yielding during an event.

	switch_crc( EventName )
	{
		case_crc("QuitFromInn"):
		{
			if( m_bMapScriptInScriptMenu )
			{
				SDK_RunClientEvent( eg_crc("CloseScriptMenu") );
				m_bMapScriptInScriptMenu = false;
			}
			NativeEventQuitFromInn();
		} return egsmTerminal();
		case_crc("NpcDialog"):
		{
			m_CurConvState.CurDialog = Parms[0].as_crc();
			m_CurConvState.Choices.Clear();
			m_CurConvState.Choices.Append( Branches.GetArray() , Branches.Len() );
			m_MapScriptYieldReason = ex_script_yield_reason::CONVERSATION_CHOICE;
			return egsmYield();
		} break;
		case_crc("NpcGossip"):
		{
			EGRandom GossipRng( m_NpcGossipSeed ); // NPCs say the same thing all day long so the dialog is only different when the seed changes.
			exNpcGossipDialog NpcDialog = ExNpcGossip::Get().GetRandomDialogForNpc( Parms[0].as_crc() , GossipRng );

			m_CurConvState.CurDialog = NpcDialog.CharacterDialog.Text;
			m_CurConvState.Choices.Clear();
			m_CurConvState.Choices.Append( NpcDialog.PlayerResponse.Text );
			m_MapScriptYieldReason = ex_script_yield_reason::NPC_GOSSIP;
			return egsmYield();
		} break;
		case_crc("NpcClearDialog"):
		{
			bool bLeaveOverlayOpen = SmResolveParm(Parms[0]).as_bool();
			SDK_RunClientEvent( eg_crc("OnConversationClear") , bLeaveOverlayOpen );
			if( !bLeaveOverlayOpen )
			{
				m_bMapScriptIsInConversation = false;
			}
			return egsmDefault();
		} break;
		case_crc("NpcSetSpeakerNameplate"):
		{
			m_CurConvState.Speaker = Parms[0].as_crc();
			SDK_ReplicateToClient( &m_CurConvState , sizeof(m_CurConvState) );
			SDK_RunClientEvent( eg_crc("RefreshConversationNameplate") );
			return egsmDefault();
		} break;
		case_crc("ShowDialogBox"):
		{
			m_CurYieldForMenuState.Speaker = eg_crc("txtGameTitleShort");
			m_CurYieldForMenuState.CurDialog = Parms[0].as_crc();
			m_CurYieldForMenuState.Choices.Clear();
			m_CurYieldForMenuState.Choices.Append( Branches.GetArray() , Branches.Len() );
			m_bMapScriptIsInDialogBox = true;
			SDK_ReplicateToClient( &m_CurYieldForMenuState , sizeof(m_CurYieldForMenuState) );
			SDK_RunClientEvent( eg_crc("OnScriptDialogBox") );
			m_MapScriptYieldReason = ex_script_yield_reason::MENU_RESPONSE;
			return egsmYield();
		} break;
		case_crc("ShowGameMenu"):
		{
			m_MapScriptYieldReason = ex_script_yield_reason::GAME_MENU;
			SDK_RunClientEvent( eg_crc("OpenGameMenu") , Parms[0].as_crc() );
			return egsmYield();
		} break;
		case_crc("TransitionToEndGame"):
		{
			SDK_RunClientEvent( eg_crc("ClientNotifyEvent") , eg_crc("EndGame") );
		} return egsmTerminal();
		case_crc("TransitionToMap"):
		{
			if( Parms.Len() >=2 )
			{
				m_TransitTarget = exTransitTarget( CT_Clear );
				m_TransitTarget.TargetMapId = Parms[0].as_crc();
				m_TransitTarget.TargetEntranceId = Parms[1].as_crc();
				SetGameState( ex_game_s::TRANS_MAP );
			}
			else
			{
				assert( false ); // Invalid call to TransitionToMap
			}

			return egsmTerminal();
		} break;
		case_crc("DoAutoTransition"):
		{
			// Auto transition stuff should have been set up already.
			SetGameState( ex_game_s::TRANS_MAP );
			return egsmTerminal();
		} break;
		case_crc("SaveFromInn"):
		{
			BhSaveFromInn( eg_event_parms( CT_Clear ) );
			return egsmDefault();
		} break;
		case_crc("SpawnCombatOnPlayer"):
		{
			eg_string_crc EncounterId = Parms.Len() >= 1 ? Parms[0].as_crc() : CT_Clear;

			ExEnt* Player = EGCast<ExEnt>( SDK_GetEnt( m_PlayerId ) );
			if( Player )
			{
				const eg_uint32 EncounterIdNum = EncounterId.ToUint32();
				const eg_d_string EncounterIdStr = EGSFormat8( "{0}" , EncounterIdNum );
				ExEnemyController* NewEnemy = EGCast<ExEnemyController>(SDK_SpawnEnt( eg_crc("ENT_InWorldEnemy") , Player->GetPose() , *EncounterIdStr ));
				if( NewEnemy )
				{
					NewEnemy->SetEncounterId( EncounterId );
					m_bHandleEncountersAfterScript = true;
				}
			}
		} return egsmDefault();
		case_crc("SetupCostToHeal"):
		{
			SmSetVar( eg_crc("NextCostToHeal") , CalculateCostToHealParty() );
		} return egsmDefault();
		case_crc("HealParty"):
		{
			for( eg_int i=0; i<ExRoster::PARTY_SIZE; i++ )
			{
				ExFighter* PartyMember = m_Roster.GetPartyMemberByIndex( i );
				if( PartyMember )
				{
					PartyMember->SetDead( false );
					PartyMember->SetUnconcious( false );
					PartyMember->SetPetrified( false );
					PartyMember->SetCursed( false );
					PartyMember->ResolveReplicatedData();
					PartyMember->RestoreAllHP();
					PartyMember->RestoreAllMP();
				}
			}
			m_Roster.SetBroadcastRosterUpdateOnNextReplication();
			m_Roster.ReplicateDirtyData( this );
			SDK_RunClientEvent( eg_crc("ClientNotifyEvent") , eg_crc("HealParty") );
		} return egsmDefault();
		case_crc("ShowTranslator"):
		{
			const eg_string_crc TranslatorText = Parms.Len() >= 1 ? Parms[0].as_crc() : CT_Clear;
			SetActiveTranslationText( TranslatorText );
		} return egsmDefault();
		case_crc("UnlockSkill"):
		{
			const eg_string_crc SkillId = Parms.Len() >= 1 ? Parms[0].as_crc() : CT_Clear;
			SetSkillUnlocked( SkillId , true );
		} return egsmDefault();
		case_crc("IgnoreScriptEncounter"):
		{
			m_bIgnoreScriptEncounter = true;
		} return egsmDefault();
		case_crc("ResumeEncounters"):
		{
			m_bHandleEncountersAfterScript = true; // Handle with care in theory could create an infinite loop.
		} return egsmDefault();

		case_crc("SetEndingMovieUnlocked"):
		{
			eg_bool bUnlocked = Parms.Len() >= 1 && Parms[0].as_bool();
			SDK_RunClientEvent( eg_crc("ClientNotifyEvent") , bUnlocked ? eg_crc("UnlockEndingMovie") : eg_crc("LockEndingMovie") );
		} return egsmDefault();
		case_crc("SetCastleGameUnlocked"):
		{
			eg_bool bUnlocked = Parms.Len() >= 1 && Parms[0].as_bool();
			SDK_RunClientEvent( eg_crc("ClientNotifyEvent") , bUnlocked ? eg_crc("UnlockCastleGame") : eg_crc("LockCastleGame") );
		} return egsmDefault();
		case_crc("UnlockAchievement"):
		{
			const eg_string_crc AchId = Parms.Len() >= 1 ? Parms[0].as_crc() : CT_Clear;
			if( AchId.IsNotNull() )
			{
				ExAchievements::Get().UnlockAchievement( AchId );
			}
		} return egsmDefault();
		case_crc("DoTownPortal"):
		{
			m_bRecallTownPortalQueued = true;
		} return egsmTerminal();
		case_crc("IsInventoryFull"):
		{
		} return egsmBranch( m_Roster.GetInventory().IsBagFull() ? eg_crc("TRUE") : eg_crc("FALSE") );
		case_crc("GiveItem"):
		{
			assert( !m_Roster.GetInventory().IsBagFull() );
			if( !m_Roster.GetInventory().IsBagFull() )
			{
				exInventoryItem NewItem;
				NewItem.RawItem.ItemId = Parms.Len() >= 1 ? Parms[0].as_crc() : CT_Clear;
				NewItem.RawItem.Level = Parms.Len() >= 2 ? Parms[1].as_int() : 1;
				NewItem.ResolvePointers();
				ExInventory& Inventory = m_Roster.GetInventory();
				Inventory.AddItem( NewItem );
				m_Roster.SetBroadcastRosterUpdateOnNextReplication();
			}
		} return egsmDefault();
		case_crc("PlaySound"):
		{
			SDK_RunClientEvent( eg_crc("PlaySound") , Parms[0].as_crc() );
		} return egsmDefault();
	}

	assert( false ); // Event not handled.
	return egsmNotHandled();
}

const ExFighter* ExGame::GetBestThiefOrSelectedPartyMember() const
{
	const ExFighter* Out = GetPartyMemberByIndex( GetSelectedPartyMemberIndex() );
	for( eg_uint i=0; i<ExRoster::PARTY_SIZE; i++ )
	{
		const ExFighter* PartyMember = m_Roster.GetPartyMemberByIndex( i );
		if( PartyMember && PartyMember->GetClass() == ex_class_t::Thief )
		{
			// If we find a higher level thief use it:
			if( Out == nullptr || Out->GetClass() != ex_class_t::Thief || PartyMember->GetAttrValue( ex_attr_t::LVL ) > Out->GetAttrValue( ex_attr_t::LVL ) )
			{
				Out = PartyMember;
			}
		}
	}
	return Out;
}

const ExFighter* ExGame::GetSelectedPartyMember() const
{
	return m_Roster.GetPartyMemberByIndex( m_PartyStatus.SelectedPartyMember );
}

eg_real ExGame::GetNormalizedTimeOfDay() const
{
	return m_TimeSmoother.GetSmoothTimeOfDayNormalized();
}

ExEnt* ExGame::GetEnt( eg_ent_id Id )
{
	return EGCast<ExEnt>( SDK_GetEnt( Id ) );
}

const ExEnt* ExGame::GetEnt( eg_ent_id Id ) const
{
	return EGCast<ExEnt>( SDK_GetEnt( Id ) );
}

ExPlayerController* ExGame::GetPlayerEnt()
{
	return EGCast<ExPlayerController>( GetEnt( GetPrimaryPlayer() ) );
}

const ExPlayerController* ExGame::GetPlayerEnt() const
{
	return EGCast<ExPlayerController>( GetEnt( GetPrimaryPlayer() ) );
}

void ExGame::SetSkillUnlocked( eg_string_crc SkillId, eg_bool bUnlocked )
{
	assert( IsServer() );
	if( m_Skills )
	{
		m_Skills->SetSkillUnlocked( SkillId , bUnlocked );
	}
	SDK_RunClientEvent( eg_crc("ReplicateUnlockedSkill") ,  exSkillRepInfo( SkillId , bUnlocked ) );
}

void ExGame::SetPlayerTargetedEnt( eg_ent_id NewTarget )
{
	if( m_PlayerTargetedEnt != NewTarget )
	{
		m_PlayerTargetedEnt = NewTarget;
		SDK_ReplicateToClient( &m_PlayerTargetedEnt , sizeof(m_PlayerTargetedEnt) );
	}
}

eg_int ExGame::GetMinFrontLineSize() const
{
	return ExGlobalData::Get().GetCombatPlayerTeamFrontLineMinSize();
}

eg_int ExGame::GetEnemyFrontLineSize() const
{
	return ExGlobalData::Get().GetCombatEnemyTeamTeamFrontLineSize();
}

eg_int ExGame::ComputeNumVisitableSquares() const
{
	eg_int CountOut = 0;

	EGNavGraphVertex StartVertex = GetPlayerEnt() ? GetPlayerEnt()->GetMapVertex() : EGNavGraphVertex();

	EGQueue<EGNavGraphVertex,1024> Queue;
	EGArray<EGNavGraphVertex> Visited;
	Queue.EnQueue( StartVertex );

	while( Queue.HasItems() )
	{
		EGNavGraphVertex Item = Queue.Front();
		Queue.DeQueue();
		if( Visited.Contains( Item ) )
		{
			continue;
		}

		CountOut++;
		Visited.Append( Item );
		for( eg_uint i=0; i<Item.GetNumEdges(); i++ )
		{
			Queue.EnQueue( Item.GetEdge( this , i ) );
		}
	}

	return CountOut;
}

void ExGame::QueueSpawn( eg_string_crc Type, const eg_transform& Pose, eg_cpstr InitString )
{
	assert( IsServer() );
	SDK_SpawnEnt( Type , Pose , InitString );
}

void ExGame::OnSaveGameLoaded()
{
	Super::OnSaveGameLoaded();

	assert( IsServer() );

	assert( GetGameState() == ex_game_s::NONE || GetGameState() == ex_game_s::TRANS_LOADING || GetGameState() == ex_game_s::CREATED_NEW_GAME ); // We should not have loaded a map if a turn was in progress.
	m_TurnTime = 0.f;
	m_PartyStatusChecksum = eg_crc("");
	m_bHasPropperSpawnLocation = false;

	if( m_bNeedsInitialSave )
	{
		m_bNeedsInitialSave = false;
		m_bDontShowPopupForNextSave = true;
		m_bSaveOnNextUpdate = true;
	}

	// Always need to get nav graph since it may be different from map to map.
	m_NavGraph = SDK_GetNavGraph( eg_nav_graph_id( eg_crc( "nav" ) ) );
	assert( m_NavGraph.IsValid() ); // This map isn't going to work without a nav

	m_MonsterList.Clear(); // Reset the monster list, because even if we are loading a save the monsters themselves will populate the list when they spawn.
	assert( m_TrackedIoEnts.IsEmpty() ); // ExIoEnt should be managing this...
	m_TrackedIoEnts.Clear();

	eg_string_crc MapScriptId = eg_string_crc(*GetCurrentMapInfo().ScriptId);

	m_SafeSpots.Clear();
	//
	// Process map tags
	//

	EGArray<egMapTag> MapTags;
	SDK_GetMapTags( MapTags );
	for( const egMapTag& MapTag : MapTags )
	{
		if( MapTag.Type == eg_crc("monster") )
		{
			if( !m_bIsSave )
			{
				eg_string_crc EncounterId = GetCurrentMapInfo().EncounterData.Fixed;
				eg_bool bIsBoss = false;
				eg_d_string EncodedInfo = EGParse_GetAttValue( MapTag.Attrs , "info" );
				if( EncodedInfo.Len() > 0 )
				{
					EGArray<eg_byte> DecodedData = EGBase64_Decode( EncodedInfo );
					// DecodedData.Append( '\0' );

					EGParse_ProcessFnCallScript( reinterpret_cast<eg_char8*>(DecodedData.GetArray()) , DecodedData.Len() , 
						[this,&bIsBoss,&EncounterId]( egParseFuncInfo& ParseInfo ) -> void
						{
							if( EGString_EqualsI( ParseInfo.FunctionName , "Boss" ) )
							{
								EncounterId = GetCurrentMapInfo().EncounterData.Boss;
								bIsBoss = true;
						}
						} );
				}

				ExEnemyController* NewMonster = EGCast<ExEnemyController>(SDK_SpawnEnt( eg_crc("ENT_InWorldEnemy") , MapTag.Pose ) );
				assert( NewMonster != nullptr ); // What was this...
				if( NewMonster )
				{
					NewMonster->SetEncounterId( EncounterId );
					NewMonster->SetIsBoss( true ); // Treat all placed monsters as bosses so they don't come back (fixes city guard issue).

					if( NewMonster->IsBoss() )
					{
						exDefeatedBoss DefeatedBossInfo;
						DefeatedBossInfo.MapId = m_CurrentMapId;
						DefeatedBossInfo.NavVertex = NewMonster->GetBossVertex();
						if( m_DefeatedBossList.Contains( DefeatedBossInfo ) )
						{
							NewMonster->DeleteFromWorld();
						}
					}
				}
			}
		}
		else if( MapTag.Type == eg_crc("FUNCTION") )
		{
			eg_d_string EncodedFunction = EGParse_GetAttValue( MapTag.Attrs , "info" );
			EGArray<eg_byte> DecodedData = EGBase64_Decode( EncodedFunction );
			DecodedData.Append( '\0' );

			egParseFuncInfo ParseInfo;
			EGPARSE_RESULT ParseRes = EGParse_ParseFunction( reinterpret_cast<const eg_char8*>(DecodedData.GetArray()) , &ParseInfo );
			if( EGPARSE_OKAY == ParseRes )
			{
				eg_uint ClosestVertexIdx = m_NavGraph.GetVertexIndexClosestTo( this , MapTag.Pose.GetPosition() );
				EGNavGraphVertex ClosestVertex( CT_Clear );
				eg_bool bGotVertex = m_NavGraph.GetVertex( this , ClosestVertexIdx , &ClosestVertex );
				assert( bGotVertex ); // Couldn't associate this function call with a vertex...

				EGLogf( eg_log_t::GameLib , __FUNCTION__ ": Processing %s..." , ParseInfo.FunctionName );

				eg_string_crc CrcFunction = eg_string_crc(ParseInfo.FunctionName);

				switch_crc( CrcFunction )
				{
					case_crc("SetSafeSpot"):
					{
						m_SafeSpots.Append( ClosestVertex );
					} break;
					default:
					{
						EGLogf( eg_log_t::GameLib , __FUNCTION__ ": %s not recognized. Function did nothing." , ParseInfo.FunctionName );
						assert( false );
					} break;
				}
			}
			else
			{
				EGLogf( eg_log_t::GameLib , __FUNCTION__ ": The map could not process %s" , DecodedData );
			}
		}
	}

	// Decide where the player should spawn
	eg_bool bHasSpawnLocation = false;

	if( GetGameState() == ex_game_s::TRANS_LOADING || GetGameState() == ex_game_s::CREATED_NEW_GAME )
	{
		SetGameState( ex_game_s::NONE );
		m_SafeTurns = 0;

		eg_transform FallbackSpawnLocation = CT_Default;

		for( const exMapTransitionEnt& TransEnt : m_MapTransitionEnts )
		{
			if( TransEnt.Ent && m_TransitTarget.TargetEntranceId == TransEnt.Ent->GetId() )
			{
				m_PlayerSpawnLocation = FaceToTransform( TransEnt.SpawnFace );
				m_PlayerSpawnLocation.TranslateThis( TransEnt.NavVertex.GetPosition().ToVec3() );
				bHasSpawnLocation = true;
			}
		}

		if( m_TransitTarget.bIsTownPortalRecall && m_TownPortalRecallMarker.TargetMapId == m_TransitTarget.TargetMapId )
		{
			m_PlayerSpawnLocation = m_TownPortalRecallMarker.TargetPose;
			bHasSpawnLocation = true;
			AttemptAutoSave( false ); // Attempt to save upon entering a map.
		}
	}
	else if( m_bIsSave )
	{
		// m_PlayerSpawnLocation should be in the save.
		bHasSpawnLocation = true;
	}

	m_bHasPropperSpawnLocation = bHasSpawnLocation;

	if( !bHasSpawnLocation )
	{
		m_PlayerSpawnLocation = eg_transform( CT_Default );
	}

	if( m_Mapping )
	{
		m_Mapping->SetMap( m_CurrentMapId );
	}

	if( m_bIsSave )
	{
		m_bIsSave = false;
	}
}

void ExGame::OnMapLoaded( const EGGameMap* GameMap )
{
	Super::OnMapLoaded( GameMap );

	if( IsServer() && (GetGameState() == ex_game_s::TRANS_LOADING || GetGameState() == ex_game_s::CREATED_NEW_GAME) )
	{
		OnSaveGameLoaded();

		m_PartyStatusChecksum = eg_crc( "" ); // Reset the replicated data so that the new client gets it.
		SDK_RunClientEvent( eg_crc( "RefreshHUD" ) );

		if( m_EntWorldOwner )
		{
			EGNavGraph NavGraph = SDK_GetNavGraph( eg_nav_graph_id( eg_crc( "nav" ) ) );

			if( m_bHasPropperSpawnLocation )
			{
				eg_transform SpawnPose = m_PlayerSpawnLocation;

				ExPlayerController* Player = GetPlayerEnt();
				if( Player )
				{
					Player->RestablishNavGraph( SpawnPose );
				}
			}
		}

		// Re-seed randomness
		m_Rng.ReSeed( m_Rng.CreateSeed() );
		m_bHadAmbushOnLastRest = false;
	}
}

void ExGame::PostSave( EGFileData& GameDataOut )
{
	Super::PostSave( GameDataOut );

	if( m_Skills )
	{
		m_Skills->SaveTo( GameDataOut );
	}

	if( m_Mapping )
	{
		m_Mapping->SaveTo( GameDataOut );
	}
}

void ExGame::PostLoad( const EGFileData& GameDataIn )
{
	Super::PostLoad( GameDataIn );

	if( m_Skills )
	{
		m_Skills->LoadFrom( GameDataIn );
	}

	if( m_Mapping )
	{
		m_Mapping->LoadFrom( GameDataIn );
		m_Mapping->SetMap( m_CurrentMapId );
	}
}

void ExGame::OnDataReplicated( const void* Offset , eg_size_t Size )
{
	eg_cpstr ThingStr = nullptr;

	auto HandleCase = [&ThingStr,&Offset,&Size]( auto& Var , eg_cpstr Name ) -> eg_bool
	{
		if( EGMem_Contains( &Var , sizeof(Var) , Offset , Size ) )
		{
			assert( nullptr == ThingStr );
			ThingStr = Name;
			return true;
		}

		return false;
	};

	HandleCase( m_Roster , "Roster" );
	HandleCase( m_GameStateVars , "Game Vars" );
	if( HandleCase( m_Time , "Time and Date" ) )
	{
		m_TimeSmoother.HandleTimeChange( m_Time );
	}
	HandleCase( m_CurrentMapId , "Map ID" );
	HandleCase( m_PartyStatus , "Party Status" );
	HandleCase( m_PlayerId , "Player ID" );
	HandleCase( m_BuybackItems , "Buyback Items" );
	HandleCase( m_ShopInventory , "Shop Inventory" );
	HandleCase( m_CurConvState , "Conversation State" );
	HandleCase( m_RestHoursLeft , "Rest Hours" );
	HandleCase( m_NextCombatEncounterId , "Next Combat Encounter" );
	HandleCase( m_CurYieldForMenuState , "Yield For Menu State" );
	HandleCase( m_PlayerTargetedEnt , "Targeted Ent" );
	HandleCase( m_CameraMode , "Camera Mode" );
	HandleCase( m_ShopSeed , "Shop Seed" );
	HandleCase( m_ShopSeedPartyLevel , "Shop Seed Party Level" );
	if( HandleCase( m_CameraSplineRepPose.RecPose , "Path Pose" ) )
	{
		if( IsClient() )
		{
			// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "New Spline Pose: {0}" , EGTransformFormatter(m_CameraSplineRepPose) ) );
		}
	}
	if( HandleCase ( m_EncounterDisposition , "Encounter Disposition" ) )
	{
		if( IsClient() )
		{
			OnClientDispositionChangedDelegate.Broadcast();
		}
	}
	if( HandleCase( m_FrontLineSize , "Front Line Size") )
	{
		if( IsClient() )
		{
			OnClientDispositionChangedDelegate.Broadcast();
		}
	}
	if( HandleCase( m_ActiveTranslationText , "Active Translation" ) )
	{
		if( IsClient() )
		{
			OnClientTranslationChangedDelegate.Broadcast();
		}
	}
	if( HandleCase( m_GameStateInternal , "Game State" ) )
	{
		if( IsClient() )
		{
			OnClientGameStateChangedDelegate.Broadcast();
		}
	}
	if( HandleCase( m_LastTreasureChestContents , "Last Treasure Chest Contents") )
	{
		if( IsClient() )
		{
			ShowTreasureChestRewards();
		}
	}
	
	if( nullptr == ThingStr )
	{
		ThingStr = "UNKNOWN";
	}

	// EGLogf( eg_log_t::General , "Data replicated to %s was %s" , IsServer() ? "server" : "client" , ThingStr );
}

void ExGame::OnClientEnterWorld( const eg_lockstep_id& LockstepId )
{
	Super::OnClientEnterWorld( LockstepId );

	assert( !m_ClientsInWorld.Contains( LockstepId ) );
	m_ClientsInWorld.AppendUnique( LockstepId );

	m_PartyStatusChecksum = eg_crc( "" ); // Reset the replicated data so that the new client gets it.
	SDK_RunClientEvent( eg_crc( "RefreshHUD" ) );

	if( m_EntWorldOwner )
	{
		EGNavGraph NavGraph = SDK_GetNavGraph( eg_nav_graph_id( eg_crc( "nav" ) ) );

		eg_transform SpawnPose = m_PlayerSpawnLocation;

		eg_bool bMainController = m_ClientPCs.IsEmpty();
		EGEnt* SpawnedEnt = m_EntWorldOwner->SpawnEnt( bMainController && NavGraph.IsValid() ? eg_crc("PlayerController") : eg_crc("FreeMovePlayerController") , SpawnPose );
		ExEnt* SpawnedPC = EGCast<ExEnt>(SpawnedEnt);
		if( SpawnedPC )
		{
			SpawnedPC->AssignToPlayer( LockstepId );
			m_ClientPCs.Append( SpawnedPC );
		}
	}
}

void ExGame::OnClientLeaveWorld( const eg_lockstep_id& LockstepId )
{
	EGWeakPtr<ExEnt> ClientPC = nullptr;

	for( EGWeakPtr<ExEnt>& Player : m_ClientPCs )
	{
		if( Player && Player->GetAssignedPlayer() == LockstepId )
		{
			ClientPC = Player;
		}
	}

	if( ClientPC )
	{
		m_ClientPCs.DeleteByItem( ClientPC );
		ClientPC->DeleteFromWorld();
	}

	assert( m_ClientsInWorld.Contains( LockstepId ) );
	m_ClientsInWorld.DeleteByItem( LockstepId );

	Super::OnClientLeaveWorld( LockstepId );
}

void ExGame::NativeEventQuitFromInn()
{
	assert( IsServer() );

	if( IsServer() )
	{
		// The client will handle this.
		SDK_RunClientEvent( eg_crc( "QuitFromInn" ) ); // Forward to client
	}
}

void ExGame::HandleHighestPriorityEncounter( eg_bool bAllowRandomEncounter )
{
	eg_uint EncounterCount = 0;
	eg_ent_id EncounteredEnt = INVALID_ID;
	ExPlayerController* PlayerEnt = GetPlayerEnt();
	if( nullptr == PlayerEnt )
	{
		assert( false );
		return;
	}
	ex_face FaceDir = PoseToFace( PlayerEnt->GetPose() );
	EGNavGraphVertex VertexForEncounter = PlayerEnt->GetMapVertex();
	PlayerEnt->RestoreCamera();
	// Delete any treasure chest we no longer want.
	if( m_TreasureChestToDelete.IsValid() )
	{
		m_TreasureChestToDelete->FadeOut();
		m_TreasureChestToDelete = nullptr;
	}

	assert( !m_CurrentCombatEntity.IsValid() );

	eg_bool bIsSpotSafe = false;

	switch( m_LastMapRevealResult )
	{
		case ex_map_reveal_result::None:
			EGLogf( eg_log_t::Verbose , "Last Map Reveal: None" );
			break;
		case ex_map_reveal_result::NewlyRevealed:
			EGLogf( eg_log_t::Verbose , "Last Map Reveal: NewlyRevealed" );
			break;
		case ex_map_reveal_result::AlreadyRevealed:
			EGLogf( eg_log_t::Verbose , "Last Map Reveal: AlreadyRevealed" );
			break;
	}

	//
	// Monster encounters...
	//
	if( 0 == EncounterCount )
	{
		for( eg_uint i=0; i<m_MonsterList.Len(); i++ )
		{
			ExMapNavBase* MonsterEnt = EGCast<ExMapNavBase>(GetEnt( m_MonsterList[i] ));

			if( MonsterEnt && !MonsterEnt->IsPendingDestruction() )
			{
				if( MonsterEnt->GetMapVertex() == VertexForEncounter )
				{
					EncounteredEnt = m_MonsterList[i];
					EncounterCount++;
				}
			}
		}

		if( EncounterCount > 0 && EncounteredEnt != INVALID_ID )
		{
			EGLogf( eg_log_t::Verbose , "Monster Encounter (%u)" , EncounteredEnt.Id );
			ExEnemyController* Enemy = EGCast<ExEnemyController>(SDK_GetEnt( EncounteredEnt ));
			eg_string_crc EncounterId = eg_crc("DefaultEncounter");
			if( Enemy && Enemy->GetEncounterId().IsNotNull() )
			{
				EncounterId = Enemy->GetEncounterId();
			}
			else if( GetCurrentMapInfo().EncounterData.Fixed.IsNotNull() )
			{
				EncounterId = GetCurrentMapInfo().EncounterData.Fixed;
			}
			PlayerEnt->OnEncounterStarted(); // We want the player to know there was an encounter so the "Halt" event can play, etc...
			m_CurrentCombatEntity = Enemy;
			if( m_CurrentCombatEntity )
			{
				m_CurrentCombatEntity->HandleCombatStarted();
			}
			BeginCombat( EncounterId );
		}
	}
	
	//
	// Safe spots encounters
	//
	if( m_SafeSpots.Contains( VertexForEncounter ) )
	{
		bIsSpotSafe = true;
	}

	//
	// IO Encounters
	//
	if( 0 == EncounterCount )
	{
		EGArray<ExIoEnt*> EncIoEnts;

		for( eg_uint i=0; i<m_TrackedIoEnts.Len(); i++ )
		{
			exTrackedIoEnt& TrackedIo = m_TrackedIoEnts[i];
			if( TrackedIo.IoEnt && TrackedIo.IoEnt->AreEncountersEnabled() && TrackedIo.NavVertex == VertexForEncounter )
			{
				bIsSpotSafe = true; // If an IO is on the spot it's always safe even if not facing the party.

				if( IsFacing( TrackedIo.Face , FaceDir ) && GetGameState() == ex_game_s::NONE )
				{
					EGLogf( eg_log_t::Verbose , "Encountered IO %u" , TrackedIo.IoEnt->GetID().Id );
					EncIoEnts.Append( TrackedIo.IoEnt.GetObject() );
				}
			}
		}

		auto IoCompareFn = []( const ExIoEnt* Left , const ExIoEnt* Right ) -> eg_bool
		{
			if( Left && Right )
			{
				return Left->GetEncounterPriority() > Right->GetEncounterPriority();
			}

			return false;
		};

		EncIoEnts.Sort( IoCompareFn );

		for( ExIoEnt* IoEnt : EncIoEnts )
		{
			if( GetGameState() == ex_game_s::NONE && 0 == EncounterCount )
			{
				m_bIgnoreScriptEncounter = false;

				IoEnt->OnEncounter();

				if( !m_bIgnoreScriptEncounter )
				{
					EncounterCount++;
				}
				m_bIgnoreScriptEncounter = false;
			}
		}
	}

	// Transition Ents
	if( 0 == EncounterCount )
	{
		for( const exMapTransitionEnt& Trans : m_MapTransitionEnts )
		{
			if( Trans.Ent && Trans.Ent->GetScriptId().IsNotNull() && Trans.NavVertex == VertexForEncounter )
			{
				bIsSpotSafe = true; // If any transition was on the spot it's safe.

				if( Trans.InteractFace == ex_face::ALL || Trans.InteractFace == FaceDir )
				{
					eg_string_crc ScriptId = Trans.Ent->GetScriptId();
					eg_string_crc ScriptEntry = Trans.Ent->GetScriptEntry();
					Trans.Ent->HandlePlayerLookAt( PlayerEnt );
					BeginStateMachine( ScriptId , ScriptEntry );
					EncounterCount++;
					break;
				}
			}
		}
	}

	// Random combat
	if( 0 == EncounterCount )
	{
		if( bAllowRandomEncounter && !bIsSpotSafe && m_SafeTurns <= 0 )
		{
			const ex_attr_value PartyMinLevel = GetPartyMinTrainToLevel();
			const ex_attr_value PartyMaxLevel = GetPartyMaxTrainToLevel();
			const eg_int MapSquaresRevealed = GetNumMapSquaresRevealed();
			const eg_real EncounterPct = GetCurrentMapInfo().EncounterData.GetEncounterProbabilityPercent( PartyMinLevel , PartyMaxLevel , m_EncounterDisposition , m_LastMapRevealResult == ex_map_reveal_result::NewlyRevealed , MapSquaresRevealed );
			const eg_real RandomPct = m_Rng.GetRandomRangeF( 0.f , 100.f );
			const eg_bool bDoEncounter = RandomPct < EncounterPct; // Less than because if EncounterPct is 0 and RandomPct is 0 we don't want an encounter.

			EGLogf( eg_log_t::Verbose , "Chance for encounter: %g/%g %s" , EncounterPct , RandomPct , bDoEncounter ? "Encounter!" : "Safe" );
			
			if( bDoEncounter )
			{
				PlayerEnt->OnEncounterStarted(); // We want the player to know there was an encounter so the "Halt" event can play, etc...
				BeginRandomCombat();
				EncounterCount++;
			}
		}
	}
}

eg_bool ExGame::CanHaveEncounterHereWhileResting() const
{
	eg_bool bIsSpotSafe = false;

	const ExPlayerController* PlayerEnt = GetPlayerEnt();
	if( nullptr == PlayerEnt )
	{
		assert( false );
		return false;
	}

	EGNavGraphVertex VertexForEncounter = PlayerEnt->GetMapVertex();
	
	//
	// Safe spots encounters
	//
	if( m_SafeSpots.Contains( VertexForEncounter ) )
	{
		bIsSpotSafe = true;
	}

	//
	// IO Encounters
	//
	{
		for( eg_uint i=0; i<m_TrackedIoEnts.Len(); i++ )
		{
			const exTrackedIoEnt& TrackedIo = m_TrackedIoEnts[i];
			if( TrackedIo.IoEnt && TrackedIo.IoEnt->AreEncountersEnabled() && TrackedIo.NavVertex == VertexForEncounter )
			{
				if( TrackedIo.IoEnt->IsA( &ExTreasureChest::GetStaticClass() ) )
				{
					// Treasure chests do not make a spot safe.
				}
				else
				{
					bIsSpotSafe = true; // If an IO is on the spot it's always safe even if not facing the party.
				}
			}
		}
	}

	// Auto transition encounters
	{
		for( const exMapTransitionEnt& Trans : m_MapTransitionEnts )
		{
			if( Trans.NavVertex == VertexForEncounter )
			{
				bIsSpotSafe = true; // If any transition was on the spot it's safe.
			}
		}
	}

	return !bIsSpotSafe;
}

void ExGame::BeginRandomCombat()
{
	assert( GetGameState() == ex_game_s::NONE );

	EGLogf( eg_log_t::GameLib , "Random Encounter!" );
	m_TotalRandomEncounters++;

	eg_string_crc EncounterId = eg_crc("DefaultEncounter");
	const eg_string_crc RandomEncounterId = GetCurrentMapInfo().EncounterData.RandomEncouter.GetEncounter( m_Rng );
	if( RandomEncounterId )
	{
		EncounterId = RandomEncounterId;
	}

	if( !EX_CHEATS_ENABLED || !ExGame_SimulateRandomCombat.GetValue() )
	{
		BeginCombat( EncounterId );
	}
	else
	{
		ex_attr_value GoldEarned = 0;
		ex_xp_value XpEarned = 0;
		DebugProcessEncounter( EncounterId , &GoldEarned , &XpEarned );

		{
			egsm_var PartyGold = SmGetVar( eg_crc("PartyGold") );
			PartyGold = PartyGold + GoldEarned;
			SmSetVar( eg_crc("PartyGold") , PartyGold );
		}

		eg_int NumPartyMembers = 0;
		for( eg_uint i=0; i<ExRoster::PARTY_SIZE; i++ )
		{
			if( GetPartyMemberByIndex( i ) )
			{
				NumPartyMembers++;
			}
		}

		eg_int XpPerMember = NumPartyMembers > 0 ? EG_To<eg_int>(XpEarned/NumPartyMembers) : 0;

		if( NumPartyMembers > 0 )
		{
			for( eg_uint i=0; i<ExRoster::PARTY_SIZE; i++ )
			{
				ExFighter* PartyMember = GetPartyMemberByIndex( i );
				if( PartyMember )
				{
					PartyMember->AwardXP( XpEarned/NumPartyMembers );
				}
			}
		}

		SmSetVar( eg_crc("DebugCombatXp") , XpPerMember );
		SmSetVar( eg_crc("DebugCombatGold") , GoldEarned );
		SmSetVar( eg_crc("TotalCombats") , SmGetVar( eg_crc("TotalCombats") ).as_int() + 1 );

		m_SafeTurns = GetCurrentMapInfo().EncounterData.SafeTurnsAfterCombat;

		BeginStateMachine( eg_crc("CoreLib") , eg_crc("HandleSimulatedRandomCombat") );
	}
}

void ExGame::RevealAutomap( const eg_transform& Pose )
{
	if( m_Mapping )
	{
		m_LastMapRevealResult = m_Mapping->RevealPose( Pose );
	}
}

void ExGame::BeginCombat( const eg_string_crc& EncounterId )
{
	assert( IsServer() );
	assert( GetGameState() == ex_game_s::NONE );

	ClearActiveTranslationText();

	m_NextCombatEncounterId = EncounterId;
	SetGameState( ex_game_s::COMBAT );
	SDK_ReplicateToClient( &m_NextCombatEncounterId , sizeof(m_NextCombatEncounterId) ); // Client needs to know next combat id.
	SDK_RunClientEvent( eg_crc("OnBeginCombat") );
}

void ExGame::Server_HandleEvent( const eg_lockstep_id& SenderId , const egRemoteEvent& Event )
{
	unused( SenderId );

	eg_bool bHandled = m_BhHandler.ExecuteEvent( this , Event );
	if( bHandled )
	{
		return;
	}

	assert( false ); // event not handled?
}

void ExGame::Client_HandleEvent( const egRemoteEvent& Event )
{
	eg_bool bHandled = m_BhHandler.ExecuteEvent( this, Event );
	if( bHandled )
	{
		return;
	}

	assert( false ); // event not handled?
}

void ExGame::BhOnBeginCombat( const eg_event_parms& Parms )
{
	unused( Parms );

	assert( IsInCombat() && IsClient() );
	if( IsClient() )
	{
		if( ExRestMenu_IsOpen( GetClientOwner() ) )
		{
			ExRestMenu_SetAmbush( GetClientOwner() );
		}
		else
		{
			GetClientOwner()->SDK_PushMenu( eg_crc( "CombatMenu" ) );
		}
	}
}

void ExGame::BhBeginRestMenu( const eg_event_parms& Parms )
{
	unused( Parms );

	assert( IsClient() );
	if( IsClient() )
	{
		assert( GetClientOwner()->SDK_GetMenuStack()->IsEmpty() );
		GetClientOwner()->SDK_PushMenu( eg_crc("RestMenu") );
	}
}

void ExGame::BhOnRosterRequestComplete( const eg_event_parms& Parms )
{
	unused( Parms );

	assert( IsClient() );
}

void ExGame::BhOnPartyChanged( const eg_event_parms& Parms )
{
	BhOnRosterChanged( Parms );
}

void ExGame::BhOnRosterChanged( const eg_event_parms& Parms )
{
	unused( Parms );

	assert( IsClient() );

	if( IsClient() )
	{
		m_Roster.ResolveReplicatedData();

		OnClientRosterChangedDelegate.Broadcast();
	}
}

void ExGame::BhDoSave( const eg_event_parms& Parms )
{
	unused( Parms );
	m_bSaveOnNextUpdate = true;
}

void ExGame::BhOnSendInitialRepData( const eg_event_parms& Parms )
{
	unused( Parms );

	SDK_ReplicateToClient( &m_CurrentMapId, sizeof( m_CurrentMapId ) );
	SDK_ReplicateToClient( &m_PartyStatus, sizeof( m_PartyStatus ) );
	SDK_ReplicateToClient( &m_PlayerId , sizeof(m_PlayerId) );
	SDK_ReplicateToClient( &m_BuybackItems , sizeof(m_BuybackItems) );
	SDK_ReplicateToClient( &m_ShopInventory , sizeof(m_ShopInventory) );
	SDK_ReplicateToClient( &m_ActiveTranslationText , sizeof(m_ActiveTranslationText) );
	SDK_ReplicateToClient( &m_EncounterDisposition , sizeof(m_EncounterDisposition) );
	SDK_ReplicateToClient( &m_FrontLineSize , sizeof(m_FrontLineSize) );
	m_Roster.SetFullRosterDirty();
	m_Roster.ReplicateDirtyData( this );
	SDK_ReplicateToClient( &m_Time , sizeof( m_Time ) );
	SDK_ReplicateToClient( &m_ShopSeed , sizeof(m_ShopSeed) );
	SDK_ReplicateToClient( &m_ShopSeedPartyLevel , sizeof(m_ShopSeedPartyLevel) );

	// Need to replicate all game state (this could potentially be big).
	SDK_RunClientEvent( eg_crc( "BeginRepFullGameState" ), m_GameStateVars.Len() );

	for( eg_uint i = 0; i<m_GameStateVars.Len(); i++ )
	{
		egsmVarPair PairToRep;
		PairToRep.Id = m_GameStateVars.GetKeyByIndex( i );
		PairToRep.Value = m_GameStateVars.GetByIndex( i );
		eg_int64 CompactGameVarPair = PairToRep.Store();
		SDK_RunClientEvent( eg_crc( "ReplicateGameVar" ), CompactGameVarPair );
	}

	if( m_Skills )
	{
		EGArray<eg_string_crc> UnlockedSkills;
		m_Skills->GetAllUnlocked( UnlockedSkills );
		for( const eg_string_crc& SkillId : UnlockedSkills )
		{
			SDK_RunClientEvent( eg_crc("ReplicateUnlockedSkill") ,  exSkillRepInfo( SkillId , true ) );
		}
	}

	if( m_Mapping )
	{
		m_Mapping->ReplicateCurrentMap();
	}

	SDK_RunClientEvent( eg_crc( "EndRepFullGameState" ), m_GameStateVars.Len() );

	for( eg_uint i = 0; i < m_GameStateVars.Len(); i++ )
	{
		egsm_var& Var = m_GameStateVars.GetByIndex( i );
		SDK_ReplicateToClient( &Var , sizeof(Var) );
	}
	
	SDK_RunClientEvent( eg_crc( "OnInitialRepDataComplete" ) );
}

void ExGame::BhOnTurnStart( const eg_event_parms& Parms )
{
	unused( Parms );

	//pEngine->SDK_DebugOutpt( "Turn Started" );
	assert( GetGameState() == ex_game_s::NONE );
	SetGameState( ex_game_s::MOVING );
	m_TurnTime = 0.f;
	m_bIgnoreTreasureUntilNextTurn = false;

	//Notify all monsters that the turn started.
	for( eg_uint i=0; i<m_MonsterList.Len(); i++ )
	{
		ExEnt* Monster = GetEnt( m_MonsterList[i] );
		if( Monster )
		{
			Monster->SendMsg( NOTIFY_TURN_START , INVALID_ID );
		}
	}

	SDK_RunClientEvent( eg_crc("ClientNotifyTurnStart") );
}

void ExGame::BhOnPlayerRotated( const eg_event_parms& Parms )
{
	unused( Parms );

	// If we rotated we probably want to interact with any IOs or map transitions.
	// We should never be in a state where there is a monster encounter right now
	HandleHighestPriorityEncounter( false );
}

void ExGame::BhOnTurnEnd( const eg_event_parms& Parms )
{
	unused( Parms );

	//pEngine->SDK_DebugOutpt( "Turn Ended" );
	assert( GetGameState() == ex_game_s::MOVING );
	SetGameState( ex_game_s::NONE );

	if( m_SafeTurns > 0 )
	{
		m_SafeTurns--;
	}

	//Notify all monsters that the turn ended.
	for( eg_uint i=0; i<m_MonsterList.Len(); i++ )
	{
		ExMapNavBase* MonsterEnt = static_cast<ExMapNavBase*>(GetEnt( m_MonsterList[i] ));
		MonsterEnt->SendMsg( NOTIFY_TURN_END , INVALID_ID );
	}

	m_Time.AddSeconds( exTime::SECONDS_IN_DAY/ExGlobalData::Get().GetTurnsInADay() );
	SDK_ReplicateToClient( &m_Time , sizeof(m_Time) );

	HandleHighestPriorityEncounter( true );

	SDK_RunClientEvent( eg_crc("ClientNotifyTurnEnd") );

	// EGLogf( eg_log_t::General , "Squares revealed: %d" , GetNumMapSquaresRevealed() );
}

void ExGame::BhOnCombatResults( const eg_event_parms& Parms )
{
	assert( GetGameState() == ex_game_s::COMBAT );
	assert( !m_CurrentCombatEntity.IsStale() );
	SetGameState( ex_game_s::NONE );

	m_SafeTurns = GetCurrentMapInfo().EncounterData.SafeTurnsAfterCombat;

	const ex_combat_result Result = static_cast<ex_combat_result>(Parms.as_int64());

	if( m_CurrentCombatEntity )
	{	
		if( Result == ex_combat_result::Victory && m_CurrentCombatEntity->IsBoss() )
		{
			exDefeatedBoss DefeatedBossInfo;
			DefeatedBossInfo.MapId = m_CurrentMapId;
			DefeatedBossInfo.NavVertex = m_CurrentCombatEntity->GetBossVertex();
			m_DefeatedBossList.AppendUnique( DefeatedBossInfo );
		}
		m_CurrentCombatEntity->HandleCombatEnded( Result );
		m_CurrentCombatEntity = nullptr;
	}

	switch( Result )
	{
		case ex_combat_result::Defeat:
		{
			SetGameState( ex_game_s::DEAD );
		} break;
		case ex_combat_result::Victory:
		{
			SmSetVar( eg_crc("TotalCombats") , SmGetVar( eg_crc("TotalCombats") ).as_int() + 1 );
			eg_int CombatsSinceLastSave = SmGetVar( eg_crc("CombatsSinceLastSave") ).as_int();
			CombatsSinceLastSave++;
			SmSetVar( eg_crc("CombatsSinceLastSave") , CombatsSinceLastSave );
			switch( CombatsSinceLastSave )
			{
				case 5:
					ExAchievements::Get().UnlockAchievement( eg_crc("ACH_THRILL_SEEKER_5") );
					break;
				case 10:
					ExAchievements::Get().UnlockAchievement( eg_crc("ACH_THRILL_SEEKER_10") );
					break;
				case 15:
					ExAchievements::Get().UnlockAchievement( eg_crc("ACH_THRILL_SEEKER_20") );
					break;
				/*
				case 30:
					ExAchievements::Get().UnlockAchievement( eg_crc("ACH_THRILL_SEEKER_30") );
					break;
				*/
			}

			HandleHighestPriorityEncounter( false ); // Treasure may have been created, or more combat was encountered, so continue encounters.
		} break;
		case ex_combat_result::Fled:
		{
			CalmHostileEnemies();
			MovePlayerToEscapeSquare();
		} break;
	}
}

void ExGame::RunScript( eg_string_crc InScriptId , eg_string_crc InEntryPoint )
{
	const eg_string_crc ScriptId = InScriptId.IsNotNull() ? InScriptId : eg_string_crc(*ExMapInfos::Get().FindInfo( GetCurrentMapId() ).ScriptId);
	const eg_string_crc EntryPoint = InEntryPoint;
	BeginStateMachine( ScriptId , EntryPoint );
}

void ExGame::BhOnStartConversation( const eg_event_parms& Parms )
{
	unused( Parms );

	if( IsClient() )
	{
		assert( nullptr == m_ConversationMenu );
		m_ConversationMenu = EGCast<ExConversationMenu>(GetClientOwner()->SDK_PushMenu( eg_crc("ConversationMenu") ));
	}
}

void ExGame::BhOnConversationChoiceProcessed( const eg_event_parms& Parms )
{
	if( IsClient() )
	{
		eg_bool bEnd = Parms.as_int64() != 0;
		if( m_ConversationMenu )
		{
			m_ConversationMenu->Continue( bEnd ? ExConversationMenu::ex_continue::Close : ExConversationMenu::ex_continue::RefreshDialog );
			if( bEnd )
			{
				m_ConversationMenu = nullptr;
			}
		}
		else
		{
			assert( false );
		}
	}
}

void ExGame::BhSellInventoryItem( const eg_event_parms& Parms )
{
	assert( IsServer() );
	ExInventory& Inventory = m_Roster.GetInventory();
	exInventoryItem ItemToSell = Inventory.GetItemByIndex( Parms.as_int32() );
	Inventory.DeleteItemByIndex( Parms.as_int32() );
	ItemToSell.ResolvePointers();
	ex_attr_value SellValue = ItemToSell.GetSellValue();
	ex_attr_value PartyGold = GetPartyGold();
	SetPartyGold( PartyGold + SellValue );

	if( m_BuybackItems.IsFull() )
	{
		m_BuybackItems.DeleteByIndex( 0 );
	}
	m_BuybackItems.Append( ItemToSell );
	SDK_ReplicateToClient( &m_BuybackItems , sizeof(m_BuybackItems) );

	m_Roster.SetBroadcastRosterUpdateOnNextReplication();
}

void ExGame::BhSetHandleEncounterAfterScript( const eg_event_parms& Parms )
{
	unused( Parms );

	m_bHandleEncountersAfterScript = true;
}

void ExGame::BhOnConversationClear( const eg_event_parms& Parms )
{
	if( IsClient() && m_ConversationMenu )
	{
		if( Parms.as_bool() )
		{
			m_ConversationMenu->Continue( ExConversationMenu::ex_continue::ClearDialog );
		}
		else
		{
			m_ConversationMenu->Continue( ExConversationMenu::ex_continue::Close );
			m_ConversationMenu =  nullptr;
		}
	}
}

void ExGame::BhBeginTreasureChestInteraction( const eg_event_parms& Parms )
{
	if( IsServer() )
	{
		assert( GetGameState() == ex_game_s::NONE );
		SetGameState( ex_game_s::TREASURE_CHEST );
		m_MapEventTreasureChest = Parms.as_ent_id();

		// Just pass the event on to the client.
		if( !IsClient() )
		{
			SDK_RunClientEvent( eg_crc("BeginTreasureChestInteraction") , Parms );
		}
	}

	if( IsClient() )
	{
		GetClientOwner()->SDK_PushMenu( eg_crc("TreasureChestMenu") );
	}
}

void ExGame::BhDoTreasureChestOpening( const eg_event_parms& Parms )
{
	unused( Parms );
	assert( IsServer() );
	assert( GetGameState() == ex_game_s::TREASURE_CHEST );

	ExTreasureChest* TreasureChest = EGCast<ExTreasureChest>( GetEnt(m_MapEventTreasureChest) );
	if( TreasureChest )
	{
		TreasureChest->OpenTreasureChest();
	}
}

void ExGame::BhEndTreasureChestIntraction( const eg_event_parms& Parms )
{
	unused( Parms );
	assert( IsServer() );
	assert( GetGameState() == ex_game_s::TREASURE_CHEST );

	// We want the map event to be none for the script, so clear it before running the script.
	SetGameState( ex_game_s::NONE );

	eg_bool bRunScript = Parms.as_bool();
	
	if( bRunScript )
	{
		ExTreasureChest* TreasureChest = EGCast<ExTreasureChest>( GetEnt(m_MapEventTreasureChest) );
		if( TreasureChest )
		{
			TreasureChest->RunTreasureScript();
			if( !TreasureChest->IsUnique() )
			{
				if( m_TreasureChestToDelete.IsValid() )
				{
					m_TreasureChestToDelete->FadeOut();
					m_TreasureChestToDelete = nullptr;
				}
				m_TreasureChestToDelete = TreasureChest;
			}
		}
	}
	else
	{
		// If we didn't run the treasure script we want the player to no longer
		// be looking at the chest so it feels as if they backed out.
		ExPlayerController* PC = GetPlayerEnt();
		if( PC )
		{
			PC->RestoreCamera();
		}
	}
}


void ExGame::BhClosedTreasureDialog( const eg_event_parms& Parms )
{
	unused( Parms );
	
	if( IsServer() )
	{
		assert( GetGameState() == ex_game_s::TREASURE_CHEST_RESULTS );
		SetGameState( ex_game_s::NONE );

		ExTreasureChest* TreasureChest = EGCast<ExTreasureChest>( GetEnt(m_MapEventTreasureChest) );
		if( !m_LastTreasureChestContents.bWasAbleToOpen && TreasureChest )
		{
			TreasureChest->CloseTreasureChest();
		}

		m_MapEventTreasureChest = INVALID_ID;
	}
}

void ExGame::BhOnScriptDialogBox( const eg_event_parms& Parms )
{
	unused( Parms );

	assert( IsClient() );

	if( IsClient() )
	{
		// Fix up the replicated data.
		exDialogParms DlgParms( CT_Clear );
		DlgParms.RemoteEventOnClose = eg_crc("OnScriptDialogBoxChoiceMade");
		DlgParms.HeaderText = eg_loc_text(m_CurYieldForMenuState.Speaker);
		DlgParms.BodyText = EGFormat( m_CurYieldForMenuState.CurDialog , this );
		DlgParms.Choices.Append( m_CurYieldForMenuState.Choices.GetArray() , m_CurYieldForMenuState.Choices.Len() );

		ExDialogMenu_PushDialogMenu( GetClientOwner() , DlgParms );
	}
}

void ExGame::BhOnScriptDialogBoxChoiceMade( const eg_event_parms& Parms )
{
	if( IsServer() )
	{
		assert( m_bMapScriptIsInDialogBox );
		assert( m_MapScriptYieldReason == ex_script_yield_reason::MENU_RESPONSE );
		ResumeStateMachine( Parms.as_string_crc() );
	}
}

void ExGame::BhOnConversationChoice( const eg_event_parms& Parms )
{
	assert( GetGameState() == ex_game_s::RUNNING_SCRIPT );
	assert( m_MapScriptYieldReason == ex_script_yield_reason::CONVERSATION_CHOICE || m_MapScriptYieldReason == ex_script_yield_reason::NPC_GOSSIP );
	assert( m_bMapScriptIsInConversation );

	if( m_MapScriptYieldReason == ex_script_yield_reason::NPC_GOSSIP )
	{
		ResumeStateMachine( CT_Clear );
	}
	else
	{
		eg_size_t ConvChoiceIndex = static_cast<eg_size_t>(Parms.as_int64());
		eg_string_crc ChoiceMade = m_CurConvState.Choices.IsValidIndex(ConvChoiceIndex) ? m_CurConvState.Choices[ConvChoiceIndex] : eg_string_crc( CT_Clear );
	
		ResumeStateMachine( ChoiceMade );
	}
} 

void ExGame::BhRefreshConversationNameplate( const eg_event_parms& Parms )
{
	unused( Parms );

	if( IsClient() && m_ConversationMenu )
	{
		m_ConversationMenu->RefreshNameplate();
	}
}

void ExGame::BhOpenScriptMenu( const eg_event_parms& Parms )
{
	assert( IsClient() )
	if( IsClient() )
	{
		// If there is already a script menu up and it's the same type
		// refresh it, otherwise push a new script menu and dismiss the old
		// one.
		if( m_ScriptMenu && m_ScriptMenuType == Parms.as_string_crc() )
		{
			m_ScriptMenu->Refresh();
		}
		else
		{
			m_ScriptMenuType = Parms.as_string_crc();
			if( m_ScriptMenu )
			{
				m_ScriptMenu->DismissMenu();
				m_ScriptMenu = nullptr;
			}

			m_ScriptMenu = EGCast<ExScriptMenu>(GetClientOwner()->SDK_PushMenu( m_ScriptMenuType ));
			if( m_ScriptMenu )
			{
				m_ScriptMenu->Refresh();
			}
		}
	}
}

void ExGame::BhOnScriptMenuChoiceProcessed( const eg_event_parms& Parms )
{
	unused( Parms );

	if( IsClient() && m_ScriptMenu )
	{
		m_ScriptMenu->Continue();
	}
}

void ExGame::BhOnScriptMenuChoice( const eg_event_parms& Parms )
{
	assert( GetGameState() == ex_game_s::RUNNING_SCRIPT );
	assert( m_MapScriptYieldReason == ex_script_yield_reason::MENU_RESPONSE );
	assert( m_bMapScriptInScriptMenu );

	eg_size_t ConvChoiceIndex = static_cast<eg_size_t>(Parms.as_int64());
	eg_string_crc ChoiceMade = m_CurYieldForMenuState.Choices.IsValidIndex(ConvChoiceIndex) ? m_CurYieldForMenuState.Choices[ConvChoiceIndex] : eg_string_crc( CT_Clear );

	ResumeStateMachine( ChoiceMade );
	SDK_RunClientEvent( eg_crc("OnScriptMenuChoiceProcessed") );
}

void ExGame::BhCloseScriptMenu( const eg_event_parms& Parms )
{
	unused( Parms );

	assert( IsClient() )
	
	if( IsClient() && m_ScriptMenu )
	{
		m_ScriptMenu->DismissMenu();
		m_ScriptMenuType = CT_Clear;
	}

	m_ScriptMenu = nullptr;
}

void ExGame::BhSaveFromInn( const eg_event_parms& Parms )
{
	unused( Parms );

	assert( GetGameState() == ex_game_s::RUNNING_SCRIPT );
	ex_game_s CurMapEvent = GetGameState();
	eg_bool bCurMapScriptIsInConversation = m_bMapScriptIsInConversation;
	m_GameStateInternal = ex_game_s::NONE;
	m_bMapScriptIsInConversation = false;
	m_bDontShowPopupForNextSave = true;
	SaveGame( true );
	m_bMapScriptIsInConversation = bCurMapScriptIsInConversation;
	m_GameStateInternal = CurMapEvent;
}

void ExGame::BhRosterMenuRemovePartyMember( const eg_event_parms& Parms )
{
	assert( IsServer() );

	m_Roster.RemovePartyMemberByIndex( static_cast<eg_uint>(Parms.as_int64() ) );
	if( m_PartyStatus.SelectedPartyMember == Parms.as_int64() )
	{
		for( eg_int i=1; i<ExRoster::PARTY_SIZE; i++ )
		{
			const eg_int NewIndex = (m_PartyStatus.SelectedPartyMember+i)%ExRoster::PARTY_SIZE;
			if( nullptr != GetPartyMemberByIndex( NewIndex ) )
			{
				BhSetSelectedPartyMember( NewIndex );
				break;
			}
		}
	}
	SDK_RunClientEvent( eg_crc( "OnRosterRequestComplete" ) );
}

void ExGame::BhRosterMenuExchangePartyMembers( const eg_event_parms& Parms )
{
	assert( IsServer() );

	exDoubleIndex Indexes( Parms.as_int64() );
	m_Roster.ExchangePartyMembers( Indexes.Index0 , Indexes.Index1 );
	SDK_RunClientEvent( eg_crc("OnRosterRequestComplete") );
}

void ExGame::BhRosterMenuAddPartyMemberToSlot( const eg_event_parms& Parms )
{
	assert( IsServer() );

	exDoubleIndex Indexes( Parms.as_int64() );
	m_Roster.AddRosterMemberToParty( Indexes.Index0 , Indexes.Index1 );
	SDK_RunClientEvent( eg_crc( "OnRosterRequestComplete" ) );
}

void ExGame::BhRosterMenuDeleteRosterMember( const eg_event_parms& Parms )
{
	assert( IsServer() );

	m_Roster.DeleteRosterMember( static_cast<eg_uint>( Parms.as_int64() ) );
	// If we deleted the selected party member move the selection to the start.
	if( GetPartyMemberByIndex( m_PartyStatus.SelectedPartyMember ) == nullptr )
	{
		for( eg_uint i=0; i<ExRoster::PARTY_SIZE; i++ )
		{
			if( GetPartyMemberByIndex( i ) )
			{
				m_PartyStatus.SelectedPartyMember = i;
				break;
			}
		}
	}
	SDK_RunClientEvent( eg_crc( "OnRosterRequestComplete" ) );
}

void ExGame::BhAttuneSkill( const eg_event_parms& Parms )
{
	assert( IsServer() );

	exAttuneSkillRepInfo AttuneInfo = Parms;	
	ExFighter* RosterMember = m_Roster.GetRosterCharacterByIndex( AttuneInfo.RosterIndex );
	if( RosterMember )
	{
		RosterMember->SetSkillAttuned( AttuneInfo.SkillId , AttuneInfo.bAttune );
	}
	m_Roster.SetBroadcastRosterUpdateOnNextReplication();
}

void ExGame::BhBeginRepFullGameState( const eg_event_parms& Parms )
{
	assert( IsClient() );
	unused( Parms );

	new( &m_GameStateVars ) ExGameVarSearchMap( CT_Clear );

	if( m_Skills )
	{
		m_Skills->ResetUnlockedSkills();
	}

	if( m_Mapping )
	{
		m_Mapping->ClientBeginFullReplication( m_CurrentMapId );
	}
}

void ExGame::BhReplicateGameVar( const eg_event_parms& Parms )
{
	assert( IsClient() );
	egsmVarPair Pair;
	Pair.Load( Parms.as_int64() );
	m_GameStateVars.Insert( Pair.Id, Pair.Value );
}

void ExGame::BhReplicateUnlockedSkill( const eg_event_parms& Parms )
{
	exSkillRepInfo RepInfo = Parms;
	if( m_Skills )
	{
		m_Skills->SetSkillUnlocked( RepInfo.SkillId , RepInfo.bUnlocked );
	}
}

void ExGame::BhEndRepFullGameState( const eg_event_parms& Parms )
{
	assert( IsClient() );

	unused( Parms );

	assert( m_GameStateVars.Len() == Parms.as_int64() );
	// If everything has gone correctly, the game state vars here should
	// be in the exact same order as they are on the server, so regular
	// replications should copy them over just fine.

	const exMapInfoData& MapProps = GetCurrentMapInfo();
	m_DayNightCycle.Clear();
	ExDayNightCycles::Get().GetCycles( MapProps.DayNightCycleId , m_DayNightCycle );
}

void ExGame::BhRest( const eg_event_parms& Parms )
{
	unused( Parms );

	m_bRestOnNextUpdate = true;
	m_NextRestType = static_cast<ex_rest_t>(Parms.as_int32());
}

void ExGame::BhDropGold( const eg_event_parms& Parms )
{
	if( IsServer() )
	{
		ExPlayerController* PlayerEnt = GetPlayerEnt();
		if( PlayerEnt )
		{
			ex_face FaceDir = PoseToFace( PlayerEnt->GetPose() );
			EGNavGraphVertex VertexToSpawnTreasure = PlayerEnt->GetMapVertex();

			ExTreasureChest* TreasureChest = FindIoOnVertex<ExTreasureChest>( VertexToSpawnTreasure );
			if( TreasureChest )
			{
				// When gold is dropped on a square that already has treasure we
				// just add that gold to the chest. It's possible that this will
				// close the chest. Currently if the chest is facing another
				// direction it won't be interacted with right away. The party
				// will have to face the chest.
				TreasureChest->AddGold( Parms.as_int32() );
			}
			else
			{
				SmSetVar( eg_crc("LastGoldAmountFound") , Parms.as_int32() );
				eg_transform RotTrans;
				RotTrans = eg_transform::BuildRotationY( EG_Deg( 180.f ) );
				eg_transform TreasureSpawnPose = RotTrans * PlayerEnt->GetPose();
				eg_d_string SpawnValue = EGSFormat8( "AddGold({0});" , Parms.as_int32() );
				ExTreasureChest* Treasure = EGCast<ExTreasureChest>(SDK_SpawnEnt( ExGlobalData::Get().GetDroppedTreasureEntity() , TreasureSpawnPose , *SpawnValue ));
				if( Treasure )
				{
					Treasure->SetUnique( false );
				}
			}
		}
	}
}

void ExGame::BhOpenGameMenu( const eg_event_parms& Parms )
{
	assert( IsClient() );
	if( IsClient() )
	{
		assert( !m_GameMenu.IsValid() );
		m_GameMenu = EGCast<ExMenu>(GetClientOwner()->SDK_PushMenu( Parms.as_string_crc() ));
		assert( m_GameMenu.IsValid() );
		if( !m_GameMenu.IsValid() )
		{
			SDK_RunServerEvent( eg_crc("OnGameMenuClosed") );
		}
	}
}

void ExGame::BhOnGameMenuClosed( const eg_event_parms& Parms )
{
	unused( Parms );

	assert( IsServer() );
	if( IsServer() )
	{
		assert( m_bMapScriptInGameMenu );
		assert( m_MapScriptYieldReason == ex_script_yield_reason::GAME_MENU );
		m_bMapScriptInGameMenu = false;
		ResumeStateMachine( eg_crc("") );
	}
}

void ExGame::BhTrainPartyMember( const eg_event_parms& Parms )
{
	assert( IsServer() )
	if( IsServer() )
	{
		ExFighter* PartyMember = GetPartyMemberByIndex( Parms.as_int32() );
		if( PartyMember && PartyMember->CanTrainToNextLevel() )
		{
			PartyMember->ResolveReplicatedData();
			egsm_var PartyGold = SmGetVar( eg_crc("PartyGold") );
			ex_attr_value CostToTrain = ExCore_GetCostToTrainToLevel( PartyMember->GetAttrValue( ex_attr_t::LVL ) + 1 );
			assert( CostToTrain <= PartyGold.as_int() ) // Menu should have prevented getting here.
			if( CostToTrain <= PartyGold.as_int() )
			{
				PartyGold = PartyGold - CostToTrain;
				SmSetVar( eg_crc("PartyGold") , PartyGold );
				PartyMember->TrainToNextLevel();
				m_Roster.ReplicateDirtyData( this );
				SDK_RunClientEvent( eg_crc("RefreshGameMenu") );

				m_Time.AddHours( ExGlobalData::Get().GetHoursToTrain() );
				SDK_ReplicateToClient( &m_Time , sizeof(m_Time) );
			}
		}
		else
		{
			assert( false ); // Menu should have prevented getting here.
		}
	}
}

void ExGame::BhRefreshGameMenu( const eg_event_parms& Parms )
{
	unused( Parms );

	assert( IsClient() );
	if( IsClient() && m_GameMenu.IsValid() )
	{
		m_GameMenu->Refresh();
	}
}

void ExGame::BhOnItemReplicated( const eg_event_parms& Parms )
{
	assert( IsClient() );
	if( IsClient() )
	{
		m_Roster.OnItemReplicated( Parms.as_int32() );
	}
}

void ExGame::BhSetSelectedPartyMember( const eg_event_parms& Parms )
{
	m_PartyStatus.SelectedPartyMember = Parms.as_int32();
	SDK_ReplicateToClient( &m_PartyStatus.SelectedPartyMember , sizeof(m_PartyStatus.SelectedPartyMember) );
}

void ExGame::BhClientHandlePartyMemberSwitched( const eg_event_parms& Parms )
{
	unused( Parms );

	assert( IsClient() );
	if( IsClient() )
	{
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED );
	}
}

void ExGame::BhEquipInventoryItem( const eg_event_parms& Parms )
{
	m_Roster.HandleEquipEvent( Parms );
}

void ExGame::BhSwapInventoryItem( const eg_event_parms& Parms )
{
	m_Roster.HandleSwapEvent( Parms );
}

void ExGame::BhDropInventoryItem( const eg_event_parms& Parms )
{
	assert( IsServer() );
	ExInventory& Inventory = m_Roster.GetInventory();
	Inventory.DeleteItemByIndex( Parms.as_int32() );
	m_Roster.SetBroadcastRosterUpdateOnNextReplication();
}

void ExGame::BhRevealMapSquare( const eg_event_parms& Parms )
{
	if( m_Mapping )
	{
		m_Mapping->RevealReplicatedSquare( Parms );
	}
}

void ExGame::BhMakePurchase( const eg_event_parms& Parms )
{
	exShopPurchaseParms PurchaseParms = Parms;
	exInventoryItem PurchaseItem;
	PurchaseItem.RawItem.ItemId = PurchaseParms.ArmoryItemId;
	PurchaseItem.RawItem.Level = PurchaseParms.ItemLevel;
	PurchaseItem.ResolvePointers();
	if( CanBuyItem( PurchaseItem , PurchaseParms.bIsBuyback ) == ex_can_buy_t::CanBuy )
	{
		ex_attr_value Cost = PurchaseParms.bIsBuyback ? PurchaseItem.GetSellValue() : PurchaseItem.GetBuyValue();
		ex_attr_value PartyGold = GetPartyGold();
		ExInventory& Inventory = m_Roster.GetInventory();
		SetPartyGold( PartyGold - Cost  );
		Inventory.AddItem( PurchaseItem );

		if( PurchaseParms.bIsBuyback )
		{
			m_BuybackItems.DeleteByItem( PurchaseItem.RawItem );
			SDK_ReplicateToClient( &m_BuybackItems , sizeof(m_BuybackItems) );
		}
		else
		{
			m_ShopInventory.DeleteByItem( PurchaseItem.RawItem );
			SDK_ReplicateToClient( &m_ShopInventory , sizeof(m_ShopInventory) );
		}
	}

	m_Roster.SetBroadcastRosterUpdateOnNextReplication();
}

void ExGame::BhUnlockDoor( const eg_event_parms& Parms )
{
	unused( Parms );

	assert( IsServer() );

	ExDoor* TargetedDoor = EGCast<ExDoor>(GetEnt( GetPlayerTargetedEnt() ));
	if( TargetedDoor )
	{
		const ExFighter* GuyToUnlock = GetBestThiefOrSelectedPartyMember();
		if( GuyToUnlock )
		{
			ex_door_unlock_result UnlockRes = TargetedDoor->AttemptToUnlock( GuyToUnlock );

			eg_string_crc TranslatorMessatge = CT_Clear;
			eg_uint PartyMemberIndex = m_Roster.GetPartyIndex( GuyToUnlock );
			switch( UnlockRes )
			{
			case ex_door_unlock_result::Unlocked:
				TranslatorMessatge =  eg_loc("UnlockedText","{0:NAME} unlocked the door!");
				break;
			case ex_door_unlock_result::LockedNeedsKey:
				TranslatorMessatge = eg_loc("CantUnlockKey","This door is locked by some mechanism or requires a key that the party does not possess.");
				break;
			case ex_door_unlock_result::LockedNeedsThief:
				TranslatorMessatge =  eg_loc("CantUnlockThief","The lock on this door can be picked by a |SC(EX_CLASS)|Thief|RC()| only.");
				break;
			case ex_door_unlock_result::LockedNeedsLevel:
				TranslatorMessatge =  eg_loc("CantUnlockLevel","A more skilled |SC(EX_CLASS)|Thief|RC()| is needed to pick this lock.");
				break;
			}

			SetActiveTranslationText( TranslatorMessatge , PartyMemberIndex );
		}
		else
		{
			assert( false ); // No party?
		}
	}
	else
	{
		assert( false ); // Was not targeting a door.
	}
}

void ExGame::BhQueueWorldMapTransition( const eg_event_parms& Parms )
{
	assert( IsServer() && m_QueuedWorldMapTransition.IsNull() );
	if( IsServer() )
	{
		eg_string_crc TargetMap = Parms.as_string_crc();
		m_QueuedWorldMapTransition = TargetMap;
	}
}

void ExGame::BhServerOpenContextMenu( const eg_event_parms& Parms )
{
	unused( Parms );

	assert( IsServer() );
	if( IsServer() )
	{
		ExPlayerController* PlayerEnt = GetPlayerEnt();

		if( GetGameState() == ex_game_s::NONE && (PlayerEnt == nullptr || PlayerEnt->GetMoveState() == ex_move_s::NONE) )
		{
			BeginStateMachine( eg_crc("MenuLib") , eg_crc("OpenContextMenu") );
		}
	}
}

void ExGame::BhClientNotifyWait( const eg_event_parms& Parms )
{
	unused( Parms );
	
	assert( IsClient() );

	OnClientNotifyWaitDelegate.Broadcast();
}

void ExGame::BhClientNotifyTurnStart( const eg_event_parms& Parms )
{
	unused( Parms );
	
	assert( IsClient() );

	OnClientNotifyTurnStartDelegate.Broadcast();
}

void ExGame::BhClientNotifyTurnEnd( const eg_event_parms& Parms )
{
	unused( Parms );
	
	assert( IsClient() );

	OnClientNotifyTurnEndDelegate.Broadcast();
}

void ExGame::BhServerDoTownPortal( const eg_event_parms& Parms )
{
	const eg_bool bRecall = Parms.as_bool();

	ClearActiveTranslationText();

	if( bRecall )
	{
		if( GetGameState() == ex_game_s::NONE )
		{
			const eg_bool bClearTownPortal = true;

			if( m_TownPortalRecallMarker.TargetMapId.IsNotNull() && (!bClearTownPortal || !m_TownPortalRecallMarker.bRecallWasCast) )
			{
				m_TransitTarget = exTransitTarget( CT_Clear );
				m_TransitTarget.TargetMapId = m_TownPortalRecallMarker.TargetMapId;
				m_TransitTarget.bIsTownPortalRecall = true;

				if( bClearTownPortal )
				{
					m_TownPortalRecallMarker.bClearTownPortalAfterSave = true;
				}

				if( m_TransitTarget.TargetMapId.IsNotNull() )
				{
					SetGameState( ex_game_s::TRANS_MAP );
				}
				else
				{
					m_TransitTarget = CT_Clear;
					assert( false ); // No travel target.
				}
			}
			else
			{
				BeginStateMachine( eg_crc("CoreLib") , eg_crc("HandleNoTownPortalRecall") );
			}
		}
		else
		{
			assert( false );
		}
	}
	else
	{
		if( GetGameState() == ex_game_s::NONE )
		{
			m_TownPortalRecallMarker.TargetMapId = GetCurrentMapInfo().Id;
			m_TownPortalRecallMarker.TargetPose = GetPlayerEnt() ? GetPlayerEnt()->GetPose() : CT_Default;
			m_TownPortalRecallMarker.bRecallWasCast = false;

			m_TransitTarget = exTransitTarget( CT_Clear );
			m_TransitTarget.TargetMapId = ExGlobalData::Get().GetTownPortalTravelTarget().MapId;
			m_TransitTarget.TargetEntranceId = ExGlobalData::Get().GetTownPortalTravelTarget().EntranceId;
			if( m_TransitTarget.TargetMapId.IsNotNull() )
			{
				SmSetVar( eg_crc("LloydsBeaconOpenControl") , 10 );
				SetGameState( ex_game_s::TRANS_MAP );
			}
			else
			{
				m_TransitTarget = CT_Clear;
				assert( false ); // No travel target.
			}
		}
		else
		{
			assert( false );
		}
	}
}

void ExGame::BhQuitGame( const eg_event_parms& Parms )
{
	unused( Parms );

	m_bDontShowPopupForNextSave = true;
	AttemptAutoSave( true ); // Save before quitting.
	SDK_RunServerEvent( eg_crc("ResetGame") );
}

void ExGame::BhClientNotifyEvent( const eg_event_parms& Parms )
{
	assert( IsClient() );
	if( IsClient() )
	{
		const eg_string_crc EventParm = Parms.as_string_crc();
		switch_crc( EventParm )
		{
			case_crc("HealParty"):
			{
				ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::HealParty );
			} break;

			case_crc("LockEndingMovie"):
			{
				ExProfileData_SetEndingMovieUnlocked( false );
			} break;

			case_crc("UnlockEndingMovie"):
			{
				ExProfileData_SetEndingMovieUnlocked( true );
			} break;

			case_crc("LockCastleGame"):
			{
				ExProfileData_SetCastleGameUnlocked( false );
			} break;

			case_crc("UnlockCastleGame"):
			{
				ExProfileData_SetCastleGameUnlocked( true );
			} break;

			case_crc("EndGame"):
			{
				if( EGClient* Client = GetClientOwner() )
				{
					if( EGMenuStack* MenuStack = Client->SDK_GetMenuStack() )
					{
						MenuStack->Push( eg_crc("EndGameFlowMenu") );
					}
				}
			} break;
		}
	}
}

void ExGame::BhServerNotifyEvent( const eg_event_parms& Parms )
{
	assert( IsServer() );
	if( IsServer() )
	{
		const eg_string_crc EventParm = Parms.as_string_crc();
		switch_crc( EventParm )
		{
			case_crc("PartyCreated"):
			{
				m_bSaveOnNextUpdate = true;
			} break;
			case_crc("ToggleEncounterDisposition"):
			{
				m_EncounterDisposition = ( m_EncounterDisposition == ex_encounter_disposition::Default ) ? ex_encounter_disposition::FirstProbabilitySet : ex_encounter_disposition::Default;
				SDK_ReplicateToClient( &m_EncounterDisposition , sizeof(m_EncounterDisposition) );
			} break;
			case_crc("ToggleFrontLineSizeUp"):
			case_crc("ToggleFrontLineSizeDown"):
			{
				const eg_int MIN_FL_SIZE = ExGlobalData::Get().GetCombatPlayerTeamFrontLineMinSize();
				const eg_int MAX_FL_SIZE = ExRoster::PARTY_SIZE;
				m_FrontLineSize = ((m_FrontLineSize - MIN_FL_SIZE) + ((EventParm == eg_crc("ToggleFrontLineSizeUp")) ? 1 : -1) + (MAX_FL_SIZE+1-MIN_FL_SIZE))%(MAX_FL_SIZE+1-MIN_FL_SIZE) + MIN_FL_SIZE;
				SDK_ReplicateToClient( &m_FrontLineSize , sizeof(m_FrontLineSize) );
			} break;
			case_crc("SortInventory"):
			{
				m_Roster.SortInventory();
			} break;
			case_crc("EndGameReturnToTown"):
			{
				m_TransitTarget = exTransitTarget( CT_Clear );
				m_TransitTarget.TargetMapId = ExGlobalData::Get().GetEndGameTravelTarget().MapId;
				m_TransitTarget.TargetEntranceId = ExGlobalData::Get().GetEndGameTravelTarget().EntranceId;

				if (!m_TransitTarget.TargetMapId)
				{
					// Fallback to make sure EXPLOR works correctly.
					m_TransitTarget.TargetMapId = eg_crc( "MAP_Town_01" );
					m_TransitTarget.TargetEntranceId = eg_crc( "TownPortal" );
				}
				SetGameState( ex_game_s::TRANS_MAP );
			} break;
		}
	}
}

void ExGame::BhPlayCameraSpline( const eg_event_parms& Parms )
{
	if( IsServer() )
	{
		if( Parms.as_string_crc().IsNotNull() )
		{
			PlayCameraSpline( Parms.as_string_crc() );
		}
		else
		{
			StopCameraSpline();
		}
	}
}

void ExGame::BhPlaySound( const eg_event_parms& Parms )
{
	switch_crc( Parms.as_string_crc() )
	{
		case_crc("BuyFood"):
		{
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::BuyFood );
		} break;
	}
}

void ExGame::Server_OnConsoleCmd( eg_cpstr Cmd )
{
	egParseFuncInfo Info;
	EGPARSE_RESULT r = EGParse_ParseFunction( Cmd , &Info );
	if(EGPARSE_OKAY != r || !EGString_EqualsI( Info.SystemName , "game") )
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Invalid function call \"%s\". Parse error %u: %s." , Cmd , r , ::EGParse_GetParseResultString(r) );
		return;
	}

	eg_string FunctionName = Info.FunctionName;
	FunctionName.ConvertToUpper();
	eg_string_crc FunctionAsCrc = eg_string_crc( FunctionName );

	switch_crc( FunctionAsCrc )
	{
		case_crc("GENERATENEWSHOPINVENTORY"):
		{
			GeneratedNewSeeds();
		} break;
		case_crc("GIVEQUESTITEMS"):
		{
			ExQuestItems::Get().UnlockAllQuestItems( this );
		} break;
		case_crc("COMPUTESQUARES"):
		{
			eg_int NumSquares = ComputeNumVisitableSquares();
			// EGLogf( eg_log_t::General , "%d squares can be visited." , NumSquares );
			EGLogf( eg_log_t::General , "%d/%d squares visited." , GetNumMapSquaresRevealed() , NumSquares );
		} break;
		case_crc("ROLLDIE"):
		{
			const eg_int TimesToRoll = Info.NumParms >= 1 ? EGString_ToInt(Info.Parms[0]) : 1;
			for( eg_int i=0; i<TimesToRoll; i++ )
			{
				EGLogf( eg_log_t::General , "Die Roll %d: %g" , i+1 , m_Rng.GetRandomRangeF(0.f,100.f) );
			}
		} break;
		case_crc("CANAMBUSHHERE"):
		{
			const eg_bool bCanBeAmbushed = CanHaveEncounterHereWhileResting();
			EGLogf( eg_log_t::General , "An ambush %s happen here." , bCanBeAmbushed ? "can" : "cannot" );
		} break;
		case_crc("PLAYCAMERASPLINE"):
		{
			if( Info.NumParms >= 1 )
			{
				PlayCameraSpline( eg_string_crc( Info.Parms[0] ) );
			}
		} break;
		case_crc("STOPCAMERASPLINE"):
		{
			StopCameraSpline();
		} break;
		case_crc( "AWARDGOLD"):
		{
			egsm_var PartyGold = SmGetVar( eg_crc("PartyGold") );
			PartyGold = PartyGold + eg_string(Info.Parms[0]).ToInt();
			SmSetVar( eg_crc("PartyGold") , PartyGold );
		} break;
		case_crc( "AWARDXP" ):
		{
			if( Info.NumParms == 1 )
			{
				for( eg_uint i=0; i<ExRoster::PARTY_SIZE; i++ )
				{
					ExFighter* PartyMember = GetPartyMemberByIndex( i );
					if( PartyMember )
					{
						PartyMember->AwardXP( eg_string(Info.Parms[0]).ToInt() );
					}
				}
			}
			else if( Info.NumParms == 2 )
			{
				ExFighter* PartyMember = GetPartyMemberByIndex( eg_string(Info.Parms[0]).ToInt() );
				if( PartyMember )
				{
					PartyMember->AwardXP( eg_string(Info.Parms[1]).ToInt() );
				}
			}
		} break;
		case_crc("SHOWVARS"):
		case_crc("SHOWSTATEVARS" ):
		{
			const ExGameVarSearchMap& StateVars = m_GameStateVars;

			eg_string VarFilter = Info.NumParms >= 1 ? Info.Parms[0] : "";
			
			for( eg_uint i=0; i<StateVars.Len(); i++ )
			{
				const eg_string_crc& Key = StateVars.GetKeyByIndex( i );
				const egsm_var& GameVar = StateVars.GetByIndex( i );

				eg_string VarName = EGSmMgr_VarCrcToStr( Key );

				eg_bool bShow = true;
				if( VarFilter.Len() > 0 )
				{
					bShow = EGString_EqualsICount( VarFilter , VarName , VarFilter.Len() );
				}

				if( bShow )
				{
					EGLogf( 
						eg_log_t::General 
						, "\t%s = <%i,%g,%s,0x%X>" 
						, VarName.String() , GameVar.as_int() , GameVar.as_real() 
						, GameVar.as_bool() ? "true" : "false" 
						, GameVar.as_crc().ToUint32() );
				}
			}

		} break;
		case_crc( "UNLOCKSKILL" ):
		{
			if( Info.NumParms >= 1 )
			{
				eg_string_crc SkillId = eg_string_crc(Info.Parms[0]);
				SetSkillUnlocked( SkillId , true );
			}
		} break;
		case_crc( "SET" ):
		case_crc( "SETVAR" ):
		{
			if( Info.NumParms >= 2 )
			{
				eg_string_crc VarName = eg_string_crc(Info.Parms[0]);
				eg_string NewValue = Info.Parms[1];
				egsm_var VarToSet = SmGetVar( VarName );
				// Now it's a matter of deciding what NewValue is...
				if( NewValue.IsInteger() )
				{
					VarToSet = NewValue.ToInt();
					EGLogf( eg_log_t::GameLibFromGame , "\t%s = %i" , Info.Parms[0] , VarToSet.as_int() );

				}
				else if( NewValue.IsNumber() )
				{
					VarToSet = NewValue.ToFloat();
					EGLogf( eg_log_t::GameLibFromGame , "\t%s = %g" , Info.Parms[0] , VarToSet.as_real() );

				}
				else if( NewValue.EqualsI( "TRUE" ) || NewValue.EqualsI( "FALSE" ) )
				{
					VarToSet = NewValue.EqualsI( "TRUE" );
					EGLogf( eg_log_t::GameLibFromGame , "\t%s = %s" , Info.Parms[0] , VarToSet.as_bool() ? "true" : "false"  );

				}
				else // Otherwise assume crc
				{
					VarToSet = eg_string_crc(NewValue);
					EGLogf( eg_log_t::GameLibFromGame , "\t%s = 0x%X" , Info.Parms[0] , VarToSet.as_crc().ToUint32() );
				}

				SmSetVar( VarName , VarToSet );
			}
			else
			{
				EGLogf( eg_log_t::Warning , "Usage: game.SetVar( VarName , NewValue )" );
			}
		} break;
		case_crc("COMBAT"):
		case_crc("DORANDOMENCOUNTER"):
		{
			if( ConDraw_IsActive() )
			{
				ConDraw_ToggleActive();
			}
			
			if( GetGameState() == ex_game_s::NONE )
			{
				if( Info.NumParms >= 1 )
				{
					BeginCombat( eg_string_crc(Info.Parms[0]) );
				}
				else
				{
					BeginRandomCombat();
				}
			}
		} break;
		case_crc("TESTCOMBATTARGETING"):
		{
			ExCombatAI_TestChoosCombatantToAttack( this );
		} break;
		case_crc("BALANCEPARTY"):
		{
			DebugBalanceParty( Info.NumParms >= 1 ? EGString_ToInt(Info.Parms[0]) : 0 );
		} break;
		case_crc("GIVEITEM"):
		{
			exInventoryItem NewItem;
			NewItem.RawItem.ItemId = Info.NumParms >= 1 ? eg_string_crc(Info.Parms[0]) : CT_Clear;
			NewItem.RawItem.Level = Info.NumParms >= 2 ? EGString_ToInt(Info.Parms[1]) : 1;
			NewItem.ResolvePointers();
			if( NewItem.ArmoryItem )
			{
				ExInventory& Inventory = m_Roster.GetInventory();
				Inventory.AddItem( NewItem );
				m_Roster.SetBroadcastRosterUpdateOnNextReplication();
			}
			else
			{
				EGLogf( eg_log_t::General , "No such item." );
			}
		} break;
		case_crc("SIMULATECOMBATENCOUNTERS"):
		{
			DebugSimulateCombatEncounters( Info.NumParms >= 1 ? EGString_ToInt(Info.Parms[0]) : 1 , Info.NumParms >= 2 ? EGString_ToInt(Info.Parms[1] ) : 0 );
		} break;
		case_crc("SETENDINGMOVIEUNLOCKED"):
		{
			if( Info.NumParms >= 1 && EGString_ToBool(Info.Parms[0]) )
			{
				SDK_RunClientEvent( eg_crc("ClientNotifyEvent") , eg_crc("UnlockEndingMovie") );
			}
			else
			{
				SDK_RunClientEvent( eg_crc("ClientNotifyEvent") , eg_crc("LockEndingMovie") );
			}
		} break;
		default:
		{
			EGLogf( eg_log_t::Warning , __FUNCTION__ ": Unrecognized function %s." , Info.FunctionName );
		} break;
	}
}

void ExGame::BeginStateMachine( eg_string_crc ProcId, eg_string_crc EntryPoint )
{
	assert( GetGameState() == ex_game_s::NONE );
	assert( !m_bMapScriptIsInConversation );
	assert( m_MapScriptYieldReason == ex_script_yield_reason::NONE );
	SetGameState( ex_game_s::RUNNING_SCRIPT );

	m_CurScriptState = egsmState( CT_Clear );

	EGSmRuntime_Run( m_CurScriptState , ProcId , EntryPoint , this );

	StateMachineFollowUp();
}

void ExGame::ResumeStateMachine( eg_string_crc Input )
{
	assert( m_MapScriptYieldReason != ex_script_yield_reason::NONE ); // Why did we yield?
	m_MapScriptYieldReason = ex_script_yield_reason::NONE;

	EGSmRuntime_Resume( m_CurScriptState , Input , this );

	StateMachineFollowUp();
}

void ExGame::StateMachineFollowUp()
{
	if( m_CurScriptState.ProcState == egsm_proc_state::YIELD )
	{
		assert( GetGameState() == ex_game_s::RUNNING_SCRIPT ); // If we're yielding for anything the map state shouldn't be changed (anything that changes map state should be terminal anyway)

		switch( m_MapScriptYieldReason )
		{
		case ex_script_yield_reason::CONVERSATION_CHOICE:
		case ex_script_yield_reason::NPC_GOSSIP:
			SDK_ReplicateToClient( &m_CurConvState , sizeof(m_CurConvState) );
			if( !m_bMapScriptIsInConversation )
			{
				m_bMapScriptIsInConversation = true;
				SDK_RunClientEvent( eg_crc("OnStartConversation") );
			}
			else
			{
				SDK_RunClientEvent( eg_crc("OnConversationChoiceProcessed") , false );
			}
			break;
		case ex_script_yield_reason::MENU_RESPONSE:
			assert( m_bMapScriptIsInDialogBox || m_bMapScriptInScriptMenu );
			break;
		case ex_script_yield_reason::GAME_MENU:
			assert( !m_bMapScriptInGameMenu );
			m_bMapScriptInGameMenu = true;
			break;
		default:
			assert( false );
			break;
		}
	}
	else
	{
		if( m_bMapScriptIsInConversation )
		{
			SDK_RunClientEvent( eg_crc("OnConversationChoiceProcessed") , true );
			m_bMapScriptIsInConversation = false;
		}

		if( m_bMapScriptInScriptMenu )
		{
			SDK_RunClientEvent( eg_crc("CloseScriptMenu") );
			m_bMapScriptInScriptMenu = false;
		}

		if( m_bMapScriptIsInDialogBox )
		{
			m_bMapScriptIsInDialogBox = false;
		}

		assert( m_CurScriptState.ProcState != egsm_proc_state::TERMINAL_YIELD ); // Terminal yields are no longer supported.

		m_CurConvState.Clear();
		m_CurScriptState.ClearToTerminalError();
		if( GetGameState() == ex_game_s::RUNNING_SCRIPT )
		{
			SetGameState( ex_game_s::NONE );
			ExPlayerController* PC = GetPlayerEnt();
			if( PC )
			{
				PC->RestoreCamera();
			}
		}
		else
		{
			assert( GetGameState() == ex_game_s::TRANS_MAP ); // Went to an invalid game state?
		}
		m_MapScriptYieldReason = ex_script_yield_reason::NONE;
	}
}

void ExGame::SetActiveTranslationText( const eg_string_crc& NewText , eg_uint PartyMemberIndex )
{
	assert( IsServer() );
	if( m_ActiveTranslationText.Message != NewText )
	{
		m_ActiveTranslationText.Message = NewText;
		m_ActiveTranslationText.PartyMemberIndex = PartyMemberIndex;
		SDK_ReplicateToClient( &m_ActiveTranslationText , sizeof(m_ActiveTranslationText) );
	}
}

void ExGame::ResetStuffForPlayerMoving()
{
	m_bIgnoreTreasureUntilNextTurn = false;
	ClearActiveTranslationText();
}

void ExGame::SetLastTreasureChestContents( const exTreasureChestResults& Contents )
{
	assert( IsServer() );
	if( IsServer() )
	{
		m_LastTreasureChestContents = Contents;

		SetGameState( ex_game_s::TREASURE_CHEST_RESULTS );
		SDK_ReplicateToClient( &m_LastTreasureChestContents , sizeof(m_LastTreasureChestContents) );
	}

	m_bIgnoreTreasureUntilNextTurn = true;
}

void ExGame::ShowTreasureChestRewards()
{
	assert( IsClient() );
	if( IsClient() )
	{
		// Fix up the replicated data.
		exDialogParms DlgParms( CT_Clear );
		// DlgParms.RemoteEventOnClose = eg_crc("OnScriptDialogBoxChoiceMade");
		DlgParms.HeaderText = eg_loc_text(eg_loc("ExGameTreasure","Treasure"));// eg_loc_text(m_CurYieldForMenuState.Speaker);
		DlgParms.RemoteEventOnClose = eg_crc("ClosedTreasureDialog");
		if( m_LastTreasureChestContents.bWasAbleToOpen )
		{
			eg_bool bHasQuestItems = false;

			eg_d_string16 FinalStr = eg_loc_text(eg_loc("ExGameChestContains","The chest contains:\n")).GetString();
			if( m_LastTreasureChestContents.GoldAmount > 0 )
			{
				eg_loc_text GoldText = EGFormat( eg_loc("ExGameTreasureGold","|SC(EX_GOLD)|{0} gold|RC()|\n") , m_LastTreasureChestContents.GoldAmount );
				FinalStr.Append( GoldText.GetString() );
			}
			for( const exRawItem& Item : m_LastTreasureChestContents.InventoryItems )
			{
				exInventoryItem InvItem( Item );
				InvItem.ResolvePointers();
				eg_loc_text ItemText = EGFormat( L"{0:NAME}\n" , &InvItem );
				FinalStr.Append( ItemText.GetString() );
			}
			for( const eg_string_crc& Item : m_LastTreasureChestContents.QuestItems )
			{
				exQuestItemInfo ItemInfo = ExQuestItems::Get().GetItemInfo( Item );
				eg_loc_text ItemText = EGFormat( L"{0:NAME}\n" , &ItemInfo );
				FinalStr.Append( ItemText.GetString() );
				bHasQuestItems = true;
			}

			if( bHasQuestItems )
			{
				FinalStr.Append( eg_loc_text(eg_loc("ExGameTreasureQuestItemBagHint","\n(Quest Items are stored in a special bag found in the Game Status menu.)")).GetString() );
			}

			DlgParms.BodyText = eg_loc_text( *FinalStr );
			ExDialogMenu_PushDialogMenu( GetClientOwner() , DlgParms );

		}
		else
		{
			DlgParms.BodyText = eg_loc_text(eg_loc("NotEnoughBackpackForTreasure","The party does not have enough backpack space to collect the items in this chest."));
			ExDialogMenu_PushDialogMenu( GetClientOwner() , DlgParms );
		}

	}
}

void ExGame::BeginResting()
{
	assert( IsServer() );
	assert( GetGameState() == ex_game_s::NONE );

	const eg_int RestHours = ExGlobalData::Get().GetRestHours();

	auto ComputePerHourRestorations = [this,&RestHours]() -> void
	{
		// Calculate amount of healing per hour
		for( eg_uint i=0; i<ExRoster::PARTY_SIZE; i++ )
		{
			ExFighter* PartyMember = GetPartyMemberByIndex( i );
			m_PartyMemberRestHPPerHour[i] = 0;
			m_PartyMemberRestMPPerHour[i] = 0;
			if( PartyMember )
			{
				PartyMember->ResolveReplicatedData();
				const ex_attr_value HpNeeded = PartyMember->GetAttrValue( ex_attr_t::HP ) - PartyMember->GetHP();
				const ex_attr_value MpNeeded = PartyMember->GetAttrValue( ex_attr_t::MP ) - PartyMember->GetMP();
				m_PartyMemberRestHPPerHour[i] = EG_Max<ex_attr_value>(HpNeeded/RestHours,1);
				m_PartyMemberRestMPPerHour[i] = EG_Max<ex_attr_value>(MpNeeded/RestHours,1);

				// Do a heal for all the slack that's left over due to rounding when calculating HP per hour
				if( !PartyMember->IsDead() )
				{
					ex_attr_value HpSlack = HpNeeded - m_PartyMemberRestHPPerHour[i]*RestHours;
					ex_attr_value MpSlack = MpNeeded - m_PartyMemberRestMPPerHour[i]*RestHours;
					if( HpSlack > 0 )
					{
						PartyMember->ApplyRawHeal( HpSlack );
					}
					if( MpSlack > 0 )
					{
						PartyMember->ApplyManaRestoration( MpSlack );
					}

					// Do the first heal right away that way the UI will appear as if the
					// heal finishes before it closes.
					PartyMember->ApplyManaRestoration( m_PartyMemberRestMPPerHour[i] );
					PartyMember->ApplyRawHeal( m_PartyMemberRestHPPerHour[i] );
				}
			}
		}

		const eg_int PartyFood = SmGetVar( eg_crc("PartyFood") ).as_int();

		for( eg_uint i=0; i<ExRoster::PARTY_SIZE; i++ )
		{
			ExFighter* PartyMember = GetPartyMemberByIndex( i );
			if( PartyMember )
			{
				if( !PartyMember->IsDead() && PartyFood > 0 )
				{
					PartyMember->SetUnconcious( false );
				}
			}
		}
	};

	if( IsServer() && GetGameState() == ex_game_s::NONE )
	{
		SetGameState( ex_game_s::RESTING );
		m_RestTimer = 0.f;
		m_RestHoursLeft = ExGlobalData::Get().GetRestHours();

		if( m_NextRestType == ex_rest_t::UntilMorning )
		{
			// Immediately pass time to the next whole hour
			eg_uint SecondOfMinute = m_Time.GetDateInfo().SecondOfMinute;
			if( SecondOfMinute != 0 )
			{
				m_Time.AddSeconds( exTime::SECONDS_IN_MINUTE - SecondOfMinute );
			}

			eg_uint MinuteOfHour = m_Time.GetDateInfo().MinuteOfHour;
			if( MinuteOfHour != 0 )
			{
				m_Time.AddMinutes( exTime::MINUTES_IN_HOUR - MinuteOfHour );
			}

			eg_uint HourOfDay = m_Time.GetDateInfo().HourOfDay;
			if( HourOfDay >= ExGlobalData::Get().GetRestTilMorningTime() )
			{
				m_RestHoursLeft = exTime::HOURS_IN_DAY - HourOfDay + ExGlobalData::Get().GetRestTilMorningTime();
			}
			else
			{
				m_RestHoursLeft = ExGlobalData::Get().GetRestTilMorningTime() - HourOfDay;
			}
		}

		ComputePerHourRestorations();
		m_StartRestingHours = m_RestHoursLeft;

		// Decide if there will be a combat encounter while resting
		m_bCombatEncounterAfterRest = false;
		m_RestHoursUntilAmbush = 0;
		if( CanHaveEncounterHereWhileResting() )
		{
			const eg_uint MinRestHoursForAmbush = ExGlobalData::Get().GetMinRestHoursForRandomCombat();

			// Encounter probabilty comes from map info.
			const ex_attr_value PartyMinLevel = GetPartyMinTrainToLevel();
			const eg_real EncounterPct = GetCurrentMapInfo().EncounterData.RestingEncounterPercent;
			const eg_real RandomPct = m_Rng.GetRandomRangeF( 0.f , 100.f );
			const eg_bool bDoEncounter = RandomPct < EncounterPct; // Less than because if EncounterPct is 0 and RandomPct is 0 we don't want an encounter.

			if( m_bHadAmbushOnLastRest )
			{
				m_bCombatEncounterAfterRest = false;
			}
			else
			{
				m_bCombatEncounterAfterRest = bDoEncounter;
			}

			if( EX_CHEATS_ENABLED && ExGame_AlwaysDoCombatAfterResting.GetValueThreadSafe() )
			{
				m_bCombatEncounterAfterRest = true;
			}
			if( m_RestHoursLeft < MinRestHoursForAmbush )
			{
				m_bCombatEncounterAfterRest = false;
			}
			if( m_bCombatEncounterAfterRest )
			{
				m_RestHoursUntilAmbush = m_Rng.GetRandomRangeI( MinRestHoursForAmbush , m_RestHoursLeft-1 );
			}

			m_bHadAmbushOnLastRest = m_bCombatEncounterAfterRest;

			EGLogf( eg_log_t::Verbose , "Resting chance for encounter: %g/%g %s (happens in %d hours)" , EncounterPct , RandomPct , m_bCombatEncounterAfterRest ? "Encounter!" : "Safe" , m_RestHoursUntilAmbush );
		}

		SDK_ReplicateToClient( &m_bCombatEncounterAfterRest , sizeof(m_bCombatEncounterAfterRest) );
		SDK_ReplicateToClient( &m_StartRestingHours , sizeof(m_StartRestingHours) );
		SDK_ReplicateToClient( &m_RestHoursLeft , sizeof(m_RestHoursLeft) );
		SDK_RunClientEvent( eg_crc("BeginRestMenu") );
	}
	else
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Tried to rest when it wasn't allowed." );
		assert( false );
	}
}

void ExGame::HandleLoadingComplete()
{
	assert( IsClient() );

	if( m_Mapping )
	{
		m_Mapping->RefreshMap();
	}
}

void ExGame::RemoveMonster( eg_ent_id MonsterId )
{
	eg_uint FoundIndex = MAX_MAP_MONSTERS;
	for( eg_uint i = 0; MAX_MAP_MONSTERS == FoundIndex && i < m_MonsterList.Len(); i++ )
	{
		if( m_MonsterList[i] == MonsterId )
		{
			FoundIndex = i;
		}
	}

	if( FoundIndex != MAX_MAP_MONSTERS )
	{
		assert( m_MonsterList.HasItems() );
		m_MonsterList.DeleteByIndex( FoundIndex );
	}
	else
	{
		EGLogf( eg_log_t::GameLib , __FUNCTION__ ": Failed to remove monster (new game?)" );
		//assert( false ); // This monster was not in the list.
	}
}

void ExGame::AddTrackedIoEnt( ExIoEnt* InIoEnt , const EGNavGraphVertex& Location , ex_face Face )
{
	exTrackedIoEnt NewTrackedIo;
	NewTrackedIo.IoEnt = InIoEnt;
	NewTrackedIo.NavVertex = Location;
	NewTrackedIo.Face = Face;

#if defined( __DEBUG__ )
	for( eg_uint i=0; i<m_TrackedIoEnts.Len(); i++ )
	{
		if( m_TrackedIoEnts[i].IoEnt.GetObject() == InIoEnt )
		{
			assert( false ); // This IO was already added.
		}
	}
#endif
	m_TrackedIoEnts.Push( NewTrackedIo );
}

void ExGame::RemoveTrackedIoEnt( ExIoEnt* InIoEnt )
{
	for( eg_uint i=0; i<m_TrackedIoEnts.Len(); i++ )
	{
		if( m_TrackedIoEnts[i].IoEnt.GetObject() == InIoEnt )
		{
			m_TrackedIoEnts.DeleteByIndex( i );
			break;
		}
	}
}

void ExGame::SetChestOpen( eg_string_crc TreasureCrc )
{
	m_TreasureState.AppendUnique( TreasureCrc );
}

eg_bool ExGame::IsChestOpen( eg_string_crc TreasureCrc ) const
{
	return m_TreasureState.Contains( TreasureCrc );
}

ex_face ExGame::StringToFace( eg_cpstr StrFace )
{
	ex_face Out = ex_face::UNK;

	eg_string StrFaceStr( StrFace );
	StrFaceStr.ConvertToUpper();
	eg_string_crc FaceCrc = eg_string_crc(StrFaceStr);
	switch_crc( FaceCrc )
	{
		case_crc("NORTH"):
		{
			Out = ex_face::NORTH;
		} break;
		case_crc("EAST"):
		{
			Out = ex_face::EAST;
		} break;
		case_crc("SOUTH"):
		{
			Out = ex_face::SOUTH;
		} break;
		case_crc("WEST"):
		{
			Out = ex_face::WEST;
		} break;
		case_crc("ALL"):
		{
			Out = ex_face::ALL;
		} break;
		case_crc(""):
		case_crc("NONE"):
		{
			Out = ex_face::NONE;
		} break;
		default:
		{
			assert( false ); //Not valid!
		} break;
	}

	return Out;
}

ex_face ExGame::PoseToFace( const eg_transform& Pose )
{
	ex_face Out = ex_face::UNK;

	eg_vec4 Face(0.f,0.f,1.f,0.f);
	Face *= Pose;
	Face.NormalizeThisAsVec3();

	eg_real NSDot = Face.Dot( eg_vec4(0.f,0.f,1.f,0.f) );
	eg_real EWDot = Face.Dot( eg_vec4(1.f,0.f,0.f,0.f) );

	const eg_real Eps = .01f;

	if( EG_IsEqualEps( NSDot , 1.f, Eps ) )
	{
		Out = ex_face::NORTH;
	}
	else if( EG_IsEqualEps( NSDot , -1.f , Eps ) )
	{
		Out = ex_face::SOUTH;
	}
	else if( EG_IsEqualEps( EWDot , 1.f , Eps ) )
	{
		Out = ex_face::EAST;
	}
	else if( EG_IsEqualEps( EWDot , -1.f , Eps ) )
	{
		Out = ex_face::WEST;
	}
	else if( EG_IsEqualEps( NSDot , 0.f , Eps ) && EG_IsEqualEps( EWDot , 0.f , Eps ) )
	{
		Out = ex_face::NONE; // Laying on it's back somehow or something?
	}

	return Out;
}

eg_transform ExGame::FaceToTransform( ex_face Face )
{
	eg_transform Out;
	switch( Face )
	{
	default:
		assert( false );
		// fallthru:
	case ex_face::NORTH: Out = eg_transform::BuildRotationY( EG_Deg( 0.f ) ); break;
	case ex_face::EAST: Out = eg_transform::BuildRotationY( EG_Deg( 90.f ) ); break;
	case ex_face::SOUTH: Out = eg_transform::BuildRotationY( EG_Deg( 180.f ) ); break;
	case ex_face::WEST: Out = eg_transform::BuildRotationY( EG_Deg( 270.f ) ); break;
	}
	return Out;
}

ex_face ExGame::GetReverseFace( ex_face FaceIn )
{
	switch( FaceIn )
	{
		case ex_face::UNK: return ex_face::UNK;
		case ex_face::NORTH: return ex_face::SOUTH;
		case ex_face::EAST: return ex_face::WEST;
		case ex_face::SOUTH: return ex_face::NORTH;
		case ex_face::WEST: return ex_face::EAST;
		case ex_face::ALL: return ex_face::NONE;
		case ex_face::NONE: return ex_face::ALL;
	}

	return ex_face::UNK;
}

ex_face ExGame::GcxDirToExFace( gcx_direction DirIn )
{
	switch( DirIn )
	{
		case gcx_direction::UNKNOWN: return ex_face::NONE;
		case gcx_direction::UP: return ex_face::NORTH;
		case gcx_direction::RIGHT: return ex_face::EAST;
		case gcx_direction::DOWN: return ex_face::SOUTH;
		case gcx_direction::LEFT: return ex_face::WEST;
		case gcx_direction::ALL: return ex_face::ALL;
		case gcx_direction::NONE: return ex_face::UNK;
	}

	return ex_face::UNK;
}

gcx_marker_t ExGame::GetTargetMarkerFromSourceMarker( gcx_marker_t Source )
{
	switch( Source )
	{
		case gcx_marker_t::START: return gcx_marker_t::EXIT;
		case gcx_marker_t::EXIT: return gcx_marker_t::START;
		case gcx_marker_t::STAIRS_UP: return gcx_marker_t::STAIRS_DOWN;
		case gcx_marker_t::STAIRS_DOWN: return gcx_marker_t::STAIRS_UP;
		case gcx_marker_t::LADDER_UP: return gcx_marker_t::LADDER_DOWN;
		case gcx_marker_t::LADDER_DOWN: return gcx_marker_t::LADDER_UP;
		default:
			assert( false ); // Not a supported marker type.
			break;
	}
	return gcx_marker_t::NONE;
}
