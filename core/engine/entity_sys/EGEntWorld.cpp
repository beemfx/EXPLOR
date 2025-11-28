// (c) 2017 Beem Media

#include "EGEntWorld.h"
#include "EGEnt.h"
#include "EGEntDict.h"
#include "EGPhysSim2.h"
#include "EGPhysBodyComponent.h"
#include "EGSaveGame.h"
#include "EGFileData.h"
#include "EGNetMsgs.h"
#include "EGNetMsgBuffer.h"
#include "EGEntWorldMap.h"
#include "EGEntWorldTerrain.h"
#include "EGGame.h"
#include "EGEntWorldRegion.h"
#include "EGGameMap.h"
#include "EGGameTerrain2.h"
#include "EGDebugText.h"
#include "EGCamera2.h"
#include "EGRenderer.h"
#include "EGDebugShapes.h"
#include "EGGameLoader.h"
#include "EGEntWorldLoader.h"
#include "EGWorldFile.h"
#include "EGWorldSceneGraph.h"
#include "EGTerrainMesh.h"

EG_CLASS_DECL( EGEntWorld )

EGEntWorld::EGEntWorld() : m_InactiveEnts( EG_ENT_LIST_UPDATE_OFF_ID )
, m_ActiveEnts( EG_ENT_LIST_UPDATE_ON_ID )
, m_MasterList( eg_mem_pool::Entity )
, m_NetSmoothingEnts( EGEntList::DEFAULT_ID )
, m_RegionEnts( nullptr )
{

}

void EGEntWorld::InitWorld( eg_ent_world_role Role , EGNetMsgBuffer* NetMsgBuffer , EGObject* Owner , EGClass* GameClass )
{
	assert( m_Role == eg_ent_world_role::Unknown );
	assert( Role != eg_ent_world_role::Unknown );
	m_Role = Role;
	m_NetMsgBuffer = NetMsgBuffer;
	m_Owner = Owner;
	if( GameClass && GameClass->IsA( &EGGame::GetStaticClass() ) )
	{
		m_GameClass = GameClass;
	}

	m_AllRegionEnts = EGNewObject<EGEntWorldRegion>( eg_mem_pool::System );
	if( m_AllRegionEnts )
	{
		m_AllRegionEnts->InitRegion( eg_region_id::AllRegions );
	}

	m_WorldSceneGraph = EGNewObject<EGWorldSceneGraph>( eg_mem_pool::System );

	ResetGameInternal();
}

void EGEntWorld::DeinitWorld()
{
	if( m_WorldSceneGraph )
	{
		EGDeleteObject( m_WorldSceneGraph );
		m_WorldSceneGraph = nullptr;
	}
	
	m_NetMsgBuffer = nullptr;
	ClearWorld();
	EndPhysSim();
	assert( m_RegionEnts.IsEmpty() );
	EGDeleteObject( m_AllRegionEnts );

	m_Role = eg_ent_world_role::Unknown;
	m_Owner = nullptr;
	if( m_Game )
	{
		EGDeleteObject( m_Game );
		m_Game = nullptr;
	}

	while( m_DelayedEntEvents.HasItems() )
	{
		egDelayedEntEvent* Action = m_DelayedEntEvents.GetFirst();
		m_DelayedEntEvents.Remove( Action );
		delete Action;
	}

	ClearLoadingWorlds();
}

EGEnt* EGEntWorld::SpawnEntInternal( const egEntCreateParms& CreateParms, eg_ent_id DesiredEntId )
{
	assert( CreateParms.Role != eg_ent_role::Unknown );

	// If we are spawning into a specific spot and that entity exists
	// we destroy immediately (ideally this shouldn't happen).
	if( INVALID_ID != DesiredEntId && nullptr != GetEnt( DesiredEntId ) )
	{
		assert( false ); // Case isn't really covered right now
		EGEnt* OldEnt = GetEnt( DesiredEntId );
		if( OldEnt )
		{
			OldEnt->m_bPendingDestruction = true;
			DeinitEnt( OldEnt->GetID() );
		}
	}

	auto GetNewEntId = [this,&DesiredEntId]() -> eg_ent_id
	{
		// Figure out where we want to spawn.
		if( DesiredEntId != INVALID_ID )
		{
			return DesiredEntId;
		}

		// If there is an empty spot available use it
		for( eg_uint i=0; i<m_MasterList.Len(); i++ )
		{
			if( m_MasterList[i] == nullptr )
			{
				return eg_ent_id::IndexToId( i );
			}
		}

		return eg_ent_id::IndexToId( m_MasterList.LenAs<eg_uint>() );
	};

	eg_ent_id NewEntId = GetNewEntId();

	ExpandMasterList( NewEntId );

	eg_uint NewEntMasterIndex = eg_ent_id::IdToIndex( NewEntId );
	if( NewEntId == INVALID_ID || !(0 <= NewEntMasterIndex && NewEntMasterIndex < m_MasterList.Len()) )
	{
		assert( false ); // Could not find a spot to spawn this entity.
		return nullptr;
	}

	if( m_MasterList[NewEntMasterIndex] != nullptr )
	{
		assert( false ); // There was already an entity in this spot!
		return nullptr;
	}

	const EGEntDef* pET = EntDict_GetDef( CreateParms.EntDefId );
	if( nullptr == pET )
	{
		//assert( false ); //Wrong crc for definition.
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Tried to spawn an invalid entity (0x%08X)." , CreateParms.EntDefId.ToUint32() );
		return nullptr;
	}

	EGClass* EntClass = pET->m_EntityClassName.Class;
	if( nullptr == EntClass )
	{
		EGLogf( eg_log_t::Error, __FUNCTION__ ": Tried to spawn an entity with no class." );
		EntClass = &EGEnt::GetStaticClass();
	}

	egNewObjectParms NewObjParms( CT_Default );
	NewObjParms.SerializedMem = CreateParms.SerializedMem;
	NewObjParms.SerializedMemSize = CreateParms.SerializedMemSize;
	if( EntClass && NewObjParms.SerializedMemSize > 0 && NewObjParms.SerializedMemSize != EntClass->GetObjectSize() )
	{
		assert( false ); // Tried to load an entity of the wrong size
		NewObjParms.SerializedMem = nullptr;
		NewObjParms.SerializedMemSize = 0;
	}
	EGEnt* NewEnt = EGNewObject<EGEnt>( EntClass , eg_mem_pool::Entity , NewObjParms );
	if( nullptr == NewEnt )
	{
		assert( false ); // Out of memory to create entity.
		return nullptr;
	}

	m_MasterList[NewEntMasterIndex] = NewEnt;

	NewEnt->Init( NewEntId , this , CreateParms );

	if( m_Role == eg_ent_world_role::Server )
	{
		Net_BroadcastSpawnEnt( NewEntId );
		Net_MarkEntDirty( NewEntId );
	}

	m_NewSpawnedQueue.Append( NewEnt );

	NewEnt->m_bCanReplicateData = true;
	NewEnt->OnEnterWorld();

	return NewEnt;
}

EGGameMap* EGEntWorld::SpawnMapInternal( eg_cpstr Filename, eg_spawn_reason Reason )
{
	EGEntWorldMap* NewMap = EGNewObject<EGEntWorldMap>( eg_mem_pool::Entity );
	if( NewMap )
	{
		NewMap->InitMap( Filename , this , Reason );
		m_Maps.AppendUnique( NewMap );

		if( m_Maps.Len() == 1 )
		{
			PrimaryMapChangedDelegate.Broadcast( m_Maps[0]->GetMap() );
		}
	}

	if( m_Role == eg_ent_world_role::Server && m_NetMsgBuffer )
	{
		egNetMsgData NetData;
		NetData.Header.nMsg = eg_net_msg::EW_SpawnMap;
		NetData.Header.nSize = sizeof(NetData.EW_SpawnMap);
		NetData.Header.NetId = NET_ID_ALL;
		NetData.EW_SpawnMap.Filename = Filename;
		NetData.EW_SpawnMap.Pose = CT_Default;
		m_NetMsgBuffer->AddMsg( &NetData );
	}

	return NewMap ? NewMap->GetMap() : nullptr;
}

EGTerrain2* EGEntWorld::SpawnTerrainInternal( eg_cpstr Filename, eg_spawn_reason Reason , const eg_transform& Pose )
{
	EGEntWorldTerrain* NewTerrain = EGNewObject<EGEntWorldTerrain>( eg_mem_pool::Entity );
	if( NewTerrain )
	{
		NewTerrain->InitTerrain( Filename , this , Reason , Pose );
		m_Terrains.AppendUnique( NewTerrain );
	}

	if( m_Role == eg_ent_world_role::Server && m_NetMsgBuffer )
	{
		egNetMsgData NetData;
		NetData.Header.nMsg = eg_net_msg::EW_SpawnTerrain;
		NetData.Header.nSize = sizeof(NetData.EW_SpawnTerrain);
		NetData.Header.NetId = NET_ID_ALL;
		NetData.EW_SpawnTerrain.Filename = Filename;
		NetData.EW_SpawnTerrain.Pose = Pose;
		m_NetMsgBuffer->AddMsg( &NetData );
	}

	return NewTerrain ? NewTerrain->GetTerrain() : nullptr;
}

void EGEntWorld::BeginPhysSim( eg_cpstr ClassName )
{
	EndPhysSim();

	m_PhysClass = ClassName;

	EGClass* PhysClass = EGClass::FindClassSafe( m_PhysClass , EGPhysSim2::GetStaticClass() );

	if( PhysClass && PhysClass->IsA( &EGPhysSim2::GetStaticClass() ) )
	{
		m_PhysSim = EGNewObject<EGPhysSim2>( PhysClass , eg_mem_pool::System );
	}

	if( nullptr == m_PhysSim )
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Failed to initialize physics simulation." );
		return;
	}

	m_PhysSim->InitScene();
	m_PhysSim->SetGravity( m_PhysGravity );

	// Add all maps
	for( EGEntWorldMap* Map : m_Maps )
	{
		if( !Map->IsLoading() && Map->IsLoaded() )
		{
			m_PhysSim->AddGameMap( Map->GetMap() );
		}
	}

	// Add all terrains
	for( EGEntWorldTerrain* Terrain : m_Terrains )
	{
		if( !Terrain->IsLoading() && Terrain->IsLoaded() )
		{
			m_PhysSim->AddTerrain( Terrain->GetTerrain() , Terrain->GetPose() );
		}
	}

	// Any entities with physics components should create their bodies
	auto CreatePhysBodiesInList = [this]( auto& List , auto& GetEnt ) -> void
	{
		for( auto* ListEnt : List )
		{
			EGEnt* Ent = GetEnt( ListEnt );
			if( Ent )
			{
				assert( nullptr == Ent->m_PhysBody );
				EG_SafeRelease( Ent->m_PhysBody );
				eg_transform SpawnedPose = Ent->GetPose();
				EGPhysBodyComponent* PhysBodyComp = Ent->FindComponentByClass<EGPhysBodyComponent>();
				if( PhysBodyComp )
				{
					m_PhysSim->CreatePhysBody( Ent , PhysBodyComp->GetPhysBodyDef() , SpawnedPose );
				}
			}
		}
	};

	CreatePhysBodiesInList( m_NewSpawnedQueue , []( auto* ListItem ) -> EGEnt* { return ListItem; } );
	CreatePhysBodiesInList( m_ActiveEnts , []( auto* ListItem ) -> EGEnt* { return ListItem->Owner; });
	CreatePhysBodiesInList( m_InactiveEnts , []( auto* ListItem ) -> EGEnt* { return ListItem->Owner; });
}

void EGEntWorld::EndPhysSim()
{
	// All physics bodies must be destroyed
	auto ReleasePhysBodiesInList = []( auto& List , auto& GetEnt ) -> void
	{
		for( auto* ListEnt : List )
		{
			EGEnt* Ent = GetEnt( ListEnt );
			if( Ent && Ent->m_PhysBody )
			{
				EG_SafeRelease( Ent->m_PhysBody );
			}
		}
	};

	ReleasePhysBodiesInList( m_NewSpawnedQueue , []( auto* ListItem ) -> EGEnt* { return ListItem; } );
	ReleasePhysBodiesInList( m_ActiveEnts , []( auto* ListItem ) -> EGEnt* { return ListItem->Owner; });
	ReleasePhysBodiesInList( m_InactiveEnts , []( auto* ListItem ) -> EGEnt* { return ListItem->Owner; });

	EG_SafeRelease( m_PhysSim );
	m_PhysClass = "";
}

