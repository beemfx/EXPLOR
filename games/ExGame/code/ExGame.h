// (c) 2016 Beem Media. All Rights Reserved.

#pragma once

#include "EGGame.h"
#include "ExGameTypes.h"
#include "ExCore.h"
#include "EGNavGraph.h"
#include "ExInput.h"
#include "EGSmRuntime.h"
#include "ExRoster.h"
#include "ExTime.h"
#include "ExDayNightCycles.h"
#include "EGDelegate.h"
#include "EGWeakPtr.h"
#include "EGGcxEnums.h"
#include "EGRandom.h"
#include "EGParse.h"
#include "EGTransformSpline.h"
#include "EGNetSmoothTransform.h"
#include "EGTimer.h"

class ExEnt;
class ExIoEnt;
class ExMapNavBase;
class ExPlayerController;
class ExEnemyController;
class ExScriptMenu;
class ExConversationMenu;
class ExHUD;
class ExMenu;
class ExSkills;
class ExMapping;
struct exMapInfoData;
class ExTreasureChest;
class ExCombat;
class ExMapIconComponent;
class ExMapTransitionEnt;
class ExCameraSplineNodeEnt;

typedef EGFixedArray<eg_ent_id,MAX_MAP_MONSTERS> ExMapMonsterList;
typedef EGFixedArray<exRawItem,MAX_BUYBACK_ITEMS> ExBuybackList;
typedef EGFixedArray<exRawItem,32> ExShopInventory;

class ExGame : public EGGame , public ISmRuntimeHandler
{
	EG_CLASS_BODY( ExGame , EGGame )

friend class ExServer;

public:
	
	static const eg_uint32 SAVE_ID = 0x58534156; // "XSAV"
	static const eg_uint32 SAVE_VERSION; // Increment this when format changes (and size did not)

	static const eg_int MAX_GAME_INT = 1000000000;

	struct exTranslatorInfo
	{
		eg_string_crc Message = CT_Clear;
		eg_uint PartyMemberIndex = ExRoster::PARTY_INVALID_INDEX;
	};
	
	struct exPartyStatus
	{
		eg_uint   Turn;
		eg_uint   SelectedPartyMember;
	};

	struct exMapTransitionEnt
	{
		EGWeakPtr<ExMapTransitionEnt> Ent;
		EGNavGraphVertex NavVertex = CT_Clear;
		ex_face SpawnFace = ex_face::UNK;
		ex_face InteractFace = ex_face::UNK;

		eg_bool operator == ( const exMapTransitionEnt& CompareTo ) const { return Ent == CompareTo.Ent; }
	};

	struct exTrackedIoEnt
	{
		EGWeakPtr<ExIoEnt> IoEnt;
		EGNavGraphVertex   NavVertex = CT_Clear;
		ex_face            Face;
	};

	struct exDefeatedBoss
	{
		eg_string_crc    MapId = CT_Preserve;
		EGNavGraphVertex NavVertex = CT_Preserve;

		eg_bool operator == ( const exDefeatedBoss& Right ) const { return MapId == Right.MapId && NavVertex == Right.NavVertex; }
	};

	typedef EGFixedArray<exDefeatedBoss,MAX_DEFATED_BOSSES> ExDefatedBossList;

	struct exTransitTarget
	{
		eg_string_crc TargetMapId;
		eg_string_crc TargetEntranceId;
		gcx_marker_t  TargetAutoMarker;
		eg_bool       bIsTownPortalRecall;

		exTransitTarget() = default;
		exTransitTarget( eg_ctor_t Ct )
		: TargetMapId( Ct )
		, TargetEntranceId( Ct )
		{
			if( Ct == CT_Clear || Ct == CT_Default )
			{
				TargetAutoMarker = gcx_marker_t::NONE;
				bIsTownPortalRecall = false;
			}
		}
	};

	struct exTownPortalRecallMarker
	{
		eg_string_crc TargetMapId;
		eg_transform  TargetPose;
		eg_bool       bRecallWasCast;
		eg_bool       bClearTownPortalAfterSave = false; // Not serialized

		exTownPortalRecallMarker() = default;
		exTownPortalRecallMarker( eg_ctor_t Ct )
		: TargetMapId( Ct )
		, TargetPose( Ct )
		{
			if( Ct == CT_Default || Ct == CT_Clear )
			{
				bRecallWasCast = false;
			}
		}
	};

