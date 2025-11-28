// (c) 2017 Beem Media

#pragma once

#include "EGEntTypes.h"
#include "EGWeakPtr.h"
#include "EGDelegate.h"
#include "EGItemMap.h"
#include "EGCppLibAPI.h"

struct egNetCommId;
class EGEnt;
class EGGame;
class EGGameMap;
class EGTerrain2;
class EGPhysSim2;
class EGSaveGame;
class EGNetMsgBuffer;
class EGEntWorldMap;
class EGEntWorldTerrain;
class EGEntWorldRegion;
class EGCamera2;
class EGDebugSphere;
class EGDebugBox;
class EGEntObj;
class EGGameLoader;
class EGEntWorldLoader;
class EGWorldSceneGraph;

struct egEntWorldRaycastResult
{
	eg_phys_group Group = eg_phys_group::Unknown;
	eg_vec4       HitPoint = eg_vec4( CT_Clear );
	eg_ent_id     HitEnt = INVALID_ID;
};

class EGEntWorld : public EGObject
{
	EG_CLASS_BODY( EGEntWorld , EGObject )

private:

	struct egChangeActive
	{
		eg_ent_id EntId;
		eg_bool   bActive;
	};

	struct egDelayedEntEvent : public IListable
	{
		eg_ent_id     EntId;
		eg_string_crc EventId;
	};

	typedef EGArray<egChangeActive> EGChangeActiveQueue;
	typedef EGArray<eg_ent_id> EGDestroyQueue;
	typedef EGArray<EGEnt*> EGNewSpawnedQueue;
	typedef EGList<egDelayedEntEvent> EGDelayedEntEvents;
	typedef EGItemMap<eg_region_id,EGEntWorldRegion*> EGRegionMap;
	typedef std::function<void(const EGEnt*)> EGEntCb;

private:

	eg_ent_world_role             m_Role = eg_ent_world_role::Unknown;
	EGWeakPtr<EGObject>           m_Owner;
	EGGame*                       m_Game;
	EGNetMsgBuffer*               m_NetMsgBuffer;
	EGGameLoader*                 m_GameLoader = nullptr;
	EGWorldSceneGraph*            m_WorldSceneGraph;
	eg_real                       m_EntUpdateRate = 0.f;
	EGPhysSim2*                   m_PhysSim = nullptr;
	EGArray<EGEnt*>               m_MasterList;
	EGArray<EGEntWorldMap*>       m_Maps;
	EGArray<EGEntWorldTerrain*>   m_Terrains;
	EGArray<EGEntWorldLoader*>    m_PreLoadingWorlds;
	EGArray<EGEntWorldLoader*>    m_PostLoadingWorlds;
	EGEntList                     m_InactiveEnts;
	EGEntList                     m_ActiveEnts;
	EGEntWorldRegion*             m_AllRegionEnts = nullptr;
	EGRegionMap                   m_RegionEnts;
	eg_vec4                       m_ActivityPos = eg_vec4(0.f,0.f,0.f,1.f);
	eg_region_id                  m_ActivityRegion = eg_region_id::NoRegion;
	mutable eg_region_id          m_CameraRegionId = eg_region_id::NoRegion;
	mutable EGArray<const EGEnt*> m_BuildSceneGraphVisEntCache;
	EGEntList                     m_NetSmoothingEnts;
	EGNewSpawnedQueue             m_NewSpawnedQueue;
	EGDestroyQueue                m_DestroyQueue;
	EGChangeActiveQueue           m_ChangeActiveQueue;
	EGDelayedEntEvents            m_DelayedEntEvents = EGDelayedEntEvents::DEFAULT_ID;
	EGArray<eg_ent_id>            m_DirtyEntList;
	eg_string_small               m_PhysClass;
	EGClass*                      m_GameClass = nullptr;
	eg_vec4                       m_PhysGravity = eg_vec4( 0.f , -9.8f , 0.f , 0.f );
	eg_uint                       m_LastDirtyEntSendSize = 0;
	eg_uint                       m_GameTimeMs = 0;
	eg_int                        m_PendingMapLoads = 0;
	eg_int                        m_PendingTerrainLoads = 0;
	eg_bool                       m_bRequestAllowed = true;
	eg_bool                       m_bIsUpdating = false;
	eg_bool                       m_bDebugWireFrameTerrain = false;
	EGArray<eg_lockstep_id>       m_ClientsInWorld;

public:

	EGMCDelegate<EGGameMap*>  PrimaryMapChangedDelegate;
	EGMCDelegate<EGGameMap*>  MapLoadedDelegate;
	EGMCDelegate<EGTerrain2*> TerrainLoadedDelegate;
	EGMCDelegate<>            SaveGameLoadCompleteDelegate;
	EGMCDelegate<>            PostLoadWorldComplete;

public:

	EGEntWorld();

	void InitWorld( eg_ent_world_role Role , EGNetMsgBuffer* NetMsgBuffer , EGObject* Owner , EGClass* GameClass );
	virtual void OnDestruct() override { DeinitWorld(); Super::OnDestruct(); }

	void BeginPhysSim( eg_cpstr ClassName );
	void EndPhysSim();

	void SetGameTimeMs( eg_uint InGameTimeMs ) { m_GameTimeMs = InGameTimeMs; }
	eg_uint GetGameTimeMs() const { return m_GameTimeMs; }
	void Update( eg_real DeltaTime );
	void SetActivityPos( const eg_vec4& Pos );
	void SetEntUpdateRate( eg_real NewRate ) { m_EntUpdateRate = NewRate; }
	eg_real GetEntUpdateRate() const { return m_EntUpdateRate; }

	EGEnt* GetEnt( eg_ent_id EntId ) { return m_MasterList.IsValidIndex( eg_ent_id::IdToIndex( EntId ) ) ? m_MasterList[eg_ent_id::IdToIndex( EntId )] : nullptr; }
	const EGEnt* GetEnt( eg_ent_id EntId ) const { return m_MasterList.IsValidIndex( eg_ent_id::IdToIndex( EntId ) ) ? m_MasterList[eg_ent_id::IdToIndex( EntId )] : nullptr; }
	
	template<typename RetType>
	RetType* GetEnt( eg_ent_id EntId ) { return EGCast<RetType>( GetEnt( EntId ) ); }

	eg_ent_world_role GetRole() const { return m_Role; }
	EGObject* GetOwner() { return m_Owner.GetObject(); }
	const EGObject* GetOwner() const { return m_Owner.GetObject(); }
	template<typename T> T* GetOwner() { return EGCast<T>(GetOwner()); }
	template<typename T> const T* GetOwner() const { return EGCast<T>(GetOwner()); }
	EGPhysSim2* GetPhysSim() { return m_PhysSim; }

	void ClearWorld();
	void LoadGame( EGGameLoader& Loader );
	void ResetGame();
	EGGameMap* SpawnMap( eg_cpstr Filename );
	void DestroyMap( EGGameMap* GameMap );
	EGTerrain2* SpawnTerrain( eg_cpstr Filename , const eg_transform& Pose = CT_Default );
	void DestroyTerrain( EGTerrain2* Terrain );
	void MoveTerrain( EGTerrain2* GameTerrain , const eg_transform& NewPose );
	EGEnt* SpawnEnt( const eg_string_crc& EntDef , const eg_transform& Pose = CT_Default , eg_cpstr InitString = "" );
	void DestroyEnt( const eg_ent_id& EntId );
	void SetEntActive( const eg_ent_id& EntId , eg_bool bActive );
	void GetAllEnts( EGArray<eg_ent_id>& OutIds ) const;
	void PrintDebugInfo();
	EGGame* GetGame() { return m_Game; }
	const EGGame* GetGame() const { return m_Game; }
	void FullyReplicateToClient( const egNetCommId& DestId ) const;
	EGGameMap* GetPrimaryMap();
	const EGGameMap* GetPrimaryMap() const;
	void SpawnMapEnts( const EGGameMap* GameMap );
	void LoadWorld( eg_cpstr Filename );

	void SetGravity( const eg_vec3& NewGravityVector );

	eg_region_id GetRegionOfPos( const eg_vec4& Pos , const eg_region_id& LastKnownRegion ) const;
	EGEntWorldRegion* GetRegion( const eg_region_id& RegionId );
	const EGEntWorldRegion* GetRegion( const eg_region_id& RegionId ) const;
	
	void UpdateScenGraph( const EGCamera2& Camera ) const;
	const EGWorldSceneGraph* GetScenGraph() const { return m_WorldSceneGraph; }
	void DrawEntBBs( EGDebugBox* DbBox ) const;
	void DrawLights( const eg_ent_id& CloseToEnt , EGDebugSphere* DbLight ) const;
	void DrawMapBBs() const;
	void DrawMapGraphs( EGDebugSphere* VertexSphere ) const;
	void DrawMapPortals( EGDebugSphere* DbPortal ) const;

	egEntWorldRaycastResult RaycastWorld( const eg_vec4& Origin , const eg_vec4& NormalizedDir );
	
	void SaveToSaveGame( EGSaveGame& Out );