void EGEntWorld::Update( eg_real DeltaTime )
{
	if( m_Game )
	{
		m_Game->Update( DeltaTime );
	}

	m_bIsUpdating = true;

	UpdateLoadingWorlds();

	// Handle Net Smoothing
	EGArray<EGEntListHook*> SmoothingComplete;
	for( EGEntListHook* EntListHook : m_NetSmoothingEnts )
	{
		EGEnt* Ent = EntListHook->Owner;
		eg_bool bComplete = Net_Smooth( Ent , DeltaTime );
		if( bComplete )
		{
			SmoothingComplete.Append( EntListHook );
		}
	}

	for( EGEntListHook* Hook : SmoothingComplete )
	{
		m_NetSmoothingEnts.Remove( Hook );
	}

	// Terrain LOD
	for( EGEntWorldTerrain* Terrain : m_Terrains )
	{
		if( Terrain )
		{
			Terrain->UpdateLOD( DeltaTime , m_ActivityPos );
		}
	}

	if( m_PhysSim )
	{
		m_PhysSim->Simulate( DeltaTime );
	}

	for( EGEntListHook* UpdateEnt : m_ActiveEnts )
	{
		EGEnt* Ent = static_cast<EGEnt*>( UpdateEnt->Owner );
		Ent->ActiveUpdate( DeltaTime );
		if( m_Role == eg_ent_world_role::Server )
		{
			Net_MarkEntDirty( Ent->GetID() );
		}
	}

	if( m_Role == eg_ent_world_role::Client || m_Role == eg_ent_world_role::EditorPreview )
	{
		auto UpdateEntsInRegion = [this,&DeltaTime]( EGEntWorldRegion* Region ) -> void
		{
			if( Region )
			{
				for( EGEntListHook* EntHook : Region->GetEntList() )
				{
					EntHook->Owner->RelevantUpdate( DeltaTime );
				}
			}
		};

		UpdateEntsInRegion( m_AllRegionEnts );
		EGEntWorldRegion* ActivityRegion = GetRegion( m_ActivityRegion );
		if( ActivityRegion )
		{
			UpdateEntsInRegion( ActivityRegion );
			for( const eg_region_id& AdjId : ActivityRegion->GetAdjacentRegions() )
			{
				EGEntWorldRegion* AdjRegion = GetRegion( AdjId );
				UpdateEntsInRegion( AdjRegion );
			}
		}
	}

	m_bIsUpdating = false;

	ProcessRequestQueues();
	CullEnts();

	// Show some debug stuff
	eg_bool bShowDebugStuff = false;
	if( bShowDebugStuff )
	{
		eg_uint LogIndex = 0;
		#define EW_LOG( _m_ ) LogIndex++; EGLogToScreen::Get().Log( this , LogIndex , 1.f , _m_ )
		EW_LOG( m_Role == eg_ent_world_role::Server ? "Ent World Server" : "Ent World Client" );
		eg_uint TotalEnts = m_ActiveEnts.LenAs<eg_uint>() + m_InactiveEnts.LenAs<eg_uint>();
		EW_LOG( EGString_Format( "Total Ents: %u" , TotalEnts ) );
		EW_LOG( EGString_Format( "Active Ents: %u" , m_ActiveEnts.LenAs<eg_uint>() ) );
		EW_LOG( EGString_Format( "Inactive Ents: %u" , m_InactiveEnts.LenAs<eg_uint>() ) );
		EW_LOG( EGString_Format( "All Region Ents: %u" , m_AllRegionEnts->GetEntList().LenAs<eg_uint>() ) );
		for( eg_size_t i=0; i<m_RegionEnts.Len(); i++ )
		{
			EGEntWorldRegion* Region = m_RegionEnts.GetByIndex( i );
			if( Region && Region->GetMap() )
			{
				EW_LOG( EGString_Format( "\"%s\".\"%s\" Ents: %u" , Region->GetMap()->GetName() , Region->GetRegionName() , Region->GetEntList().LenAs<eg_uint>() ) );
			}
		}
		EW_LOG( EGString_Format( "Smoothing Ents: %u" , m_NetSmoothingEnts.LenAs<eg_uint>() ) );
		#undef EW_LOG
	}
}

void EGEntWorld::SetActivityPos( const eg_vec4& Pos )
{
	m_ActivityPos = Pos;
	m_ActivityRegion = GetRegionOfPos( m_ActivityPos , eg_region_id(m_ActivityRegion) );
}

void EGEntWorld::ClearWorld()
{
	auto ClearEnts = [this]() -> void
	{
		assert( m_bRequestAllowed );
		assert( !m_bIsUpdating );

		m_ChangeActiveQueue.Clear();

		for( eg_size_t i=0; i<m_MasterList.Len(); i++ )
		{
			if( m_MasterList[i] )
			{
				DestroyEntInternal( m_MasterList[i]->GetID() , true );
			}
		}

		CullEnts();

		assert( m_DestroyQueue.IsEmpty() );
		assert( m_ActiveEnts.IsEmpty() );
		assert( m_InactiveEnts.IsEmpty() );
		assert( m_NewSpawnedQueue.IsEmpty() );

#if defined(__DEBUG__)
		for( EGEnt* Ent : m_MasterList )
		{
			assert( nullptr == Ent );
		}

		assert( m_AllRegionEnts && m_AllRegionEnts->GetEntList().IsEmpty() );
#endif

		m_ActiveEnts.Clear();
		m_InactiveEnts.Clear();

		m_MasterList.Clear();
		m_DestroyQueue.Clear();
		m_ChangeActiveQueue.Clear();
		m_NewSpawnedQueue.Clear();
	};

	auto ClearMaps = [this]() -> void
	{
		// If we were just clearing maps and not just both
		// ents and maps we would need to move any entities in
		// these maps regions to the AllRegion ents.

		for( EGEntWorldTerrain* Terrain : m_Terrains )
		{
			EGDeleteObject( Terrain );
		}
		m_Terrains.Clear();

		while( m_Maps.HasItems() )
		{
			DestroyMapInternal( m_Maps[m_Maps.Len()-1] );
		}
		assert( m_Maps.IsEmpty() );
	};
	
	ClearEnts();
	ClearMaps();
	RebuildPhysSim();

	if( m_Role == eg_ent_world_role::Server && m_NetMsgBuffer )
	{
		egNetMsgData MsgData;
		MsgData.Header.nMsg = eg_net_msg::EW_Clear;
		MsgData.Header.NetId = NET_ID_ALL;
		MsgData.Header.nSize = 0;
		m_NetMsgBuffer->AddMsg( &MsgData );
	}

	ClearLoadingWorlds();
}

void EGEntWorld::LoadGame( EGGameLoader& Loader )
{
	MapLoadedDelegate.AddUnique( this , &ThisClass::OnSaveMapLoaded );
	TerrainLoadedDelegate.AddUnique( this , &ThisClass::OnSaveTerrainLoaded );

	m_GameLoader = &Loader;
	m_GameLoader->AddRef();

	for( const EGSaveGame::egMapObjectInfo& ObjInfo : Loader.m_SaveGame.m_MapObjects )
	{
		switch( ObjInfo.Type )
		{
		case EGSaveGame::eg_map_object_t::Unknown:
			assert( false );
			break;
		case EGSaveGame::eg_map_object_t::Map:
		{
			m_PendingMapLoads++;
			EGGameMap* GameMap = SpawnMapInternal( ObjInfo.Filename , eg_spawn_reason::GameLoad );
			if( GameMap )
			{
				if( GameMap->IsLoaded() )
				{
					m_PendingMapLoads--;
				}
			}
			else
			{
				m_PendingMapLoads--;
			}
		} break;
		case EGSaveGame::eg_map_object_t::Terrain:
		{
			m_PendingTerrainLoads++;
			EGTerrain2* Terrain = SpawnTerrainInternal( ObjInfo.Filename , eg_spawn_reason::GameLoad , ObjInfo.Pose );
			if( Terrain )
			{
				if( Terrain->GetLoadState() != LOAD_LOADING )
				{
					m_PendingTerrainLoads--;
				}
			}
			else
			{
				m_PendingTerrainLoads--;
			}
		} break;
		}
	}

	HandleThingsLoaded();
}

void EGEntWorld::ResetGame()
{
	if( m_Role == eg_ent_world_role::Server )
	{
		m_NetMsgBuffer->AddNoParmMsg( NET_ID_ALL , eg_net_msg::EW_ResetGame );
	}
	ResetGameInternal();
}

EGGameMap* EGEntWorld::SpawnMap( eg_cpstr Filename )
{
	return SpawnMapInternal( Filename , eg_spawn_reason::Spawned );
}

void EGEntWorld::DestroyMap( EGGameMap* GameMap )
{
	const EGGameMap* PrevPrimaryMap = GetPrimaryMap();

	EGEntWorldMap* WorldMap = nullptr;

	for( EGEntWorldMap* Map : m_Maps )
	{
		if( Map->GetMap() == GameMap )
		{
			WorldMap = Map;
			break;
		}
	}

	if( WorldMap )
	{
		if( m_Role == eg_ent_world_role::Server && m_NetMsgBuffer )
		{
			egNetMsgData NetData;
			NetData.Header.nMsg = eg_net_msg::EW_DestroyMap;
			NetData.Header.nSize = sizeof(NetData.EW_DestroyMap);
			NetData.Header.NetId = NET_ID_ALL;
			NetData.EW_DestroyMap.Filename = WorldMap->GetFilename();
			m_NetMsgBuffer->AddMsg( &NetData );
		}

		DestroyMapInternal( WorldMap );

		RebuildPhysSim(); // Entire physics scene has to be rebuilt when maps change
	}

	if( PrevPrimaryMap != GetPrimaryMap() )
	{
		PrimaryMapChangedDelegate.Broadcast( GetPrimaryMap() );
	}
}

EGTerrain2* EGEntWorld::SpawnTerrain( eg_cpstr Filename , const eg_transform& Pose /* = CT_Default */ )
{
	return SpawnTerrainInternal( Filename , eg_spawn_reason::Spawned , Pose );
}

void EGEntWorld::DestroyTerrain( EGTerrain2* GameTerrain )
{
	EGEntWorldTerrain* WorldTerrain = nullptr;

	for( EGEntWorldTerrain* Terrain : m_Terrains )
	{
		if( Terrain->GetTerrain() == GameTerrain )
		{
			WorldTerrain = Terrain;
			break;
		}
	}

	if( WorldTerrain && m_Terrains.Contains( WorldTerrain ) )
	{
		if( m_Role == eg_ent_world_role::Server && m_NetMsgBuffer )
		{
			egNetMsgData NetData;
			NetData.Header.nMsg = eg_net_msg::EW_DestroyTerrain;
			NetData.Header.nSize = sizeof(NetData.EW_DestroyTerrain);
			NetData.Header.NetId = NET_ID_ALL;
			NetData.EW_DestroyTerrain.Filename = WorldTerrain->GetFilename();
			m_NetMsgBuffer->AddMsg( &NetData );
		}

		EGTerrain2* GameTerrain = WorldTerrain->GetTerrain();
		if( m_Game && GameTerrain && GameTerrain->GetLoadState() == LOAD_LOADED )
		{
			m_Game->OnTerrainDestroyed( GameTerrain );
		}

		m_Terrains.DeleteByItem( WorldTerrain );
		EGDeleteObject( WorldTerrain );
		RebuildPhysSim(); // Entire physics scene has to be rebuilt when maps change
	}
}

void EGEntWorld::MoveTerrain( EGTerrain2* GameTerrain, const eg_transform& NewPose )
{
	for( EGEntWorldTerrain* Terrain : m_Terrains )
	{
		if( Terrain->GetTerrain() == GameTerrain )
		{
			Terrain->SetPose( NewPose );
			RebuildPhysSim();
			break;
		}
	}
}

EGEnt* EGEntWorld::SpawnEnt( const eg_string_crc& EntDef, const eg_transform& Pose /*= CT_Default */, eg_cpstr InitString /*= "" */ )
{
	egEntCreateParms CreateParms( CT_Clear );
	CreateParms.EntDefId = EntDef;
	CreateParms.Pose = Pose;
	CreateParms.InitString = InitString;
	CreateParms.Reason = eg_spawn_reason::Spawned;
	if( m_Role == eg_ent_world_role::EditorPreview )
	{
		CreateParms.Role = eg_ent_role::EditorPreview;
	}
	else
	{
		CreateParms.Role = eg_ent_role::Authority;
	}
	return SpawnEntInternal( CreateParms , INVALID_ID );
}