	enum class ex_game_s
	{
		NONE,
		CREATED_NEW_GAME,
		MOVING,
		COMBAT,
		DEAD, // Player is dead, nothing to do but reload
		TRANS_MAP, // Actually transitioning
		TRANS_LOADING,
		RUNNING_SCRIPT,
		TREASURE_CHEST,
		TREASURE_CHEST_RESULTS,
		RESTING,
	};

	enum class ex_script_yield_reason
	{
		NONE,
		CONVERSATION_CHOICE,
		NPC_GOSSIP,
		MENU_RESPONSE,
		GAME_MENU,
	};

	struct exRepConvState
	{
		eg_string_crc     Speaker;
		eg_string_crc     CurDialog;
		EGSmBranchOptions Choices;

		void Clear()
		{
			Speaker = CT_Clear;
			CurDialog = CT_Clear;
			Choices.Clear();
		}
	};

	enum class ex_camera_mode
	{
		PlayerView ,
		CameraSpline ,
	};

private:

	eg_uint32               m_SaveId;
	eg_uint32               m_SaveVersion;
	eg_size_t               m_ExGameSize;
	ex_save_name            m_SaveName;
	EGTimer::egSystemTime   m_SaveTime;
	EGRandom                m_Rng = CT_Preserve;
	eg_uint                 m_SaveSlot;
	ExRoster                m_Roster;
	ExMapMonsterList        m_MonsterList; // Not serialized
	ExDefatedBossList       m_DefeatedBossList = CT_Preserve; // Serialized.
	EGArray<exMapTransitionEnt> m_MapTransitionEnts;
	EGArray<exTrackedIoEnt> m_TrackedIoEnts;      // Not serialized
	EGArray<EGNavGraphVertex> m_SafeSpots; // Not serialized
	EGArray<ExMapIconComponent*> m_TrackedMapIcons;
	eg_ent_id               m_PlayerId;
	eg_ent_id               m_PlayerTargetedEnt = INVALID_ID; // Not serialized
	eg_real                 m_TurnTime; //Number of seconds for the turn tween.
	eg_int                  m_SafeTurns;
	eg_int                  m_TotalRandomEncounters;
	ex_game_s               m_GameStateInternal;
	ex_script_yield_reason  m_MapScriptYieldReason;
	eg_bool                 m_bMapScriptIsInConversation:1;
	eg_bool                 m_bMapScriptInGameMenu:1;
	eg_bool                 m_bMapScriptIsInDialogBox:1;
	eg_bool                 m_bMapScriptInScriptMenu:1;
	eg_ent_id               m_MapEventTreasureChest;
	eg_bool                 m_bHasInnSave:1;
	eg_bool                 m_bIsSave:1;
	eg_bool                 m_bNeedsInitialSave:1;
	eg_bool                 m_bSaveOnNextUpdate:1;
	eg_bool                 m_bRestOnNextUpdate:1;
	eg_bool                 m_bDontShowPopupForNextSave:1;
	eg_bool                 m_bIgnoreTreasureUntilNextTurn:1;
	ex_rest_t               m_NextRestType;
	eg_bool                 m_bHandleEncountersAfterScript:1;
	eg_bool                 m_bHasPropperSpawnLocation:1;
	eg_bool                 m_bRecallTownPortalQueued:1;
	eg_string_crc           m_PartyStatusChecksum;
	EGNavGraph              m_NavGraph = CT_Preserve;
	exTownPortalRecallMarker m_TownPortalRecallMarker = CT_Preserve;
	eg_string_crc           m_NextCombatEncounterId;
	exTransitTarget         m_TransitTarget;
	eg_string_crc           m_QueuedWorldMapTransition;
	eg_transform            m_PlayerSpawnLocation;
	EGTreasureList          m_TreasureState;
	eg_string_crc           m_CurrentMapId;
	exPartyStatus           m_PartyStatus;
	eg_ent_id               m_LastDoorOpenedByPlayer;
	exTranslatorInfo        m_ActiveTranslationText;
	ExGameVarSearchMap      m_GameStateVars; // Game state vars are replicated differently from other variables
	ExShopInventory         m_ShopInventory;
	ExBuybackList           m_BuybackItems;
	exTime                  m_Time;
	exTimeSmoother          m_TimeSmoother;
	eg_uint                 m_StartRestingHours;
	eg_uint                 m_RestHoursLeft;
	eg_uint                 m_RestHoursUntilAmbush;
	eg_real                 m_RestTimer;
	eg_bool                 m_bCombatEncounterAfterRest;
	eg_bool                 m_bHadAmbushOnLastRest;
	ex_attr_value           m_PartyMemberRestHPPerHour[ExRoster::PARTY_SIZE];
	ex_attr_value           m_PartyMemberRestMPPerHour[ExRoster::PARTY_SIZE];
	eg_bool                 m_bIgnoreScriptEncounter;
	ex_map_reveal_result    m_LastMapRevealResult;
	ex_encounter_disposition m_EncounterDisposition;
	eg_int                  m_FrontLineSize;
	eg_int                  m_ShopSeedPartyLevel;
	eg_int                  m_SeedDay;
	eg_uint                 m_ShopSeed;
	eg_uint                 m_NpcGossipSeed;

