// EGGame - The class that defines the rules for the game
// this is shared by both the server and client. The server is meant to be
// the master of this data, as it is responsible for serializing the data
// and also replication.
//
// Each game is meant to override this class and implement it to suit thier
// needs.
//
// (c) 2016 Beem Media
#pragma once

#include "EGGameTypes.h"
#include "EGNavGraph.h"
#include "EGRemoteEvent.h"
#include "EGEntTypes.h"
#include "EGTextFormat.h"
#include "EGWeakPtr.h"
#include "EGRemoteEvent.h"

class EGEntWorld;
class EGServer;
class EGClient;
class EGMenuStack;
class EGEnt;
class EGGameMap;
class EGTerrain2;
class EGFileData;
struct eg_ent_id;
struct egRemoteEvent;
struct eg_lockstep_id;
struct egLockstepCmds;
struct egEntCreateParms;

class EGGame : public EGObject , public IEGCustomFormatHandler
{
	EG_CLASS_BODY( EGGame , EGObject )

protected:

	EGWeakPtr<EGEntWorld> m_EntWorldOwner;
	eg_ent_world_role     m_WorldRole = eg_ent_world_role::Unknown;
	EGREHandler2          m_REHandler;

public:

	virtual void Init( EGEntWorld* WorldOwner );
	virtual void Update( eg_real DeltaTime ) { unused( DeltaTime ); }
	virtual void OnFullReplication() const { }
	virtual void OnMapLoaded( const EGGameMap* GameMap ) { unused( GameMap ); }
	virtual void OnMapDestroyed( const EGGameMap* GameMap ) { unused( GameMap ); }
	virtual void OnTerrainLoaded( const EGTerrain2* GameTerrain ) { unused( GameTerrain ); }
	virtual void OnTerrainDestroyed( const EGTerrain2* GameTerrain ) { unused( GameTerrain ); }
	virtual void OnSaveGameLoaded() { }
	virtual void OnClientConnected( const eg_lockstep_id& LockstepId ) { unused( LockstepId ); }
	virtual void OnClientDisconnected( const eg_lockstep_id& LockstepId ) { unused( LockstepId ); }
	virtual void OnClientEnterWorld( const eg_lockstep_id& LockstepId ) { unused( LockstepId ); }
	virtual void OnClientLeaveWorld( const eg_lockstep_id& LockstepId ) { unused( LockstepId ); }
	virtual void PostSave( EGFileData& GameDataOut ){ unused( GameDataOut ); }
	virtual void PostLoad( const EGFileData& GameDataIn ){ unused( GameDataIn ); }
	virtual void OnDataReplicated( const void* Offset , eg_size_t Size ) { unused( Offset , Size ); }

	eg_bool IsServer() const { return m_WorldRole == eg_ent_world_role::Server; }
	eg_bool IsClient() const { return m_WorldRole == eg_ent_world_role::Client; }

	EGClient* GetClientOwner();
	const EGClient* GetClientOwner() const;
	EGServer* GetServerOwner();
	const EGServer* GetServerOwner() const;
	void SDK_ReplicateToClient( const void* Chunk , eg_size_t Size ) const;
	void SDK_ReplicateToServer( const void* Chunk , eg_size_t Size ) const;
	void SDK_RunClientEvent( const eg_string_crc& Event, const eg_event_parms& Parms = eg_event_parms( CT_Clear ) );
	void SDK_RunServerEvent( const eg_string_crc& Event, const eg_event_parms& Parms = eg_event_parms( CT_Clear ) );
	void SDK_LoadLevel( eg_cpstr Filename , eg_bool PrependUserPath = false );
	void SDK_SaveGameState( eg_cpstr Filename );
	EGEnt* SDK_GetEnt( eg_ent_id EntId );
	const EGEnt* SDK_GetEnt( eg_ent_id EntId ) const;
	void SDK_GetClientMouseTarget( eg_lockstep_id LockStepId , eg_vec3& OrgOut , eg_vec3& DirOut ) const;
	void SDK_GetCommandsForPlayer( eg_lockstep_id LockstepId, egLockstepCmds* Out ) const;
	EGEnt* SDK_SpawnEnt( const eg_string_crc& EntDef , const eg_transform& Pose = CT_Default , eg_cpstr InitString = "" );
	void SDK_DestroyEnt( const eg_ent_id EntId );
	const EGGameMap* SDK_GetMap() const{ return GetMap(); }
	void SDK_GetMapTags( EGArray<egMapTag>& ArrayOut ) const;
	egMapTag SDK_GetMapTag( eg_cpstr TagId ) const;
	EGNavGraph SDK_GetNavGraph( eg_nav_graph_id Id ) const;
	EGNavGraph SDK_FindNearestGraph( const eg_string_crc Type , const eg_vec4& Position ) const;
	egRaycastInfo SDK_RayCastFromEnt( const eg_ent_id EntId, const eg_vec4& Direction ) const;
	EGMenuStack* SDK_GetMenuStack();
	eg_bool HandleRemoteEvent( const eg_lockstep_id& SenderId , const egRemoteEvent& Event );

protected:

	virtual void FormatText( eg_cpstr Flags , class EGTextParmFormatter* Formatter ) const override { unused( Flags ); Formatter->SetText( L"Override Format Text" ); }

private:

	const EGGameMap* GetMap() const;

friend class EGNavGraph;
friend class EGNavGraphVertex;

	eg_bool SDK_Graph_Find( eg_nav_graph_id Id, class EGNavGraph* Out )const;
	eg_bool SDK_Graph_FindNearest( const eg_string_crc Type, const eg_vec4& Position, class EGNavGraph* Out )const;
	eg_bool SDK_Graph_GetVertexInternal( eg_uint GraphIndexInternal, eg_uint Index, class EGNavGraphVertex* VertexOut )const;
	eg_bool SDK_Graph_GetVertex( eg_nav_graph_vertex_id VertexId, class EGNavGraphVertex* VertexOut )const;
	eg_uint SDK_Graph_FindShortestPathBetween( const EGNavGraph* Graph, const eg_vec4& Start, const eg_vec4& End, eg_uint* IndexesOut, eg_size_t MaxIndexesOut )const;
};