void EGEntWorld::DestroyEnt( const eg_ent_id& EntId )
{
	DestroyEntInternal( EntId , false );
}

void EGEntWorld::SetEntActive( const eg_ent_id& EntId , eg_bool bActive )
{
	assert( m_bRequestAllowed );

	egChangeActive NewRequest;
	NewRequest.EntId = EntId;
	NewRequest.bActive = bActive;
	m_ChangeActiveQueue.Append( NewRequest );
}

void EGEntWorld::GetAllEnts( EGArray<eg_ent_id>& OutIds ) const
{
	for( EGEnt* Ent : m_MasterList )
	{
		if( Ent )
		{
			OutIds.Append( Ent->GetID() );
		}
	}
}

void EGEntWorld::HandleGameChanged()
{
	// Reset pointers for entities (EXPLOR-73):
	for( EGEnt* Ent : m_MasterList )
	{
		if( Ent )
		{
			Ent->RefreshGame();
		}
	}
}

void EGEntWorld::PrintDebugInfo()
{
	EGLogf( eg_log_t::General , ("Entity lists:"));
	EGLogf( eg_log_t::General , ("Inactive: %u."), m_InactiveEnts.Len());	
	EGLogf( eg_log_t::General , ("Active:   %u."), m_ActiveEnts.Len());
	EGLogf( eg_log_t::General , ("Total:    %u/%u."),	m_InactiveEnts.Len() + m_ActiveEnts.Len() , m_MasterList.Len() );
}

void EGEntWorld::FullyReplicateToClient( const egNetCommId& DestId ) const
{
	if( nullptr == m_NetMsgBuffer )
	{
		assert( false ); // Can't replicate!
		return;
	}

	egNetMsgData Msg;
	Msg.Header.NetId = DestId;

	Msg.Header.nMsg = eg_net_msg::EW_BeginFullReplication;
	Msg.Header.nSize = 0;
	m_NetMsgBuffer->AddMsg( &Msg );

	// Send the maps:
	Msg.Header.nMsg = eg_net_msg::EW_SpawnMap;
	Msg.Header.nSize = sizeof(Msg.EW_SpawnMap);
	for( const EGEntWorldMap* Map : m_Maps )
	{
		if( Map )
		{
			Msg.EW_SpawnMap.Filename = Map->GetFilename();
			Msg.EW_SpawnMap.Pose = CT_Default;
			m_NetMsgBuffer->AddMsg( &Msg );
		}
	}

	// Send the terrains:
	Msg.Header.nMsg = eg_net_msg::EW_SpawnTerrain;
	Msg.Header.nSize = sizeof(Msg.EW_SpawnTerrain);
	for( const EGEntWorldTerrain* Terrain : m_Terrains )
	{
		if( Terrain )
		{
			Msg.EW_SpawnTerrain.Filename = Terrain->GetFilename();
			Msg.EW_SpawnTerrain.Pose = Terrain->GetPose();
			m_NetMsgBuffer->AddMsg( &Msg );
		}
	}

	// Send the spawned entities:
	Msg.Header.nMsg = eg_net_msg::EW_SpawnEnt;
	Msg.Header.nSize = sizeof(Msg.EW_SpawnEnt);
	for( const EGEnt* Ent : m_MasterList )
	{
		if( Ent && !Ent->m_bPendingDestruction )
		{
			Msg.EW_SpawnEnt.EntId = Ent->GetID();
			Msg.EW_SpawnEnt.DefId = Ent->m_DefCrcId;
			Msg.EW_SpawnEnt.Pose = Ent->GetPose();
			m_NetMsgBuffer->AddMsg( &Msg );
		}
	}

	// Update all
	Msg.Header.nMsg = eg_net_msg::EW_UpdateEnt;
	Msg.Header.nSize = sizeof(Msg.EW_UpdateEnt);
	for( const EGEnt* Ent : m_MasterList )
	{
		if( Ent && !Ent->m_bPendingDestruction )
		{
			Msg.EW_UpdateEnt.EntId=Ent->m_Id;
			Msg.EW_UpdateEnt.Pose = Ent->m_Pose.Pose;
			Msg.EW_UpdateEnt.Bounds = Ent->m_aabb;
			Msg.EW_UpdateEnt.TimeMs = egNet_EW_UpdateEnt::TIME_IS_NO_SMOOTH;
		}
	}

	if( m_Game )
	{
		m_Game->OnFullReplication();
	}

	// Send full replication notification, in case data needs to be replicated.
	for( const EGEnt* Ent : m_MasterList )
	{
		if( Ent && !Ent->m_bPendingDestruction )
		{
			Ent->OnFullReplication();
		}
	}

	Msg.Header.nMsg = eg_net_msg::EW_EndFullReplication;
	Msg.Header.nSize = 0;
	m_NetMsgBuffer->AddMsg( &Msg );
}

EGGameMap* EGEntWorld::GetPrimaryMap()
{
	return m_Maps.IsValidIndex(0) && m_Maps[0]->GetMap()->IsLoaded() ? m_Maps[0]->GetMap() : nullptr;
}

const EGGameMap* EGEntWorld::GetPrimaryMap() const
{
	return m_Maps.IsValidIndex(0) && m_Maps[0]->GetMap()->IsLoaded() ? m_Maps[0]->GetMap() : nullptr;
}

void EGEntWorld::SpawnMapEnts( const EGGameMap* GameMap )
{
	if( nullptr == GameMap || !GameMap->IsLoaded() )
	{
		return;
	}

	for(eg_uint i=0; i<GameMap->GetTagCount(); i++)
	{
		eg_string sAtts = GameMap->GetTagAtts(i+1);
		if(!eg_string(GameMap->GetTagType(i+1)).Compare(("entity")))
		{
			eg_string sType;
			eg_string sRot;
			eg_string InitString( CT_Clear );
			eg_vec4 v4Pos = GameMap->GetTagPos(i+1);
			eg_vec3 v3Rot = {0,0,0};

			eg_bool bGot = EGPARSE_OKAY == ::EGParse_GetAttValue(GameMap->GetTagAtts(i+1), ("entity"), &sType);
			if(EGPARSE_OKAY == ::EGParse_GetAttValue(GameMap->GetTagAtts(i+1), ("rotation"), &sRot))
			{
				EGString_GetFloatList(sRot, sRot.Len(), &v3Rot, 3, false);
				//Now convert to radians.
				v3Rot.x = EG_Deg(v3Rot.x).ToRad();
				v3Rot.y = EG_Deg(v3Rot.y).ToRad();
				v3Rot.z = EG_Deg(v3Rot.z).ToRad();

			}

			if( EGPARSE_OKAY == EGParse_GetAttValue( GameMap->GetTagAtts(i+1) , "init_string" , &InitString ) )
			{

			}

			if(bGot)
			{
				//Form the spawn matrix:
				eg_transform Pose( CT_Default );
				Pose = eg_transform::BuildRotationY(EG_Rad(v3Rot.y));
				Pose.RotateXThis(EG_Rad(v3Rot.x));
				Pose.RotateZThis(EG_Rad(v3Rot.z));
				Pose.TranslateThis( v4Pos.x , v4Pos.y , v4Pos.z );

				egEntCreateParms CreateParms( CT_Clear );
				CreateParms.EntDefId = eg_string_crc(sType);
				CreateParms.Reason = eg_spawn_reason::MapEntity;
				CreateParms.InitString = InitString;
				CreateParms.Pose = Pose;
				CreateParms.SerializedMem = nullptr;
				CreateParms.SerializedMemSize = 0;
				CreateParms.Role = eg_ent_role::Authority;

				SpawnEntInternal( CreateParms , INVALID_ID );
			}

		}
	}
}

void EGEntWorld::LoadWorld( eg_cpstr Filename )
{
	EGEntWorldLoader* NewLoadedWorld = EGNewObject<EGEntWorldLoader>();
	if( NewLoadedWorld )
	{
		NewLoadedWorld->InitWorldLoader( Filename , this );
		m_PreLoadingWorlds.Append( NewLoadedWorld );
	}
}

void EGEntWorld::SetGravity( const eg_vec3& NewGravityVector )
{
	m_PhysGravity = eg_vec4( NewGravityVector , 0.f );
	if( m_PhysSim )
	{
		m_PhysSim->SetGravity( m_PhysGravity );
	}
}

eg_region_id EGEntWorld::GetRegionOfPos( const eg_vec4& Pos, const eg_region_id& LastKnownRegion ) const
{
	eg_aabb PosBounds( Pos , Pos );

	if( LastKnownRegion != eg_region_id::AllRegions && LastKnownRegion != eg_region_id::NoRegion )
	{
		const EGEntWorldRegion* Region = GetRegion( LastKnownRegion );
		if( Region )
		{
			if( Region->GetBounds().Intersect( PosBounds , nullptr ) )
			{
				return LastKnownRegion;
			}

			const EGArray<eg_region_id>& AdjRegions = Region->GetAdjacentRegions();
			for( const eg_region_id& AdjId : AdjRegions )
			{
				const EGEntWorldRegion* AdjRegion = GetRegion( AdjId );
				if( AdjRegion && AdjRegion->GetBounds().Intersect( PosBounds , nullptr ) )
				{
					return AdjRegion->GetId();
				}
			}
		}
	}

	// If we made it here we need to check all regions.
	for( eg_size_t i=0; i<m_RegionEnts.Len(); i++ )
	{
		EGEntWorldRegion* Region = m_RegionEnts.GetByIndex( i );
		if( Region && Region->GetBounds().Intersect( PosBounds , nullptr ) )
		{
			return Region->GetId();
		}
	}

	return eg_region_id::AllRegions;
}

void EGEntWorld::GetVisibleEnts( const eg_region_id& RootRegion , EGArray<const EGEnt*>& Out ) const
{
	// More to to here, should pass in region wanted...
	auto GetEntsForRegion = [this,&Out]( const EGEntWorldRegion* Region ) -> void
	{
		if( Region )
		{
			for( const EGEntListHook* Hook : Region->GetEntList() )
			{
				assert( !Out.Contains( Hook->Owner ) );
				Out.Append( Hook->Owner );
			}
		}
	};

	GetEntsForRegion( m_AllRegionEnts );
	const EGEntWorldRegion* CameraRegion = GetRegion( RootRegion );
	if( CameraRegion && CameraRegion != m_AllRegionEnts )
	{
		GetEntsForRegion( CameraRegion );
		for( const eg_region_id& AdjId : CameraRegion->GetAdjacentRegions() )
		{
			const EGEntWorldRegion* AdjRegion = GetRegion( AdjId );
			GetEntsForRegion( AdjRegion );
		}
	}
}

void EGEntWorld::CalculateClosestLights( EGEnt* Ent ) const
{
	Ent->m_CloseLights.Clear();
	if( !Ent->IsLit() )
	{
		return;
	}

	const EGEntWorldRegion* EntRegion = GetRegion( Ent->GetWorldRegion() );
	if( EntRegion )
	{
		EntRegion->GetCloseLights( Ent->GetPose().GetPosition() , Ent->m_CloseLights );
		Ent->m_AmbientLight = eg_color32(EntRegion->GetAmbientLight());
	}
}