	egsmState               m_CurScriptState;
	exRepConvState          m_CurConvState;
	exRepConvState          m_CurYieldForMenuState;
	EGWeakPtr<ExEnemyController> m_CurrentCombatEntity;
	EGWeakPtr<ExTreasureChest> m_TreasureChestToDelete;
	exTreasureChestResults m_LastTreasureChestContents;

	EGArray<exDayNightCycleState> m_DayNightCycle; // Not serialized
	ExSkills* m_Skills = nullptr;
	ExMapping* m_Mapping = nullptr;
	EGWeakPtr<ExCombat> m_CurrentCombat;

	// Camera Spline:
	ex_camera_mode             m_CameraMode = ex_camera_mode::PlayerView;
	EGArray<EGTransformSpline> m_CameraSplines;
	eg_int                     m_ActiveCameraSplineIndex = -1;
	egNetSmoothTransform       m_CameraSplineRepPose = CT_Default;

	EGRemoteEventHandler<ExGame> m_BhHandler;

	// Open Menus:
	ExConversationMenu* m_ConversationMenu = nullptr;
	ExScriptMenu*       m_ScriptMenu = nullptr;
	eg_string_crc       m_ScriptMenuType;
	EGWeakPtr<ExMenu>   m_GameMenu = nullptr;
	EGArray<EGWeakPtr<ExEnt>> m_ClientPCs;
	EGArray<eg_lockstep_id> m_ClientsInWorld;

	EGObject* m_PatchData = nullptr; // In case new data needs to be added for patch...

public:

	EGSimpleMCDelegate OnClientRosterChangedDelegate;
	EGSimpleMCDelegate OnClientNotifyWaitDelegate;
	EGSimpleMCDelegate OnClientNotifyTurnStartDelegate;
	EGSimpleMCDelegate OnClientNotifyTurnEndDelegate;
	EGSimpleMCDelegate OnClientTranslationChangedDelegate;
	EGSimpleMCDelegate OnClientDispositionChangedDelegate;
	EGSimpleMCDelegate OnClientGameStateChangedDelegate;
	EGSimpleMCDelegate OnTrackedMapIconsChanged;
	EGMCDelegate<eg_string_crc,const egsm_var&> OnServerGameVarChanged;

public:

	ExGame();
	virtual ~ExGame() override final { }
	virtual void Init( EGEntWorld* EntWorldOwner ) override final;
	virtual void OnDestruct() override final;

	eg_uint32 GetSaveId() const { return m_SaveId; }
	eg_uint32 GetSaveVersion() const { return m_SaveVersion; }
	eg_d_string16 GetSaveName() const { return m_SaveName; }
	const EGTimer::egSystemTime& GetSaveTime() const { return m_SaveTime; }

	const exRepConvState& GetYieldForMenuState() const { return m_CurYieldForMenuState; }

	void LoadMap( eg_string_crc MapId );
	void DebugLoadMapByWoldFilename( eg_cpstr WorldFilename );
	void BeginNewGame( const exPackedCreateGameInfo& NewGameData );
	void BeginLostGame();