	void Net_MarkEntDirty( const eg_ent_id& EntId );
	void Net_BroadcastDirtyEnts( eg_uint GameTimeMs );
	void Net_BroadcastSpawnEnt( const eg_ent_id& EntId ) const;
	void Net_BroadcastDestroyEnt( const eg_ent_id& EntId ) const;
	void Net_RunEntEvent( const eg_ent_id& EntId , const eg_string_crc& EventId );
	eg_uint Net_GetDirtyEntSendSize() const { return m_LastDirtyEntSendSize; }
	void Net_ReplicateGameData( eg_size_t DataStartOffset , eg_size_t DataSize ) const;
	void Net_ReplicateEntData( const eg_ent_id& EntId , eg_size_t DataStartOffset , eg_size_t DataSize ) const;
	void Net_RequestSpawnInfoForEnt( const eg_ent_id& EntId ) const;
	eg_bool Net_Smooth( EGEnt* Ent , eg_real DeltaTime );

	void HandleNetMsg( const struct egNetMsgData& NetMsg );

	void HandleMapLoaded( EGEntWorldMap* WorldMap );
	void HandleTerrainLoaded( EGEntWorldTerrain* WorldTerrain );

	eg_bool IsMapStillLoading( const EGGameMap* GameMap ) const;
	eg_bool IsTerrainStillLoading( const EGTerrain2* GameTerrain ) const;

	void OnClientEnterWorld( const eg_lockstep_id& LockstepId );
	void OnClientLeaveWorld( const eg_lockstep_id& LockstepId );

	eg_bool IsLoadingWorlds() const;

private:

	void DeinitWorld();
	EGEnt* SpawnEntInternal( const egEntCreateParms& CreateParms , eg_ent_id DesiredEntId );
	EGGameMap* SpawnMapInternal( eg_cpstr Filename , eg_spawn_reason Reason );
	EGTerrain2* SpawnTerrainInternal( eg_cpstr Filename , eg_spawn_reason Reason , const eg_transform& Pose );
	void HandleGameChanged();
	void GetVisibleEnts( const eg_region_id& RootRegion , EGArray<const EGEnt*>& Out ) const;
	void CalculateClosestLights( EGEnt* Ent ) const;
	void DestroyEntInternal( const eg_ent_id& EntId , eg_bool bDestroyEvenIfProxy );
	void DestroyMapInternal( EGEntWorldMap* WorldMap );
	void DestroyRegion( const eg_region_id& RegionId );
	void MoveEntToRegion( EGEnt* Ent , const eg_region_id& RegionId );
	void CullEnts();
	void ProcessRequestQueues();
	void SetEntToUpdateInternal( const eg_ent_id EntId , eg_bool bActive );
	void ExpandMasterList( eg_ent_id BiggestEntId );
	void DeinitEnt( const eg_ent_id& EntId );
	void UpdateEntRegion( EGEnt* Ent );
	void RebuildPhysSim();
	void ResetGameInternal();

	void OnSaveMapLoaded( EGGameMap* GameMap );
	void OnSaveTerrainLoaded( EGTerrain2* Terrain );
	void HandleThingsLoaded();
	void FinalizeGameLoad();

	void HandleNetMsg_Clear();
	void HandleNetMsg_BeginFullReplication();
	void HandleNetMsg_EndFullReplication();
	void HandleNetMsg_SpawnEnt( const struct egNet_EW_SpawnEnt& Parms );
	void HandleNetMsg_DestroyEnt( const struct egNet_EW_DestroyEnt& Parms );
	void HandleNetMsg_UpdateEnt( const struct egNet_EW_UpdateEnt& Parms );
	void HandleNetMsg_RunEntEvent( const struct egNet_EW_RunEntEvent& Parms );
	void HandleNetMsg_ReplicateEntData( const struct egNet_EW_ReplicateEntData& Parms );
	void HandleNetMsg_ReplicateGameData( const struct egNet_EW_ReplicateGameData& Parms );
	void HandleNetMsg_RequestEntSpawnInfo( const struct egNet_EW_RequestEntSpawnInfo& Parms );
	void HandleNetMsg_SpawnMap( const struct egNet_EW_SpawnFile& Parms );
	void HandleNetMsg_DestroyMap( const struct egNet_EW_SpawnFile& Parms );
	void HandleNetMsg_SpawnTerrain( const struct egNet_EW_SpawnFile& Parms );
	void HandleNetMsg_DestroyTerrain( const struct egNet_EW_SpawnFile& Parms );
	void HandleNetMsg_ResetGame();

	eg_bool Net_CanGetServerUpdates() const;

	void UpdateLoadingWorlds();
	void ClearLoadingWorlds();
};