#if 0
void EGClient::CalcVis_Regions(const eg_uint nVisArray)
{
	if( m_PrimaryMap == nullptr )
	{
		return;
	}

	//The first thing to do, is to determine what regions are visible. This is
	//done with a DFS. The render queue stores only the indexes of the regions
	//to be rendered, and should be in an approximately front to back order in
	//order to prevent overdraw. Exactly front to back is not gauranteed since
	//this is a DFS and not a BFS, but as far as rendering is concerned it
	//will be fairly close.
	egVisInfo& VI = m_VisInfo[nVisArray];

	//We need a stack for the DFS (m_DFSStack), we also need a structure 
	//(EGBoolList) to keep track of the DFS vertexes we have visited (m_VV)
	//(the egPortalVs). And we need another structure to keep track of which
	//regions we've already decided to render (m_RV), so that we don't end up 
	//pushing them into the render que more than once. Note that all of these
	//structures have enough space reserved, that they will not be dynamically
	//resized during this method (See OnLoad).

	//Make sure the above structures are empty. Note that DFSStack should always
	//be empty since it should be emptied by the end of this DFS, but our DFS is
	//limited to how many iterations will occur so it is possible that it will
	//not be emptied if the game is attempting to render "deeper" than the limit.
	m_DFSStack.Clear();
	m_VV.UnsetAll();
	m_RV.UnsetAll();


	//PortalV will be used to store the current vertex we are considering. The
	//firt vertex we consider we'll call portal 0 (since we arent' actually
	//looking through a portal) when we start, and the region is the region the
	//camera is in.
	egPortalV PortalV(0, m_ClientView.GetCameraRegion());

	//In case we got to this method and no map was loaded or the camera is out
	//of of the regions, we'll bail out. (In this case 
	//m_VisInfo[nVisArray].m_RgnQue.size() will return 0.)
	if(0 == PortalV.nRegion)
		return;


	//To begin the DFS we push the original vertex onto the stack.
	m_DFSStack.Push(PortalV);
	//We will certainly be rasterizing the region that the camera is in.
	//(Admittedly it is possible that this region isn't visible, since we might
	//actually be standing in the "doorway", but in most cases it will, and if
	//it isn't visible, the rasterizer won't be spending that much time with it
	//anyway since the vertices will be clipped by the back clip plane.)
	VI.m_RgnQue.Push(PortalV.nRegion);
	//Mark the starting region as visited.
	m_RV.Set(PortalV.nRegion);
	//Also mark portal 0 as having been visited.
	m_VV.Set(PortalV.nPortal);

	//We'll be generating a new view frustrum each time we look through a portal
	//that way we can limit the new frustrum to only that which is visible
	//through the portal.
	EGViewFr ViewFr(*m_ClientView.GetViewFr());

	//Now do the DFS (we use a for loop to make sure we dont' get into an
	//infinite loop), also note that we are actually treating portals as
	//vertexes in the graph, and not as edges. The regions are treated as edges
	//this is because it is possible to have two portals looking into the same
	//region, and when adjusting the view frustrum we need to account for that.
	//Becuase different portals may be visible depending on which portal we are
	//looking through.
	static const eg_uint MAX_DFS_LOOPS = 100;
	for(eg_uint nInf = 0; nInf<MAX_DFS_LOOPS && (m_DFSStack.Len() > 0); nInf++)
	{
		//::EG_OLprintf(("Loop %i"), nInf);

		//We first get the portal on the top of the stack, and pop it from the
		//stack.
		PortalV = m_DFSStack.Top();
		m_DFSStack.Pop();

		//The view frustrum is now adjusted for this iteration. If we are still in
		//the orginal region, we use the camera view frustrum. If not we adjust
		//the view frustrum so that it fits around the portal that we are
		//currently looking through. Note that if a portal is partly offscreen
		//this might actually create a bigger view frustrum, but that won't matter
		//because we always make sure that the portals are visible from the 
		//camera's frustrum.
		eg_t_sphere sphAdjst;
		if(0 != PortalV.nPortal)
		{
			//Always reset the adjusted view frustrum, since we may end up
			//not adjusting the frustrum, and if that happens, we don't want
			//to end up using another portals view frustrum.
			ViewFr = *m_ClientView.GetViewFr();
			const EGWorldMap::egPortal* pPortal = m_PrimaryMap->GetPortal(PortalV.nPortal);
			eg_vec4 NewCenter( CT_Default );
			m_ClientView.WorldToView( &NewCenter , &pPortal->sphPortal.GetCenter() , 1);
			sphAdjst.Transform = eg_transform::BuildTranslation( NewCenter.ToVec3() );
			sphAdjst.Sphere.SetupSphere( pPortal->sphPortal.GetRadius() );

#if 0
			::EG_OLprintf(("Portal %i (%s to %s):"), 
				PortalV.nPortal, 
				m_Map.GetRegionName(pPortal->nFrom), 
				m_Map.GetRegionName(pPortal->nTo));
#endif
			ViewFr.AdjustAboutSphere(sphAdjst);
		}

		//The final step is to check all the child portals, if a portal is visible
		//we push it onto the DFS stack and in the next iterations they will be
		//considered.
		eg_uint nPortals = m_PrimaryMap->GetPortalCount(PortalV.nRegion);
		for(eg_uint nPort=1; nPort<=nPortals; nPort++)
		{
			const EGWorldMap::egPortal* pPortal = m_PrimaryMap->GetPortal(PortalV.nRegion, nPort);
			sphAdjst.Sphere.SetupSphere( pPortal->sphPortal.GetRadius() );
			eg_vec4 NewCenter( CT_Default );
			m_ClientView.WorldToView(&NewCenter, &pPortal->sphPortal.GetCenter(), 1);
			sphAdjst.Transform = eg_transform::BuildTranslation( NewCenter.ToVec3() );

			//If we haven't considered this portal yet, and it is visible by
			//both the adjusted frustum and the original camera frustum (this is
			//in case the adjusted frustum is wider than the original), or
			// it is always visible then
			//we will render the region that this portal is looking at.
			if(
				!m_VV.IsSet(pPortal->nID)
				&& 
				( 
					pPortal->bAlwaysVisible
					||
					(
						ViewFr.IsSphereVisible(&sphAdjst)
						&& 
						m_ClientView.GetViewFr()->IsSphereVisible(&sphAdjst)
						)
					)
				)
			{
				//if(0 == nVisArray)::EG_OLprintf(("   Portal %i (%s to %s) is visible:"), pPortal->nID, m_Map.GetRegionName(pPortal->nFrom), m_Map.GetRegionName(pPortal->nTo));

				//If we've already decided to render this region by some other
				//portal, then we don't need to render it again.
				if(!m_RV.IsSet(pPortal->nTo))
				{
					VI.m_RgnQue.Push(pPortal->nTo);
					m_RV.Set(pPortal->nTo);
				}

				//This portal is now visited, we only need to consider it's
				//children if it is a non-terminal portal.
				if(!pPortal->bTerminal)
				{
					egPortalV NewDFSItem(pPortal->nID, pPortal->nTo);
					m_DFSStack.Push(NewDFSItem);
				}
				m_VV.Set(pPortal->nID);
			}
		}
	}
	//The previous loop finishes when we have completed the DFS or when we have
	//reached the maximum number of iterations. m_VisInfo[nVisArray].m_RgnQue 
	//will now be filled with all the regions that are to be rendered, and the
	//DFS stack should be empty (unless we have gone "too deep").
	//
	//Since this is a DFS, and because all methods called from this method,
	//(such as the vector's push_back method, and the view frustum adjusting
	//methods) are O(1) the worst case complexity is
	//O(|V| + |E|), recall that each region is an Edge, and each portal is
	//treated as a Vertex in the algorithm, so the worst case is
	//O(m_Map.GetRegionCount() + m_Map.GetPortalCount())
	//Keep in mind, maps should be designed so that the worst case complexity
	//is never attained, because if it is, everything will be queued to be
	//rendered, and that could seriously be detrimental to the actual
	//rasterization. Ideally only two or three regions should be rendered at a
	//time.
}

#endif

void EGEntWorld::DrawEntBBs( EGDebugBox* DbBox ) const
{
	auto DrawBox = [&DbBox]( const EGEnt* Ent ) -> void
	{
		MainDisplayList->SetWorldTF(eg_mat::I);
		MainDisplayList->PushDefaultShader( eg_defaultshader_t::COLOR );
		MainDisplayList->DrawAABB(Ent->m_aabb, eg_color(eg_color32(255,0,255)) );	
		MainDisplayList->PopDefaultShader();

		const eg_aabb& box = Ent->m_aabb;
		DbBox->DrawDebugShape( eg_vec3( box.GetWidth() , box.GetHeight() , box.GetDepth() ) , eg_transform::BuildTranslation( box.GetCenter().ToVec3() ) , eg_color(0.2f,0.2f,1.f,.3f) );
	};

	for( const EGEnt* Ent : m_BuildSceneGraphVisEntCache )
	{
		DrawBox( Ent );
	}
}

void EGEntWorld::DrawLights( const eg_ent_id& CloseToEnt , EGDebugSphere* DbLight ) const
{
	const EGEnt* Ent = GetEnt( CloseToEnt );

	if( nullptr == Ent )
	{
		return;
	}

	//Render the debug mesh, so we know where the lights are:
	MainDisplayList->PushDefaultShader( eg_defaultshader_t::MESH_UNLIT );
	//eg_transform m1( CT_Default );
	for( const egEntLight& Light : Ent->m_CloseLights )
	{
		eg_transform Pose( CT_Default );
		Pose = eg_transform::BuildTranslation( Light.Pos );
		eg_real ScaleVec = .1f;
		MainDisplayList->SetMaterial(EGV_MATERIAL_NULL);
		DbLight->DrawDebugShape( ScaleVec , Pose , eg_color(Light.Color) );
		//Draw again, but this time with the radius of the light at a really
		//low alpha.
		eg_real Range = EGMath_sqrt(Light.RangeSq);
		eg_color Palette = eg_color(Light.Color);
		Palette.a = 0.1f;
		MainDisplayList->PushRasterizerState( eg_rasterizer_s::CULL_NONE );
		DbLight->DrawDebugShape( Range , Pose , Palette );
		MainDisplayList->PopRasterizerState();
	}

	MainDisplayList->PopDefaultShader();
}

void EGEntWorld::DrawMapBBs() const
{
	for( const EGEntWorldMap* Map : m_Maps )
	{
		if( Map->GetMap() && Map->GetMap()->IsLoaded() )
		{
			Map->GetMap()->DrawAABBs( 0xFFFFFFFF );
		}
	} 
}

void EGEntWorld::DrawMapGraphs( EGDebugSphere* VertexSphere ) const
{
	for( const EGEntWorldMap* Map : m_Maps )
	{
		if( Map->GetMap() && Map->GetMap()->IsLoaded() )
		{
			Map->GetMap()->DrawGraphs( VertexSphere );
		}
	}
}

void EGEntWorld::DrawMapPortals( EGDebugSphere* DbPortal ) const
{
	MainDisplayList->PushDefaultShader( eg_defaultshader_t::MESH_UNLIT );
	MainDisplayList->SetWorldTF(eg_mat::I);

	for( const EGEntWorldMap* WorldMap : m_Maps )
	{
		const EGGameMap* Map = WorldMap->GetMap();
		if( Map->IsLoaded() )
		{
			for(eg_uint nRegion = 1; nRegion <= Map->GetRegionCount(); nRegion++)
			{
				for(eg_uint nPort = 1; nPort <= Map->GetPortalCount(nRegion); nPort++)
				{
					const EGWorldMapBase::egPortal* pPortal = Map->GetPortal(nRegion, nPort);

					//::EG_OLprintf(("The portal has radius %f"), pPortal->sphPortal.fRadius);
					eg_transform Pose = eg_transform::BuildTranslation( pPortal->sphPortal.GetCenter().ToVec3() );
					DbPortal->DrawDebugShape( pPortal->sphPortal.GetRadius() , Pose , eg_color(.5f,5.f,1.f,.3f) );
				}
			}
		}
	}

	MainDisplayList->PopDefaultShader();
}

egEntWorldRaycastResult EGEntWorld::RaycastWorld( const eg_vec4& Origin, const eg_vec4& NormalizedDir )
{
	egEntWorldRaycastResult Out;

	if( m_PhysSim )
	{
		Out.Group = m_PhysSim->RaycastClosestShape( Origin , NormalizedDir , &Out.HitPoint , &Out.HitEnt );
	}

	return Out;
}