	virtual void Update( eg_real DeltaTime ) override final;
	virtual void OnSaveGameLoaded() override final;
	virtual void OnMapLoaded( const EGGameMap* GameMap ) override final;
	virtual void PostSave( EGFileData& GameDataOut ) override final;
	virtual void PostLoad( const EGFileData& GameDataIn ) override final;
	virtual void OnDataReplicated( const void* Offset , eg_size_t Size ) override;

	virtual void OnClientEnterWorld( const eg_lockstep_id& LockstepId ) override final;
	virtual void OnClientLeaveWorld( const eg_lockstep_id& LockstepId ) override final;

	void NativeEventQuitFromInn();

	void Server_HandleEvent( const eg_lockstep_id& SenderId , const egRemoteEvent& Event );
	void Client_HandleEvent( const egRemoteEvent& Event );
	void BhOnBeginCombat( const eg_event_parms& Parms );
	void BhBeginRestMenu( const eg_event_parms& Parms );
	void BhOnRosterRequestComplete( const eg_event_parms& Parms );
	void BhOnPartyChanged( const eg_event_parms& Parms );
	void BhOnRosterChanged( const eg_event_parms& Parms );
	void BhDoSave( const eg_event_parms& Parms );
	void BhOnSendInitialRepData( const eg_event_parms& Parms  );
	void BhOnTurnStart( const eg_event_parms& Parms );
	void BhOnPlayerRotated( const eg_event_parms& Parms );
	void BhOnTurnEnd( const eg_event_parms& Parms );
	void BhOnCombatResults( const eg_event_parms& Parms );
	void RunScript( eg_string_crc InScriptId , eg_string_crc InEntryPoint );
	void BhOnStartConversation( const eg_event_parms& Parms );
	void BhOnConversationChoiceProcessed( const eg_event_parms& Parms );
	void BhSellInventoryItem( const eg_event_parms& Parms );
	void BhSetHandleEncounterAfterScript( const eg_event_parms& Parms );

	// Treasure chest events:
	void BhBeginTreasureChestInteraction( const eg_event_parms& Parms );
	void BhDoTreasureChestOpening( const eg_event_parms& Parms );
	void BhEndTreasureChestIntraction( const eg_event_parms& Parms );
	void BhClosedTreasureDialog( const eg_event_parms& Parms );

	void BhOnScriptDialogBox( const eg_event_parms& Parms );
	void BhOnScriptDialogBoxChoiceMade( const eg_event_parms& Parms );
	void BhOnConversationClear( const eg_event_parms& Parms );
	void BhOnConversationChoice( const eg_event_parms& Parms );
	void BhRefreshConversationNameplate( const eg_event_parms& Parms );
	void BhOpenScriptMenu( const eg_event_parms& Parms );
	void BhOnScriptMenuChoiceProcessed( const eg_event_parms& Parms );
	void BhOnScriptMenuChoice( const eg_event_parms& Parms );
	void BhCloseScriptMenu( const eg_event_parms& Parms );
	void BhSaveFromInn( const eg_event_parms& Parms );
	void BhRosterMenuRemovePartyMember( const eg_event_parms& Parms );
	void BhRosterMenuExchangePartyMembers( const eg_event_parms& Parms );
	void BhRosterMenuAddPartyMemberToSlot( const eg_event_parms& Parms );
	void BhRosterMenuDeleteRosterMember( const eg_event_parms& Parms );
	void BhAttuneSkill( const eg_event_parms& Parms );
	void BhBeginRepFullGameState( const eg_event_parms& Parms );
	void BhReplicateGameVar( const eg_event_parms& Parms );
	void BhReplicateUnlockedSkill( const eg_event_parms& Parms );
	void BhEndRepFullGameState( const eg_event_parms& Parms );
	void BhRest( const eg_event_parms& Parms );
	void BhDropGold( const eg_event_parms& Parms );
	void BhOpenGameMenu( const eg_event_parms& Parms );
	void BhOnGameMenuClosed( const eg_event_parms& Parms );
	void BhTrainPartyMember( const eg_event_parms& Parms );
	void BhRefreshGameMenu( const eg_event_parms& Parms );
	void BhOnItemReplicated( const eg_event_parms& Parms );
	void BhSetSelectedPartyMember( const eg_event_parms& Parms );
	void BhClientHandlePartyMemberSwitched( const eg_event_parms& Parms );
	void BhEquipInventoryItem( const eg_event_parms& Parms );
	void BhSwapInventoryItem( const eg_event_parms& Parms );
	void BhDropInventoryItem( const eg_event_parms& Parms );
	void BhRevealMapSquare( const eg_event_parms& Parms );
	void BhMakePurchase( const eg_event_parms& Parms );
	void BhUnlockDoor( const eg_event_parms& Parms );
	void BhQueueWorldMapTransition( const eg_event_parms& Parms );
	void BhServerOpenContextMenu( const eg_event_parms& Parms );
	void BhClientNotifyWait( const eg_event_parms& Parms );
	void BhClientNotifyTurnStart( const eg_event_parms& Parms );
	void BhClientNotifyTurnEnd( const eg_event_parms& Parms );
	void BhServerDoTownPortal( const eg_event_parms& Parms );
	void BhQuitGame( const eg_event_parms& Parms );
	void BhClientNotifyEvent( const eg_event_parms& Parms );
	void BhServerNotifyEvent( const eg_event_parms& Parms );
	void BhPlayCameraSpline( const eg_event_parms& Parms );
	void BhPlaySound( const eg_event_parms& Parms );

	void Server_OnConsoleCmd( eg_cpstr Cmd );
	
	void BeginStateMachine( eg_string_crc ProcId , eg_string_crc EntryPoint );
	void ResumeStateMachine( eg_string_crc Input );
	void StateMachineFollowUp();

	void SetActiveTranslationText( const eg_string_crc& NewText , eg_uint PartyMemberIndex = ExRoster::PARTY_INVALID_INDEX );
	void ClearActiveTranslationText() { SetActiveTranslationText( CT_Clear ); }
	const exTranslatorInfo& GetActiveTranslationText() const { return m_ActiveTranslationText; }

	void ResetStuffForPlayerMoving();

	void SetLastTreasureChestContents( const exTreasureChestResults& Contents );
	const exTreasureChestResults& GetLastTreasureChestContents() const { return m_LastTreasureChestContents; }
	void ShowTreasureChestRewards();
	eg_bool IsIgnoringNextChestEncounter() const { return m_bIgnoreTreasureUntilNextTurn; }
	void SetIgnoreNextScriptEncounter() { m_bIgnoreScriptEncounter = true; }

	void BeginResting();
	void HandleLoadingComplete();

	void HandleHighestPriorityEncounter( eg_bool bAllowRandomEncounter );
	eg_bool CanHaveEncounterHereWhileResting() const;
	void BeginRandomCombat();
	void RevealAutomap( const eg_transform& Pose );
	void BeginCombat( const eg_string_crc& EncounterId );

	eg_ent_id GetPrimaryPlayer()const{ return m_PlayerId; }
	void      SetPrimaryPlayer( eg_ent_id Id ){ m_PlayerId = Id; }

	eg_bool IsInCombat() const { return GetGameState() == ex_game_s::COMBAT; }
	eg_bool IsResting() const { return GetGameState() == ex_game_s::RESTING; }
	eg_string_crc GetCurrentMapId() const { return m_CurrentMapId; }
	eg_ent_id GetLastDoorOpenedByPlayer() const { return m_LastDoorOpenedByPlayer; }
	void SetLastDoorOpenedByPlayer( eg_ent_id val ) { m_LastDoorOpenedByPlayer = val; }
	eg_uint GetSelectedPartyMemberIndex() const { return m_PartyStatus.SelectedPartyMember; }
	const ExFighter* GetBestThiefOrSelectedPartyMember() const;
	const ExFighter* GetSelectedPartyMember() const;
	eg_uint GetRestHoursLeft() const { return m_RestHoursLeft; }
	eg_uint GetStartingRestHours() const { return m_StartRestingHours; }
	eg_real GetTurnTime()const{ return m_TurnTime; }
	exTime GetTime()const{ return m_Time; }
	eg_real GetNormalizedTimeOfDay() const;
	eg_bool CanPlayerMove()const{ return GetGameState() == ex_game_s::NONE || GetGameState() == ex_game_s::MOVING; } // If we are in a map event we can't move.
	ExEnt* GetEnt( eg_ent_id Id );
	const ExEnt* GetEnt( eg_ent_id Id ) const;
	ExPlayerController* GetPlayerEnt();
	const ExPlayerController* GetPlayerEnt() const;
	const eg_string_crc& GetNextCombatEncounter() const { return m_NextCombatEncounterId; }
	void SetSkillUnlocked( eg_string_crc SkillId , eg_bool bUnlocked );
	ExSkills* GetSkillsModule() const { return m_Skills; }
	ExMapping* GetMappingModule() const { return m_Mapping; }
	void SetPlayerTargetedEnt( eg_ent_id NewTarget );
	eg_ent_id GetPlayerTargetedEnt() const { return m_PlayerTargetedEnt; }
	ex_encounter_disposition GetEncounterDisposition() const { return m_EncounterDisposition; }
	eg_int GetFrontLineSize() const { return m_FrontLineSize; }
	eg_int GetMinFrontLineSize() const;
	eg_int GetEnemyFrontLineSize() const;
	eg_int ComputeNumVisitableSquares() const;