void EGEntWorld::SaveToSaveGame( EGSaveGame& Out )
{		
	// Global Chunk
	if( m_Game )
	{
		EGSaveGame::egGlobalDataInfo GlobalInfo( CT_Default );

		// Actual Binary Data of Game
		{
			GlobalInfo.BinaryData.Resize( m_Game->GetObjectSize() );
			assert( m_Game->GetObjectSize() <= GlobalInfo.BinaryData.Len()); // Can't save everything!
			eg_size_t SizeToCopy = EG_Min( m_Game->GetObjectSize() , GlobalInfo.BinaryData.Len() );
			EGMem_Copy( GlobalInfo.BinaryData.GetArray() , m_Game , SizeToCopy );
		}

		// Whatever gets written in PostSave
		{
			EGFileData AdditionalData( eg_file_data_init_t::HasOwnMemory );
			m_Game->PostSave( AdditionalData );
			eg_size_t SizeToCopy = AdditionalData.GetSize();
			GlobalInfo.AdditionalData.Resize( SizeToCopy );
			SizeToCopy = GlobalInfo.AdditionalData.Len();
			assert( AdditionalData.GetSize() <= GlobalInfo.AdditionalData.Len() );
			if( SizeToCopy > 0 )
			{
				EGMem_Copy( GlobalInfo.AdditionalData.GetArray() , AdditionalData.GetData() , SizeToCopy );
			}

			Out.AddGlobalData( GlobalInfo );
		}
	}

	// Map objects
	for( EGEntWorldMap* Map : m_Maps )
	{
		EGSaveGame::egMapObjectInfo NewInfo;
		NewInfo.Type = EGSaveGame::eg_map_object_t::Map;
		NewInfo.Pose = CT_Default;
		EGString_Copy( NewInfo.Filename , Map->GetFilename() , countof(NewInfo.Filename) );
		Out.AddMapObjectInfo( NewInfo );
	}

	for( EGEntWorldTerrain* Terrain : m_Terrains )
	{
		EGSaveGame::egMapObjectInfo NewInfo;
		NewInfo.Pose = Terrain->GetPose();
		NewInfo.Type = EGSaveGame::eg_map_object_t::Terrain;
		EGString_Copy( NewInfo.Filename , Terrain->GetFilename() , countof(NewInfo.Filename) );
		Out.AddMapObjectInfo( NewInfo );
	}

	// Entities
	EGArray<eg_ent_id> AllEnts;
	GetAllEnts( AllEnts );
	for( const eg_ent_id& Id : AllEnts )
	{
		EGEnt* Ent = GetEnt( Id );

		if( nullptr == Ent )
		{
			continue;
		}

		if( !Ent->IsSerialized() )
		{
			continue;
		}

		Ent->PreSave();

		EGSaveGame::egEntInfo EntInfo( CT_Default );
		EGFileData AdditionalData( EntInfo.AdditionalData );
		Ent->PostSave( AdditionalData );

		EntInfo.EntData.EntId = Ent->GetID();
		EntInfo.EntData.DefCrcId = Ent->m_DefCrcId;
		EntInfo.EntData.InitString = "";
		EntInfo.EntData.Pose = Ent->GetPose();
		eg_size_t DataSize = Ent->GetObjectSize();
		eg_size_t ObjStart = reinterpret_cast<eg_size_t>(Ent);
		eg_size_t EntStart = reinterpret_cast<eg_size_t>(EGCast<EGObject>(Ent));
		EntInfo.EntData.BinaryDataSize = static_cast<eg_uint>(DataSize);
		EntInfo.BinaryData.Resize( DataSize );
		EGMem_Copy( EntInfo.BinaryData.GetArray() , Ent , EntInfo.BinaryData.Len() );
		EntInfo.EntData.bHasPose = true;
		Out.AddEntity( EntInfo );
	}
}

void EGEntWorld::Net_MarkEntDirty( const eg_ent_id& EntId )
{
	EGEnt* Ent = GetEnt( EntId );
	if( Ent && !Ent->IsInServerUpdateList() )
	{
		Ent->SetIsInServerUpdateList( true );
		m_DirtyEntList.Append( EntId );
	}
}

void EGEntWorld::Net_BroadcastDirtyEnts( eg_uint GameTimeMs )
{
	m_LastDirtyEntSendSize = 0;

	for( const eg_ent_id& EntId : m_DirtyEntList )
	{
		EGEnt* Ent = GetEnt( EntId );
		if( Ent ) // Necessary to do this check because it is very possible that this entity was flagged as dirty and deleted in the same frame.
		{
			assert( Ent->IsInServerUpdateList() );
			Ent->SetIsInServerUpdateList( false );
			UpdateEntRegion( Ent );
			egNetMsgData info;
			info.Header.nMsg=eg_net_msg::EW_UpdateEnt;
			info.Header.nSize=sizeof(info.EW_UpdateEnt);
			info.Header.NetId = NET_ID_ALL;
			info.EW_UpdateEnt.EntId=Ent->m_Id;
			info.EW_UpdateEnt.Pose = Ent->m_Pose;
			info.EW_UpdateEnt.Bounds = Ent->m_aabb;
			info.EW_UpdateEnt.TimeMs = Ent->m_bPoseHasJumped ? egNet_EW_UpdateEnt::TIME_IS_NO_SMOOTH : GameTimeMs;
			Ent->m_bPoseHasJumped = false;

			m_LastDirtyEntSendSize++;
			if( m_NetMsgBuffer )
			{
				m_NetMsgBuffer->AddMsg( &info );
			}
		}
	}
	m_DirtyEntList.Clear( false );
}

void EGEntWorld::Net_BroadcastSpawnEnt( const eg_ent_id& EntId ) const
{
	assert( m_Role == eg_ent_world_role::Server );

	const EGEnt* Ent = GetEnt( EntId );
	if( Ent && m_NetMsgBuffer && m_Role == eg_ent_world_role::Server )
	{
		egNetMsgData info;
		info.Header.nMsg=eg_net_msg::EW_SpawnEnt;
		info.Header.nSize=sizeof(info.EW_SpawnEnt);
		info.Header.NetId = NET_ID_ALL;
		info.EW_SpawnEnt.EntId = Ent->m_Id;
		info.EW_SpawnEnt.DefId = Ent->m_DefCrcId;
		info.EW_SpawnEnt.Pose = Ent->m_Pose.Pose;
		m_NetMsgBuffer->AddMsg( &info );
	}
}

void EGEntWorld::Net_BroadcastDestroyEnt( const eg_ent_id& EntId ) const
{
	assert( m_Role == eg_ent_world_role::Server );

	if( m_NetMsgBuffer && m_Role == eg_ent_world_role::Server )
	{
		egNetMsgData info;
		info.Header.nMsg=eg_net_msg::EW_DestroyEnt;
		info.Header.nSize=sizeof(info.EW_DestroyEnt);
		info.Header.NetId = NET_ID_ALL;
		info.EW_DestroyEnt.EntId = EntId;
		m_NetMsgBuffer->AddMsg( &info );
	}
}

void EGEntWorld::Net_RunEntEvent( const eg_ent_id& EntId, const eg_string_crc& EventId )
{
	assert( m_Role == eg_ent_world_role::Server );

	if( m_Role == eg_ent_world_role::Server && m_NetMsgBuffer )
	{
		egNetMsgData bcs;
		bcs.Header.nSize = sizeof(bcs.EW_RunEntEvent);
		bcs.Header.nMsg = eg_net_msg::EW_RunEntEvent;
		bcs.Header.NetId = NET_ID_ALL;
		bcs.EW_RunEntEvent.EntId  = EntId;
		bcs.EW_RunEntEvent.EventId = EventId;
		m_NetMsgBuffer->AddMsg( &bcs );
	}
}

void EGEntWorld::Net_ReplicateGameData( eg_size_t DataStartOffset , eg_size_t DataSize ) const
{
	assert( m_Game );

	egNetMsgData bcs;

	const eg_size_t MAX_REPLICATE_SIZE = sizeof( bcs.EW_ReplicateGameData.Data );

	if( m_Game && m_NetMsgBuffer )
	{
		eg_size_t AmountToReplicate = DataSize;

		auto ReplicateChunk = [this,&bcs,&MAX_REPLICATE_SIZE]( eg_size_t StartOffset, eg_size_t ChunkSize ) -> void
		{
			if( ( ( StartOffset + ChunkSize ) <= m_Game->GetObjectSize() ) && ( ChunkSize <= MAX_REPLICATE_SIZE ) )
			{
				bcs.Header.nSize = ChunkSize + ( sizeof( bcs.EW_ReplicateGameData ) - sizeof( bcs.EW_ReplicateGameData.Data ) );
				assert( bcs.Header.nSize <= sizeof( bcs.EW_ReplicateGameData ) );
				bcs.Header.nMsg = eg_net_msg::EW_ReplicateGameData;
				bcs.Header.NetId = NET_ID_ALL;
				bcs.EW_ReplicateGameData.Offset = static_cast<eg_uint64>( StartOffset );
				bcs.EW_ReplicateGameData.Size = static_cast<eg_uint8>( ChunkSize );
				EGMem_Copy( bcs.EW_ReplicateGameData.Data, reinterpret_cast<const eg_byte*>( m_Game ) + StartOffset, ChunkSize );
				m_NetMsgBuffer->AddMsg( &bcs );
			}
			else
			{
				EGLogf( eg_log_t::Error, __FUNCTION__ ": Attempted to replicate too much data to the server at once, or data was out of range." );
				assert( false );
			}
		};

		eg_size_t AmountReplicated = 0;

		while( AmountReplicated < DataSize )
		{
			eg_size_t ChunkStart = DataStartOffset + AmountReplicated;
			eg_size_t ChunkSize = EG_Min( DataSize - AmountReplicated, MAX_REPLICATE_SIZE );
			AmountReplicated += ChunkSize;
			ReplicateChunk( ChunkStart, ChunkSize );
		}

		assert( AmountReplicated == DataSize );
	}
}

void EGEntWorld::Net_ReplicateEntData( const eg_ent_id& EntId , eg_size_t DataStartOffset , eg_size_t DataSize ) const
{
	egNetMsgData bcs;

	const EGEnt* Ent = GetEnt( EntId );
	const eg_size_t MAX_REPLICATE_SIZE = sizeof(bcs.EW_ReplicateEntData.Data);
	if( Ent && m_NetMsgBuffer && ((DataStartOffset+DataSize) <= Ent->GetObjectSize()) && (DataSize <= MAX_REPLICATE_SIZE) )
	{
		bcs.Header.nSize = DataSize + (sizeof(bcs.EW_ReplicateEntData) - sizeof(bcs.EW_ReplicateEntData.Data));
		assert( bcs.Header.nSize <= sizeof(bcs.EW_ReplicateEntData) );
		bcs.Header.nMsg = eg_net_msg::EW_ReplicateEntData;
		bcs.Header.NetId = NET_ID_ALL;
		bcs.EW_ReplicateEntData.EntId = EntId;
		bcs.EW_ReplicateEntData.Offset = static_cast<eg_uint64>(DataStartOffset);
		bcs.EW_ReplicateEntData.Size   = static_cast<eg_uint8>(DataSize);
		EGMem_Copy( bcs.EW_ReplicateEntData.Data , reinterpret_cast<const eg_byte*>(Ent) + DataStartOffset , DataSize );
		m_NetMsgBuffer->AddMsg( &bcs );
	}
	else
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Attempted to replicate too much data to the server at once, or data was out of range." );
		assert( false ); //Attempted to replicate too much data to the server at once, or data was out of range.
	}
}

void EGEntWorld::Net_RequestSpawnInfoForEnt( const eg_ent_id& EntId ) const
{
	assert( m_Role == eg_ent_world_role::Client );

	if( m_NetMsgBuffer && m_Role == eg_ent_world_role::Client )
	{
		EGLogf( eg_log_t::Verbose, "Missing spawn info for %u. Requesting info..." , EntId.Id );
		egNetMsgData bcs;
		bcs.Header.nSize = sizeof( bcs.EW_RequestEntSpawnInfo );
		bcs.Header.nMsg = eg_net_msg::EW_RequestEntSpawnInfo;
		bcs.Header.NetId = NET_ID_ALL;
		bcs.EW_RequestEntSpawnInfo.EntId = EntId;
		m_NetMsgBuffer->AddMsg( &bcs );
	}
}

eg_bool EGEntWorld::Net_Smooth( EGEnt* Ent , eg_real DeltaTime )
{
	assert( 0 != m_EntUpdateRate );

	return Ent->m_Pose.Smooth( DeltaTime , m_EntUpdateRate );
}

void EGEntWorld::HandleNetMsg( const egNetMsgData& NetMsg )
{
	switch( NetMsg.Header.nMsg )
	{
		case eg_net_msg::EW_Clear: HandleNetMsg_Clear(); break;
		case eg_net_msg::EW_BeginFullReplication: HandleNetMsg_BeginFullReplication(); break;
		case eg_net_msg::EW_EndFullReplication: HandleNetMsg_EndFullReplication(); break;
		case eg_net_msg::EW_SpawnEnt: HandleNetMsg_SpawnEnt( NetMsg.EW_SpawnEnt ); break;
		case eg_net_msg::EW_DestroyEnt: HandleNetMsg_DestroyEnt( NetMsg.EW_DestroyEnt ); break;
		case eg_net_msg::EW_UpdateEnt: HandleNetMsg_UpdateEnt( NetMsg.EW_UpdateEnt ); break;
		case eg_net_msg::EW_RunEntEvent: HandleNetMsg_RunEntEvent( NetMsg.EW_RunEntEvent ); break;
		case eg_net_msg::EW_ReplicateEntData: HandleNetMsg_ReplicateEntData( NetMsg.EW_ReplicateEntData ); break;
		case eg_net_msg::EW_ReplicateGameData: HandleNetMsg_ReplicateGameData( NetMsg.EW_ReplicateGameData ); break;
		case eg_net_msg::EW_RequestEntSpawnInfo: HandleNetMsg_RequestEntSpawnInfo( NetMsg.EW_RequestEntSpawnInfo); break;
		case eg_net_msg::EW_SpawnMap: HandleNetMsg_SpawnMap( NetMsg.EW_SpawnMap ); break;
		case eg_net_msg::EW_DestroyMap: HandleNetMsg_DestroyMap( NetMsg.EW_DestroyMap ); break;
		case eg_net_msg::EW_SpawnTerrain: HandleNetMsg_SpawnTerrain( NetMsg.EW_SpawnTerrain ); break;
		case eg_net_msg::EW_DestroyTerrain: HandleNetMsg_DestroyTerrain( NetMsg.EW_DestroyTerrain ); break;
		case eg_net_msg::EW_ResetGame: HandleNetMsg_ResetGame(); break;
		default:
		{
			assert( false ); // This is not a valid EntWorld Net Message.
		} break;
	}
}

void EGEntWorld::HandleMapLoaded( EGEntWorldMap* WorldMap )
{
	EGGameMap* GameMap = WorldMap->GetMap();
	assert( GameMap && GameMap->IsLoaded() );

	for( eg_uint i=0; i<GameMap->GetRegionCount(); i++ )
	{
		EGEntWorldRegion* NewRegion = EGNewObject<EGEntWorldRegion>( eg_mem_pool::System );
		if( NewRegion )
		{
			NewRegion->InitRegion( GameMap , i+1 );
			eg_region_id NewRegionId = NewRegion->GetId();
			assert( !m_RegionEnts.Contains( NewRegionId ) ); // Two of the same, this is bad, really bad.
			if( !m_RegionEnts.Contains( NewRegionId ) )
			{
				m_RegionEnts.Insert( NewRegionId , NewRegion );
			}
			else
			{
				// Really bad!
				EGDeleteObject( NewRegion );
			}
		}
	}
	
	// Since the regions have been changed, all ent regions should be re-evaluated.

	for( EGEntListHook* EntHook : m_ActiveEnts )
	{
		UpdateEntRegion( EntHook->Owner );
	}

	for( EGEntListHook* EntHook : m_InactiveEnts )
	{
		UpdateEntRegion( EntHook->Owner );
	}

	RebuildPhysSim();

	if( m_Game )
	{
		m_Game->OnMapLoaded( GameMap );
	}

	MapLoadedDelegate.Broadcast( GameMap );
}

void EGEntWorld::HandleTerrainLoaded( EGEntWorldTerrain* WorldTerrain )
{
	unused( WorldTerrain );

	RebuildPhysSim();

	if( m_Game )
	{
		m_Game->OnTerrainLoaded( WorldTerrain->GetTerrain() );
	}

	TerrainLoadedDelegate.Broadcast( WorldTerrain->GetTerrain() );
}

eg_bool EGEntWorld::IsMapStillLoading( const EGGameMap* GameMap ) const
{
	for( EGEntWorldMap* Map : m_Maps )
	{
		if( Map->GetMap() == GameMap )
		{
			return Map->IsLoading();
		}
	}
	return false;
}

eg_bool EGEntWorld::IsTerrainStillLoading( const EGTerrain2* GameTerrain ) const
{
	for( EGEntWorldTerrain* Terrain : m_Terrains )
	{
		if( Terrain->GetTerrain() == GameTerrain )
		{
			return Terrain->IsLoading();
		}
	}
	return false;
}

void EGEntWorld::OnClientEnterWorld( const eg_lockstep_id& LockstepId )
{
	if( m_Game )
	{
		assert( !m_ClientsInWorld.Contains( LockstepId ) );
		m_ClientsInWorld.AppendUnique( LockstepId );
		m_Game->OnClientEnterWorld( LockstepId );
	}
}

void EGEntWorld::OnClientLeaveWorld( const eg_lockstep_id& LockstepId )
{
	if( m_Game )
	{
		assert( m_ClientsInWorld.Contains( LockstepId ) );
		m_Game->OnClientLeaveWorld( LockstepId );
		m_ClientsInWorld.DeleteByItem( LockstepId );
	}
}

void EGEntWorld::ProcessRequestQueues()
{
	assert( m_bRequestAllowed );

	m_bRequestAllowed = false;

	for( EGEnt* Ent : m_NewSpawnedQueue )
	{
		assert( Ent->GetUpdateType() == 0 );
		if( Ent->GetUpdateType() == 0 )
		{
			m_InactiveEnts.Insert( &Ent->m_UpdateListHook );
		}
		UpdateEntRegion( Ent );
	}
	m_NewSpawnedQueue.Clear();

	for( const egChangeActive& Change : m_ChangeActiveQueue )
	{
		SetEntToUpdateInternal( Change.EntId , Change.bActive );
	}
	m_ChangeActiveQueue.Clear();

	m_bRequestAllowed = true;
}

void EGEntWorld::SetEntToUpdateInternal( const eg_ent_id EntId, eg_bool bActive )
{
	assert( !m_bIsUpdating );
	
	//If the ID is out of range, bail...
	if( !m_MasterList.IsValidIndex( eg_ent_id::IdToIndex( EntId ) ) )
	{
		assert( false );
		return;
	}

	EGEnt* pEnt = GetEnt( EntId );

	assert( pEnt );
	if( !pEnt )
	{
		return;
	}

	//::EGLogf(("%i changed from %i to %i"), EntId.Id, pEnt->GetType(), status);

	//The first thing to do is remove the entity from the list it is currently in.
	eg_uint status_old = pEnt->GetUpdateType();
	switch( status_old )
	{
	case EG_ENT_LIST_UPDATE_OFF_ID : m_InactiveEnts.Remove( &pEnt->m_UpdateListHook ); break;
	case EG_ENT_LIST_UPDATE_ON_ID  : m_ActiveEnts.Remove( &pEnt->m_UpdateListHook ); break;
	default: assert( false ); break;
	}

	//Now we push the entity into it's new home.
	eg_uint status_new = bActive ? EG_ENT_LIST_UPDATE_ON_ID : EG_ENT_LIST_UPDATE_OFF_ID;
	switch( status_new )
	{
	case EG_ENT_LIST_UPDATE_OFF_ID : m_InactiveEnts.Insert( &pEnt->m_UpdateListHook ); break;
	case EG_ENT_LIST_UPDATE_ON_ID  : m_ActiveEnts.Insert( &pEnt->m_UpdateListHook ); break;
	default: assert( false ); break;
	}
}

void EGEntWorld::ExpandMasterList( eg_ent_id BiggestEntId )
{
	if( BiggestEntId != INVALID_ID )
	{
		const eg_uint BiggestEntIndex = eg_ent_id::IdToIndex( BiggestEntId );
		const eg_uint ListMinSize = BiggestEntIndex + 1;
		const eg_size_t OldSize = m_MasterList.Len();

		if( OldSize < ListMinSize )
		{
			m_MasterList.Resize( ListMinSize );
			for( eg_size_t i=OldSize; i< ListMinSize; i++ )
			{
				m_MasterList[i] = nullptr;
			}
		}
	}
}

void EGEntWorld::DeinitEnt( const eg_ent_id& EntId )
{
	EGEnt* Ent = GetEnt( EntId );
	assert( Ent && Ent->IsPendingDestruction() );
	if( Ent )
	{
		if( m_NewSpawnedQueue.Contains( Ent ) )
		{
			m_NewSpawnedQueue.DeleteByItem( Ent );
		}

		Ent->Deinit();

		if( Ent->m_NetSmoothingHook.GetListId() == m_NetSmoothingEnts.GetId() )
		{
			m_NetSmoothingEnts.Remove( &Ent->m_NetSmoothingHook );
		}

		// Remove the entity from the list it's in.
		eg_uint EntStatus = Ent->GetUpdateType();
		switch( EntStatus )
		{
		case 0: break;
		case EG_ENT_LIST_UPDATE_OFF_ID : m_InactiveEnts.Remove( &Ent->m_UpdateListHook ); break;
		case EG_ENT_LIST_UPDATE_ON_ID  : m_ActiveEnts.Remove( &Ent->m_UpdateListHook ); break;
		default: assert( false ); break;
		}

		MoveEntToRegion( Ent , eg_region_id::NoRegion );

		m_MasterList[ eg_ent_id::IdToIndex(EntId) ] = nullptr;
		EGDeleteObject( Ent );
	}
}

void EGEntWorld::UpdateEntRegion( EGEnt* Ent )
{
	if( Ent )
	{
		eg_region_id CurrentRegionId = Ent->GetWorldRegion();

		// Always visible ents are always in AllRegions.
		if( Ent->IsAlwaysVisible() )
		{
			if( CurrentRegionId != eg_region_id::AllRegions )
			{
				MoveEntToRegion( Ent , eg_region_id::AllRegions );
			}

			return;
		}
		
		if( CurrentRegionId == eg_region_id::NoRegion || CurrentRegionId == eg_region_id::AllRegions )
		{
			// Will have to check all regions...
		}
		else if( m_RegionEnts.Contains( CurrentRegionId ) )
		{
			EGEntWorldRegion* CurrentRegion = GetRegion( CurrentRegionId );
			if( CurrentRegion )
			{
				if( !CurrentRegion->GetBounds().Intersect( Ent->m_aabb , nullptr ) )
				{
					// Only need to check adjacent regions.
					const EGArray<eg_region_id>& AdjRegions = CurrentRegion->GetAdjacentRegions();
					for( const eg_region_id& AdjId : AdjRegions )
					{
						EGEntWorldRegion* AdjRegion = GetRegion( AdjId );
						if( AdjRegion && AdjRegion->GetBounds().Intersect( Ent->m_aabb , nullptr ) )
						{
							// EGLogf( eg_log_t::General , "Moved to adjacent region" );
							MoveEntToRegion( Ent , AdjId );
							// Moved to an adjacent region, we're done.
							return;
						}
					}
				}
				else
				{
					// Hasn't left this region, we're done.
					return;
				}
			}
		}
		else
		{
			assert( false ); // What is the state of this world?
		}

		// If we made it here we need to check all regions.
		for( eg_size_t i=0; i<m_RegionEnts.Len(); i++ )
		{
			EGEntWorldRegion* Region = m_RegionEnts.GetByIndex( i );
			if( Region && Region->GetBounds().Intersect( Ent->m_aabb , nullptr ) )
			{
				MoveEntToRegion( Ent , Region->GetId() );
				// Found a region for it...
				return;
			}
		}

		// Finally, we found nothing. Move to all regions.
		MoveEntToRegion( Ent , eg_region_id::AllRegions );
	}
}

void EGEntWorld::RebuildPhysSim()
{
	if( m_PhysSim )
	{
		eg_string_small CurPhysClass = m_PhysClass;
		EndPhysSim();
		BeginPhysSim( CurPhysClass );
	}
}

void EGEntWorld::ResetGameInternal()
{
	if( m_Game )
	{
		for( const eg_lockstep_id& ClientId : m_ClientsInWorld )
		{
			m_Game->OnClientLeaveWorld( ClientId );
		}

		EGDeleteObject( m_Game );
		m_Game = nullptr;
	}

	m_Game = EGNewObject<EGGame>( m_GameClass , eg_mem_pool::System );
	if( m_Game )
	{
		m_Game->Init( this );

		for( const eg_lockstep_id& ClientId : m_ClientsInWorld )
		{
			m_Game->OnClientEnterWorld( ClientId );
		}
	}
	HandleGameChanged();
}