	void QueueSpawn( eg_string_crc Type , const eg_transform& Pose , eg_cpstr InitString );
	void SaveGame( eg_bool bReversePlayerPose = false );
	void AttemptAutoSave( eg_bool bImmediate );
	void AddMonster( eg_ent_id MonsterId );
	void RemoveMonster( eg_ent_id MonsterId );
	void AddTrackedIoEnt( ExIoEnt* InIoEnt , const EGNavGraphVertex& Location , ex_face Face );
	void RemoveTrackedIoEnt( ExIoEnt* InIoEnt );
	void SetChestOpen( eg_string_crc TreasureCrc );
	eg_bool IsChestOpen( eg_string_crc TreasureCrc )const;

	static ex_face StringToFace( eg_cpstr StrFace );
	static ex_face PoseToFace( const eg_transform& Pose );
	static eg_transform FaceToTransform( ex_face Face );
	static ex_face GetReverseFace( ex_face FaceIn );
	static ex_face GcxDirToExFace( gcx_direction DirIn );
	static gcx_marker_t GetTargetMarkerFromSourceMarker( gcx_marker_t Source );

	static eg_bool IsFacing( ex_face Face1 , ex_face Face2 );

	ExRoster& GetRoster(){ return m_Roster; }
	const ExRoster& GetRoster() const { return m_Roster; }
	ExFighter* GetPartyMemberByIndex( eg_uint Index ){ return m_Roster.GetPartyMemberByIndex( Index ); }
	const ExFighter* GetPartyMemberByIndex( eg_uint Index ) const { return m_Roster.GetPartyMemberByIndex( Index ); }
	const ExFighter* GetPartyMemberInCombatByIndex( eg_uint Index ) const;
	const ExFighter* GetRosterCharacterByIndex( eg_uint Index ) const { return m_Roster.GetRosterCharacterByIndex( Index ); }
	ex_attr_value GetPartyMinLevel() const;
	ex_attr_value GetPartyMaxLevel() const;
	eg_uint GetShopSeed() const { assert( 0 != m_ShopSeed ); return m_ShopSeed; }
	eg_int GetShopSeedPartyLevel() const { return m_ShopSeedPartyLevel; }
	ex_xp_value GetPartyMinXP() const;
	ex_xp_value GetPartyMaxXP() const;
	ex_attr_value GetPartyMinTrainToLevel() const;
	ex_attr_value GetPartyMaxTrainToLevel() const;
	eg_int GetNumMapSquaresRevealed() const;

	void SetCurrentCombat( ExCombat* InCombat );
	const ExCombat* GetCombat() const;

	egsm_var& GetGameVar( eg_string_crc Id );
	const egsm_var& GetGameVar( eg_string_crc Id ) const;
	eg_bool HasGameVar( eg_string_crc Id ) const;

	eg_string_crc GetCurScriptDialog() const { return m_CurConvState.CurDialog; }
	eg_string_crc GetCurScriptSpeaker() const { return m_CurConvState.Speaker; }
	void GetCurScriptChoices( EGSmBranchOptions& OptionsOut ) const;

	const exMapInfoData& GetCurrentMapInfo() const;

	ExMenu* GetGameMenu();
	ExMenu* SwitchGameMenu( eg_string_crc MenuId );

	const EGArray<exDayNightCycleState>& GetCurrentDayNightCycle() const;

	void GetInventoryItems( EGArray<exInventoryItem>& Out ) const;