void EGEntWorld::OnSaveMapLoaded( EGGameMap* GameMap )
{
	for( EGEntWorldMap* WorldMap : m_Maps )
	{
		if( WorldMap->GetMap() == GameMap && WorldMap->GetSpawnReason() == eg_spawn_reason::GameLoad )
		{
			m_PendingMapLoads--;
			HandleThingsLoaded();
			break;
		}
	}
}

void EGEntWorld::OnSaveTerrainLoaded( EGTerrain2* Terrain )
{
	for( EGEntWorldTerrain* WorldTerrain : m_Terrains )
	{
		if( WorldTerrain->GetTerrain() == Terrain && WorldTerrain->GetSpawnReason() == eg_spawn_reason::GameLoad )
		{
			m_PendingTerrainLoads--;
			HandleThingsLoaded();
			break;
		}
	}
}

void EGEntWorld::HandleThingsLoaded()
{
	assert( m_PendingMapLoads >= 0 && m_PendingTerrainLoads >= 0 );

	if( m_PendingMapLoads == 0 && m_PendingTerrainLoads == 0 )
	{
		FinalizeGameLoad();
	}
}

void EGEntWorld::FinalizeGameLoad()
{
	if( m_GameLoader == nullptr )
	{
		assert( false );
		return;
	}

	MapLoadedDelegate.Remove( this , &ThisClass::OnSaveMapLoaded );
	TerrainLoadedDelegate.Remove( this , &ThisClass::OnSaveTerrainLoaded );

	EGGameLoader& Loader = *m_GameLoader;

	m_NetMsgBuffer->AddNoParmMsg( NET_ID_ALL , eg_net_msg::EW_ResetGame );

	EGArray<eg_byte> PreserveData;
	EGArray<eg_byte> PreserveAdditionalData;

	if( m_Game )
	{
		PreserveData.Resize( m_Game->GetObjectSize() );
		if( PreserveData.GetArray() && PreserveData.Len() == m_Game->GetObjectSize() )
		{
			EGMem_Copy( PreserveData.GetArray() , m_Game , m_Game->GetObjectSize() );
		}

		EGFileData AddionalData( PreserveAdditionalData );
		m_Game->PostSave( AddionalData );

		EGDeleteObject( m_Game );
		m_Game = nullptr;
	}

	{
		egNewObjectParms NewGameParms( CT_Default );
		EGFileData AdditionalData( eg_file_data_init_t::SetableUserPointer );
		if( Loader.m_SaveGame.m_GlobalDataInfos.IsValidIndex(0) )
		{
			NewGameParms.SerializedMem = Loader.m_SaveGame.m_GlobalDataInfos[0].BinaryData.GetArray();
			NewGameParms.SerializedMemSize = Loader.m_SaveGame.m_GlobalDataInfos[0].BinaryData.Len();
			AdditionalData.SetData( Loader.m_SaveGame.m_GlobalDataInfos[0].AdditionalData.GetArray() , Loader.m_SaveGame.m_GlobalDataInfos[0].AdditionalData.Len() );
		}
		else
		{
			assert( PreserveData.Len() == m_GameClass->GetObjectSize() );
			NewGameParms.SerializedMem = PreserveData.GetArray();
			NewGameParms.SerializedMemSize = m_GameClass->GetObjectSize();
			AdditionalData.SetData( PreserveAdditionalData.GetArray() , PreserveAdditionalData.Len() );
		}
		m_Game = EGNewObject<EGGame>( m_GameClass , eg_mem_pool::System , NewGameParms );
		m_Game->Init( this );			
		m_Game->PostLoad( AdditionalData );
		HandleGameChanged();
	}

	PreserveData.Clear();
	PreserveAdditionalData.Clear();

	if( m_Game )
	{
		m_Game->OnSaveGameLoaded();
	}

	//Now spawn entities:
	for( const EGSaveGame::egEntInfo& EntInfo : Loader.m_SaveGame.m_EntInfos )
	{
		egEntCreateParms CreateParms( CT_Clear );
		CreateParms.EntDefId = EntInfo.EntData.DefCrcId;
		CreateParms.Reason = eg_spawn_reason::GameLoad;
		CreateParms.InitString = EntInfo.EntData.InitString;
		CreateParms.Pose = EntInfo.EntData.Pose;
		CreateParms.SerializedMem = EntInfo.BinaryData.GetArray();
		CreateParms.SerializedMemSize = EntInfo.BinaryData.Len();
		CreateParms.Role = eg_ent_role::Authority;

		EGFileData AdditionalData( EntInfo.AdditionalData.GetArray() , EntInfo.AdditionalData.Len() );

		EGEnt* pEnt = SpawnEntInternal( CreateParms , EntInfo.EntData.EntId );
		if( pEnt )
		{
			pEnt->PostLoad( AdditionalData );
		}
	}

	EG_SafeRelease( m_GameLoader );

	SaveGameLoadCompleteDelegate.Broadcast();
}

void EGEntWorld::HandleNetMsg_Clear()
{
	assert( m_Role == eg_ent_world_role::Client ); // Only clients can have their world cleared remotely.

	if( m_Role == eg_ent_world_role::Client )
	{
		ClearWorld();
	}
}

void EGEntWorld::HandleNetMsg_BeginFullReplication()
{
	assert( m_Role == eg_ent_world_role::Client );
	ClearWorld();
}

void EGEntWorld::HandleNetMsg_EndFullReplication()
{
	assert( m_Role == eg_ent_world_role::Client );
}

void EGEntWorld::HandleNetMsg_SpawnEnt( const struct egNet_EW_SpawnEnt& Parms )
{
	assert( m_Role == eg_ent_world_role::Client );

	if( !Net_CanGetServerUpdates() ) return; //If the server isn't loaded yet, ignore this.

	egEntCreateParms CreateParms( CT_Clear );
	CreateParms.Pose = Parms.Pose;
	CreateParms.Reason = eg_spawn_reason::Replicated;
	CreateParms.Role = eg_ent_role::Replicated;
	CreateParms.EntDefId = Parms.DefId;

	EGEnt* NewEnt = SpawnEntInternal( CreateParms , Parms.EntId );

	// Presumably a net spawn ent will cause a region upate?

	/*
	eg_uint nDestRegion = m_Map.GetRegionCount()+1;

	//The region has changed, so move the entity.
	m_RegionEnts[nDestRegion].Insert(pNew);

	NewEnt->m_nNumRegions = nDestRegion!=0?1:0;
	NewEnt->m_anRegions[0] = nDestRegion;
	*/

	// We'll now search through the delayed actions and run any for this entity.

	// First create a list of all items to be processed (we're allocating, but
	// realistically this should be like 1 or 2 things).
	struct egProcessItem : public IListable
	{
		egDelayedEntEvent* EntEvent;
	};

	EGList<egProcessItem> ProcessList( EGList<egProcessItem>::DEFAULT_ID );

	for( egDelayedEntEvent* Action : m_DelayedEntEvents )
	{
		if( Action->EntId == Parms.EntId )
		{
			egProcessItem* NewProcessItem = new egProcessItem;
			NewProcessItem->EntEvent = Action;
			ProcessList.InsertLast( NewProcessItem );
		}
	}

	// Second process the items and remove them from the delayed actions list
	while( ProcessList.HasItems() )
	{
		egProcessItem* ProcessItem = ProcessList.GetFirst();
		ProcessList.Remove( ProcessItem );
		egNet_EW_RunEntEvent EntEvent;
		EntEvent.EntId = ProcessItem->EntEvent->EntId;
		EntEvent.EventId = ProcessItem->EntEvent->EventId;
		HandleNetMsg_RunEntEvent( EntEvent );
		m_DelayedEntEvents.Remove( ProcessItem->EntEvent );
		delete ProcessItem->EntEvent;
		delete ProcessItem;
	}
}

void EGEntWorld::HandleNetMsg_DestroyEnt( const struct egNet_EW_DestroyEnt& Parms )
{
	assert( m_Role == eg_ent_world_role::Client );
	if( m_Role == eg_ent_world_role::Client )
	{
		DestroyEntInternal( Parms.EntId , true );
	}
}

void EGEntWorld::HandleNetMsg_UpdateEnt( const struct egNet_EW_UpdateEnt& Parms )
{
	assert( m_Role == eg_ent_world_role::Client );

	if( m_Role == eg_ent_world_role::Client )
	{
		EGEnt* pEnt = GetEnt( Parms.EntId );
		if( nullptr == pEnt )
		{
			Net_RequestSpawnInfoForEnt( Parms.EntId );
			return;
		}

		pEnt->m_aabb = Parms.Bounds;
		if( Parms.TimeMs == egNet_EW_UpdateEnt::TIME_IS_NO_SMOOTH )
		{
			pEnt->m_Pose = Parms.Pose;
			if( pEnt->m_NetSmoothingHook.GetListId() != 0 )
			{
				m_NetSmoothingEnts.Remove( &pEnt->m_NetSmoothingHook );
			}
		}
		else
		{
			pEnt->m_Pose.RecPose.Pose = Parms.Pose;
			pEnt->m_Pose.RecPose.TimeMs = Parms.TimeMs;

			if( pEnt->m_NetSmoothingHook.GetListId() == 0 )
			{
				m_NetSmoothingEnts.Insert( &pEnt->m_NetSmoothingHook );
			}
		}

		UpdateEntRegion( pEnt );
	}
}

void EGEntWorld::HandleNetMsg_RunEntEvent( const struct egNet_EW_RunEntEvent& Parms )
{
	assert( m_Role == eg_ent_world_role::Client );

	if( m_Role == eg_ent_world_role::Client )
	{
		EGEnt* Ent = GetEnt( Parms.EntId );

		if( !Net_CanGetServerUpdates() || nullptr == Ent )
		{
			// If this ent wasn't spawned yet save this event for later.
			EGLogf( eg_log_t::Verbose , __FUNCTION__ ": An EntAction was called before the client was ready. Buffering it." );
			egDelayedEntEvent* NewAction = new egDelayedEntEvent;
			NewAction->EntId = Parms.EntId;
			NewAction->EventId = Parms.EventId;
			m_DelayedEntEvents.InsertLast( NewAction );
			return;
		}

		if( Ent )
		{
			Ent->RunEvent( Parms.EventId );
		}
	}
}

void EGEntWorld::HandleNetMsg_ReplicateEntData( const struct egNet_EW_ReplicateEntData& Parms )
{
	EGEnt* Ent = GetEnt( Parms.EntId );
	if( Ent == nullptr && m_Role == eg_ent_world_role::Client )
	{
		Net_RequestSpawnInfoForEnt( Parms.EntId );
		return;
	}

	if( Ent == nullptr && m_Role == eg_ent_world_role::Server )
	{
		// Dead Ent tried to replicate data to us.
		return;
	}

	if( Ent && (Parms.Offset+Parms.Size) <= Ent->GetObjectSize() )
	{
		eg_byte* RepTarget = reinterpret_cast<eg_byte*>(Ent) + Parms.Offset;
		EGMem_Copy( RepTarget , Parms.Data , Parms.Size );
		Ent->OnDataReplicated( reinterpret_cast<eg_byte*>(Ent) + Parms.Offset , Parms.Size );
	}
	else
	{
		assert( false ); //Bad replication data transmitted?
	}
}

void EGEntWorld::HandleNetMsg_ReplicateGameData( const struct egNet_EW_ReplicateGameData& Parms )
{
	if( m_Game && ( Parms.Offset + Parms.Size ) <= m_Game->GetObjectSize() )
	{
		EGMem_Copy( reinterpret_cast<eg_byte*>( m_Game ) + Parms.Offset , Parms.Data , Parms.Size );
		m_Game->OnDataReplicated( reinterpret_cast<eg_byte*>( m_Game ) + Parms.Offset , Parms.Size );
	}
	else
	{
		assert( false ); //Bad replication data transmitted?
	}
}

void EGEntWorld::HandleNetMsg_RequestEntSpawnInfo( const struct egNet_EW_RequestEntSpawnInfo& Parms )
{
	assert( m_Role == eg_ent_world_role::Server );

	if( m_Role == eg_ent_world_role::Server )
	{
		EGEnt* Ent = GetEnt( Parms.EntId );
		if( Ent )
		{
			EGLogf( eg_log_t::Verbose , "Sending spawn info for %u." , Parms.EntId.Id );
			Net_BroadcastSpawnEnt( Parms.EntId );
			Net_MarkEntDirty( Parms.EntId );
		}
		else
		{
			EGLogf( eg_log_t::Server, "Spawn info did not exist for %u.", Parms.EntId.Id );
			Net_BroadcastDestroyEnt( Parms.EntId );
		}
	}
}

void EGEntWorld::HandleNetMsg_SpawnMap( const struct egNet_EW_SpawnFile& Parms )
{
	SpawnMapInternal( Parms.Filename , eg_spawn_reason::Replicated );
}

void EGEntWorld::HandleNetMsg_DestroyMap( const struct egNet_EW_SpawnFile& Parms )
{
	EGEntWorldMap* WorldMap = nullptr;

	for( EGEntWorldMap* Map : m_Maps )
	{
		if( EGString_EqualsI( Map->GetFilename() , Parms.Filename ) )
		{
			WorldMap = Map;
			break;
		}
	}

	if( WorldMap )
	{
		DestroyMap( WorldMap->GetMap() );
	}
}

void EGEntWorld::HandleNetMsg_SpawnTerrain( const struct egNet_EW_SpawnFile& Parms )
{
	SpawnTerrainInternal( Parms.Filename , eg_spawn_reason::Replicated , Parms.Pose );
}

void EGEntWorld::HandleNetMsg_DestroyTerrain( const struct egNet_EW_SpawnFile& Parms )
{
	EGEntWorldTerrain* WorldTerrain = nullptr;

	for( EGEntWorldTerrain* Terrain : m_Terrains )
	{
		if( EGString_EqualsI( Terrain->GetFilename() , Parms.Filename ) )
		{
			WorldTerrain = Terrain;
			break;
		}
	}

	if( WorldTerrain )
	{
		DestroyTerrain( WorldTerrain->GetTerrain() );
	}
}

void EGEntWorld::HandleNetMsg_ResetGame()
{
	ResetGameInternal();
}

eg_bool EGEntWorld::Net_CanGetServerUpdates() const
{
	return m_Role == eg_ent_world_role::Client;
}

void EGEntWorld::UpdateLoadingWorlds()
{
	// At most handle one loaded world

	// Do a pre-load only if nothing is waiting for it's post load.
	if( !m_PostLoadingWorlds.HasItems() && m_PreLoadingWorlds.HasItems() )
	{
		for( EGEntWorldLoader* Loader : m_PreLoadingWorlds )
		{
			if( Loader->IsLoaded() )
			{
				m_PreLoadingWorlds.DeleteByItem( Loader );

				const EGWorldFile* WorldFile = Loader->GetWorldFile();
				WorldFile->ApplyToWorldPreLoad( this );

				m_PostLoadingWorlds.Append( Loader );
				return; // Bail out till next frame (and until nothing is being loaded).
			}
		}
	}

	// We'll handle any post loads first.
	if( m_PostLoadingWorlds.HasItems() )
	{
		for( EGEntWorldLoader* Loader : m_PostLoadingWorlds )
		{
			const EGWorldFile* WorldFile = Loader->GetWorldFile();

			if( WorldFile->IsPreLoadComplete( this ) )
			{
				m_PostLoadingWorlds.DeleteByItem( Loader );

				const EGWorldFile* WorldFile = Loader->GetWorldFile();
				WorldFile->ApplyToWorldPostLoad( this );
				EGDeleteObject( Loader );
				PostLoadWorldComplete.Broadcast();
				return; // Bail out till next frame and loading will commence.
			}
		}
	}
}

void EGEntWorld::ClearLoadingWorlds()
{
	for( EGEntWorldLoader* Loader : m_PreLoadingWorlds )
	{
		EGDeleteObject( Loader );
	}
	m_PreLoadingWorlds.Clear();

	for( EGEntWorldLoader* Loader : m_PostLoadingWorlds )
	{
		EGDeleteObject( Loader );
	}
	m_PostLoadingWorlds.Clear();
}

eg_bool EGEntWorld::IsLoadingWorlds() const
{
	return m_PreLoadingWorlds.HasItems() || m_PostLoadingWorlds.HasItems();
}

void EGEntWorld::DestroyEntInternal( const eg_ent_id& EntId, eg_bool bDestroyEvenIfProxy )
{
	assert( m_bRequestAllowed );

	if( EntId != INVALID_ID )
	{
		EGEnt* Ent = GetEnt( EntId );
		if( Ent && !Ent->m_bPendingDestruction )
		{
			Ent->OnLeaveWorld();
			if( Ent->m_Role == eg_ent_role::Authority || Ent->m_Role == eg_ent_role::EditorPreview || bDestroyEvenIfProxy )
			{
				Ent->m_bPendingDestruction = true;
				m_DestroyQueue.Append( EntId );
				if( m_Role == eg_ent_world_role::Server && !bDestroyEvenIfProxy )
				{
					Net_BroadcastDestroyEnt( EntId );
				}
			}
			else
			{
				assert( false ); // Only an authoritative entity can be destroyed.
			}
		}
	}
}

void EGEntWorld::DestroyMapInternal( EGEntWorldMap* WorldMap )
{
	EGGameMap* GameMap = WorldMap ? WorldMap->GetMap() : nullptr;
	if( GameMap )
	{
		if( m_Game && GameMap->IsLoaded() )
		{
			m_Game->OnMapDestroyed( GameMap );
		}

		const eg_uint RegionCount = GameMap->GetRegionCount();
		for( eg_uint i=0; i<RegionCount; i++ )
		{
			eg_region_id RegionId( GameMap , i+1 );
			DestroyRegion( RegionId );
		}
	}

	if( m_Maps.Contains( WorldMap ) )
	{
		m_Maps.DeleteByItem( WorldMap );
		EGDeleteObject( WorldMap );
	}
}

void EGEntWorld::DestroyRegion( const eg_region_id& RegionId )
{
	if( m_RegionEnts.Contains( RegionId ) )
	{
		EGEntWorldRegion* Region = m_RegionEnts[RegionId];

		// Move all ents in this region to all regions.
		EGArray<EGEnt*> EntsToMove;
		for( EGEntListHook* Hook : Region->GetEntList() )
		{
			EntsToMove.Append( Hook->Owner );
		}

		for( EGEnt* Ent : EntsToMove )
		{
			MoveEntToRegion( Ent , eg_region_id::AllRegions );
		}

		assert( Region->GetEntList().IsEmpty() );

		m_RegionEnts.Delete( RegionId );

		EGDeleteObject( Region );
	}
}

void EGEntWorld::MoveEntToRegion( EGEnt* Ent, const eg_region_id& NewRegionId )
{
	eg_region_id PrevRegionId = Ent->GetWorldRegion();

	// First remove the entity from it's current region:

	if( PrevRegionId == eg_region_id::NoRegion )
	{
		// Nothing to do
	}
	else if( PrevRegionId == eg_region_id::AllRegions )
	{	
		m_AllRegionEnts->GetEntList().Remove( &Ent->m_RegionListHook );
		Ent->m_RegionId = eg_region_id::NoRegion;
	}
	else
	{
		assert( m_RegionEnts.Contains( PrevRegionId ) );
		if( m_RegionEnts.Contains( PrevRegionId ) )
		{
			EGEntWorldRegion* Region = m_RegionEnts[PrevRegionId];
			assert( nullptr != Region );
			if( Region )
			{
				Region->GetEntList().Remove( &Ent->m_RegionListHook );
				Ent->m_RegionId = eg_region_id::NoRegion;
			}
		}
	}

	// Next put the ent into it's new region.
	if( NewRegionId == eg_region_id::NoRegion )
	{
		// Nothing to do...
		assert( Ent->m_RegionId == eg_region_id::NoRegion );
		assert( Ent->m_RegionListHook.GetListId() == 0 );
	}
	else if( NewRegionId == eg_region_id::AllRegions )
	{
		m_AllRegionEnts->GetEntList().Insert( &Ent->m_RegionListHook );
		Ent->m_RegionId = eg_region_id::AllRegions;
	}
	else
	{
		assert( m_RegionEnts.Contains( NewRegionId ) );
		if( m_RegionEnts.Contains( NewRegionId ) )
		{
			m_RegionEnts[NewRegionId]->GetEntList().Insert( &Ent->m_RegionListHook );
			Ent->m_RegionId = NewRegionId;
		}
	}
}

EGEntWorldRegion* EGEntWorld::GetRegion( const eg_region_id& RegionId )
{
	if( RegionId == eg_region_id::AllRegions )
	{
		return m_AllRegionEnts;
	}
	else if( m_RegionEnts.Contains( RegionId ) )
	{
		return m_RegionEnts[RegionId];
	}

	return nullptr;
}

const EGEntWorldRegion* EGEntWorld::GetRegion( const eg_region_id& RegionId ) const
{
	if( RegionId == eg_region_id::AllRegions )
	{
		return m_AllRegionEnts;
	}
	else if( m_RegionEnts.Contains( RegionId ) )
	{
		return m_RegionEnts[RegionId];
	}

	return nullptr;
}

void EGEntWorld::UpdateScenGraph( const EGCamera2& Camera ) const
{
	if( nullptr == m_WorldSceneGraph )
	{
		return;
	}

	m_WorldSceneGraph->ResetGraph( Camera );

	m_CameraRegionId = GetRegionOfPos( Camera.GetPose().GetPosition() , m_CameraRegionId );
	const EGEntWorldRegion* CameraRegion = GetRegion( m_CameraRegionId );

	//
	// Maps
	//
	egWorldSceneGraphMapRegion NewRegion;

	if( CameraRegion == m_AllRegionEnts )
	{
		if( m_Maps.Len() > 0 )
		{
			// EGLogToScreen::Get().Log( this , 687 , 1.f , EGString_Format( "Warning: Drawing all regions..." ) );
		}

		for( const EGEntWorldMap* Map : m_Maps )
		{
			if( Map && Map->GetMap() )
			{
				const eg_uint NumRegions = Map->GetMap()->GetRegionCount();
				for( eg_uint i=0; i<NumRegions; i++ )
				{
					NewRegion.GameMap = Map->GetMap();
					NewRegion.RegionIndex = i+1;
					m_WorldSceneGraph->AddItem( NewRegion );
				}
			}
		}
	}
	else if( CameraRegion )
	{
		if( CameraRegion->GetMap() && CameraRegion->GetMap()->IsLoaded() )
		{
			NewRegion.GameMap = CameraRegion->GetMap();
			NewRegion.RegionIndex = CameraRegion->GetRegionIndex();
			m_WorldSceneGraph->AddItem( NewRegion );
		}

		const EGArray<eg_region_id>& AdjRegions = CameraRegion->GetAdjacentRegions();
		// Draw all adjacent regions, and that's it.
		for( const eg_region_id& AdjId : AdjRegions )
		{
			const EGEntWorldRegion* AdjRegion = GetRegion( AdjId );
			if( AdjRegion && AdjRegion->GetMap() && AdjRegion->GetMap()->IsLoaded() )
			{
				NewRegion.GameMap = AdjRegion->GetMap();
				NewRegion.RegionIndex = AdjRegion->GetRegionIndex();
				m_WorldSceneGraph->AddItem( NewRegion );
			}
		}
	}

	//
	// Terrains
	//

	// For now just draw all terrains.
	egWorldSceneGraphTerrain NewTerrain;

	for( EGEntWorldTerrain* Terrain : m_Terrains )
	{
		if( Terrain && Terrain->GetTerrainMesh() && Terrain->GetTerrainMesh() )
		{
			NewTerrain.TerrainMesh = Terrain->GetTerrainMesh();
			NewTerrain.WorldPose = Terrain->GetPose();
			m_WorldSceneGraph->AddItem( NewTerrain );
		}
	}

	//
	// Entities
	//

	m_BuildSceneGraphVisEntCache.Clear( false );
	GetVisibleEnts( m_CameraRegionId , m_BuildSceneGraphVisEntCache );
	for( const EGEnt* Ent : m_BuildSceneGraphVisEntCache )
	{
		CalculateClosestLights( const_cast<EGEnt*>(Ent) );
		Ent->AddToSceneGraph( m_WorldSceneGraph );
	}
}

void EGEntWorld::CullEnts()
{
	assert( m_bRequestAllowed );

	m_bRequestAllowed = false;

	for( const eg_ent_id& EntId : m_DestroyQueue )
	{		
		DeinitEnt( EntId );
	}
	m_DestroyQueue.Clear();

	m_bRequestAllowed = true;
}