	eg_angle GetPlayerRotation() const;

	ex_attr_value GetPartyGold() const;
	void SetPartyGold( ex_attr_value NewValue );

	ex_can_buy_t CanBuyItem( const exInventoryItem& Item , eg_bool bIsBuyback ) const;
	const ExBuybackList& GetBuybackList() const { return m_BuybackItems; }
	const ExShopInventory& GetShopInventory( const eg_string_crc& InventoryId ) const { unused( InventoryId ); return m_ShopInventory; }

	void UnlockTargetedDoor();
	void QueueWorldMapTransitionFromClient( eg_string_crc TargetMapId );

	eg_int CalculateCostToHealParty() const;

	eg_bool IsSaveAndQuit() const;

	void DebugBalanceParty( ex_attr_value Level );
	void DebugSimulateCombatEncounters( eg_int NumEncounters , eg_int NumBosses );
	void DebugProcessEncounter( eg_string_crc EncounterId , ex_attr_value* GoldOut , ex_xp_value* XpOut );

	ex_game_s GetGameState() const { return m_GameStateInternal; }

	void SetTrackedMapIcon( ExMapIconComponent* InNPC , eg_bool bTrack );
	const EGArray<ExMapIconComponent*>& GetTrackedMapIcons() const { return m_TrackedMapIcons; }

	void SetTrackedMapTransitionEnt( ExMapTransitionEnt* InEnt , eg_bool bTrack );

	void SetCameraSplineNode( ExCameraSplineNodeEnt* InSplineNode , eg_bool bAdd );
	void PlayCameraSpline( eg_string_crc SplineDataId );
	void StopCameraSpline();

	eg_transform GetCameraPose() const;

	bool IsBossDefeated( const exDefeatedBoss& BossInfo ) const;

private:

	void SetGameState( ex_game_s NewGameState );

	void InitGameVars();
	void GeneratedNewSeeds();

	void Update_AsServer( eg_real DeltaTime );
	void Update_AsClient( eg_real DeltaTime );

	virtual void FormatText( eg_cpstr Flags , class EGTextParmFormatter* Formatter ) const override final;

	template< typename T >
	T* FindIoOnVertex( const EGNavGraphVertex& Vertex )
	{
		for( exTrackedIoEnt& TrackedIo : m_TrackedIoEnts )
		{
			if( TrackedIo.NavVertex == Vertex )
			{
				if( TrackedIo.IoEnt && TrackedIo.IoEnt->IsA( &T::GetStaticClass() ) )
				{
					return EGCast<T>( TrackedIo.IoEnt.GetObject() );
				}
			}
		}
		return nullptr;
	}

	void CalmHostileEnemies();
	void MovePlayerToEscapeSquare();

	ExHUD* GetHUD();

	void SpawnClients();

public:

	void ClientCreateRosterCharacter( eg_uint RosterIndex , const ExFighter& CreateCharacter );
	void ClientUpdateRosterCharacterByPartyIndex( eg_uint RosterIndex, const ExFighter& Character );

	void CheckForGameMenuClosed();

	virtual void SmOnError( eg_cpstr ErrorString ) override final;
	virtual egsm_var SmGetVar( eg_string_crc VarId ) const override final;
	virtual eg_bool SmDoesVarExist( eg_string_crc VarName ) const override final;
	virtual void SmSetVar( eg_string_crc VarId , const egsm_var& Value ) override final;
	virtual egsmNativeRes SmOnNativeEvent( eg_string_crc EventName , const EGArray<egsm_var>& Parms , const EGArray<eg_string_crc>& Branches ) override final;
};


struct exDoubleIndex
{
	eg_uint32 Index0;
	eg_uint32 Index1;

	exDoubleIndex( eg_uint32 In0 , eg_uint32 In1 ): Index0(In0) , Index1(In1){ }
	explicit exDoubleIndex( eg_int64 In64Bit )
	{
		static_assert( sizeof(*this) == sizeof(In64Bit) , "Must be same size" );
		EGMem_Copy( this , &In64Bit , sizeof(*this) );
	}

	eg_event_parms ToEventParm()
	{
		eg_int64 AsInt64;
		EGMem_Copy( &AsInt64, this , sizeof(AsInt64) );
		return AsInt64;
	}
};
