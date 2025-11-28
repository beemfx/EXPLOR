// EGGame
// (c) 2016 Beem Media

#include "EGGame.h"
#include "EGServer.h"
#include "EGClient.h"
#include "EGGameMap.h"
#include "EGEntWorld.h"

EG_CLASS_DECL( EGGame )

void EGGame::Init( EGEntWorld* EntWorldOwner )
{
	m_EntWorldOwner = EntWorldOwner;
	m_WorldRole = m_EntWorldOwner ? m_EntWorldOwner->GetRole() : eg_ent_world_role::Unknown;

	if( m_WorldRole == eg_ent_world_role::Server )
	{
		if( GetServerOwner() == nullptr )
		{
			m_WorldRole = eg_ent_world_role::Unknown;
			assert( false );
		}
	}

	if( m_WorldRole == eg_ent_world_role::Client )
	{
		if( GetClientOwner() == nullptr )
		{
			m_WorldRole = eg_ent_world_role::Unknown;
			assert( false );
		}
	}
}

EGClient* EGGame::GetClientOwner()
{
	return m_EntWorldOwner ? m_EntWorldOwner->GetOwner<EGClient>() : nullptr;
}

const EGClient* EGGame::GetClientOwner() const
{
	return m_EntWorldOwner ? m_EntWorldOwner->GetOwner<const EGClient>() : nullptr;
}

EGServer* EGGame::GetServerOwner()
{
	return m_EntWorldOwner ? m_EntWorldOwner->GetOwner<EGServer>() : nullptr;
}

const EGServer* EGGame::GetServerOwner() const
{
	return m_EntWorldOwner ? m_EntWorldOwner->GetOwner<const EGServer>() : nullptr;
}

void EGGame::SDK_ReplicateToClient( const void* Chunk , eg_size_t Size ) const
{
	assert( IsServer() );
	if( IsServer() && m_EntWorldOwner )
	{
		eg_size_t DataStart = reinterpret_cast<eg_size_t>( Chunk );
		assert( reinterpret_cast<eg_size_t>( this ) <= DataStart && ( DataStart + Size ) <= reinterpret_cast<eg_size_t>( this ) + GetObjectSize() );
		m_EntWorldOwner->Net_ReplicateGameData( DataStart - reinterpret_cast<eg_size_t>( this ) , Size );
	}
}

void EGGame::SDK_ReplicateToServer( const void* Chunk, eg_size_t Size ) const
{
	assert( IsClient() );
	if( IsClient() && m_EntWorldOwner )
	{
		eg_size_t DataStart = reinterpret_cast<eg_size_t>( Chunk );
		assert( reinterpret_cast<eg_size_t>( this ) <= DataStart && ( DataStart + Size ) <= reinterpret_cast<eg_size_t>( this ) + GetObjectSize() );

		m_EntWorldOwner->Net_ReplicateGameData( DataStart - reinterpret_cast<eg_size_t>( this ) , Size );
	}
}

void EGGame::SDK_RunClientEvent( const eg_string_crc& Event, const eg_event_parms& Parms )
{
	egRemoteEvent ReStruct( Event, Parms );

	if( IsClient() )
	{
		EGClient* ClientOwner = GetClientOwner();
		if( ClientOwner )
		{
			ClientOwner->SDK_RunClientEvent( ReStruct );
		}
	}
	else if( IsServer() )
	{
		EGServer* ServerOwner = GetServerOwner();
		if( ServerOwner )
		{
			ServerOwner->SDK_RunClientEvent( ReStruct );
		}
	}
}

void EGGame::SDK_RunServerEvent( const eg_string_crc& Event, const eg_event_parms& Parms )
{
	egRemoteEvent ReStruct( Event , Parms );

	if( IsClient() )
	{
		EGClient* ClientOwner = GetClientOwner();
		if( ClientOwner )
		{
			ClientOwner->SDK_RunServerEvent( ReStruct );
		}
	}
	else if( IsServer() )
	{
		EGServer* ServerOwner = GetServerOwner();
		if( ServerOwner )
		{
			ServerOwner->SDK_RunServerEvent( INVALID_ID , ReStruct );
		}
	}
}

void EGGame::SDK_LoadLevel( eg_cpstr Filename , eg_bool PrependUserPath /*= false */ )
{
	assert( IsServer() );
	if( IsServer() )
	{
		GetServerOwner()->SDK_LoadLevel( Filename , PrependUserPath );
	}
}

void EGGame::SDK_SaveGameState( eg_cpstr Filename )
{
	assert( IsServer() );
	if( IsServer() )
	{
		GetServerOwner()->SDK_SaveGameState( Filename );
	}
}

EGEnt* EGGame::SDK_GetEnt( eg_ent_id EntId )
{
	return m_EntWorldOwner ? m_EntWorldOwner->GetEnt( EntId ) : nullptr;
}

const EGEnt* EGGame::SDK_GetEnt( eg_ent_id EntId ) const
{
	return m_EntWorldOwner ? m_EntWorldOwner->GetEnt( EntId ) : nullptr;
}

void EGGame::SDK_GetClientMouseTarget( eg_lockstep_id LockStepId , eg_vec3& OrgOut , eg_vec3& DirOut ) const
{
	if( IsServer() )
	{
		GetServerOwner()->SDK_GetClientMouseTarget( LockStepId , OrgOut , DirOut );
	}
	else
	{
		assert( false ); // Not implement on clients.
	}
}

void EGGame::SDK_GetCommandsForPlayer( eg_lockstep_id LockstepId, egLockstepCmds* Out ) const
{
	if( IsServer() )
	{
		GetServerOwner()->SDK_GetCommandsForPlayer( LockstepId , Out );
	}

	if( IsClient() )
	{
		assert( INVALID_ID == LockstepId );
		GetClientOwner()->SDK_GetLockstepCmds( Out );
	}
}

EGEnt* EGGame::SDK_SpawnEnt( const eg_string_crc& EntDef, const eg_transform& Pose /*= CT_Default */, eg_cpstr InitString /*= "" */ )
{
	return m_EntWorldOwner ? m_EntWorldOwner->SpawnEnt( EntDef , Pose , InitString ) : nullptr;
}

void EGGame::SDK_DestroyEnt( const eg_ent_id EntId )
{
	assert( IsServer() ); // Entities can only be destroyed on servers
	
	if( IsServer() )
	{
		GetServerOwner()->SDK_DestroyEnt( EntId );
	}
}

void EGGame::SDK_GetMapTags( EGArray<egMapTag>& ArrayOut ) const
{
	const EGGameMap* Map = GetMap();

	if( Map )
	{
		const eg_uint NumTags = Map->GetTagsCount();
		for( eg_uint i=0; i<NumTags; i++ )
		{
			const EGGameMap::egTag& Tag = Map->GetTagsByIndex( i );
			egMapTag MapTag;
			MapTag.Pose = eg_transform::BuildTranslation( Map->GetTagPos( i+1 ).ToVec3() );
			MapTag.Type = eg_string_crc( Map->GetTagType( i+1 ) );
			MapTag.Attrs = Map->GetTagAtts( i+1 );
			ArrayOut.Append( MapTag );
		}
	}
}

egMapTag EGGame::SDK_GetMapTag( eg_cpstr TagId ) const
{
	const EGGameMap* Map = GetMap();

	egMapTag TagOut;
	eg_string_crc TagCrcId = eg_string_crc( TagId );
	eg_uint FoundCount = 0;

	TagOut.Pose = eg_transform::BuildTranslation( eg_vec3( 0, 0, 0) );
	TagOut.Type = eg_crc( "" );
	TagOut.Attrs = nullptr;

	if( Map )
	{
		for( eg_uint i = 1; i <= Map->GetTagCount(); i++ )
		{
			if( eg_string_crc( Map->GetTagType( i ) ) == TagCrcId )
			{
				TagOut.Pose = eg_transform::BuildTranslation( Map->GetTagPos( i ).ToVec3() );
				TagOut.Type = eg_string_crc( Map->GetTagType( i ) );
				TagOut.Attrs = Map->GetTagAtts( i );

				FoundCount++;
			}
		}
	}

	if( 0 == FoundCount )
	{
		EGLogf( eg_log_t::Warning, __FUNCTION__ ": Did not find %s tag. Returning empty tag.", TagId );
	}
	else if( FoundCount > 1 )
	{
		EGLogf( eg_log_t::Warning, __FUNCTION__ ": Found %u %s tags. Returning last one found.", FoundCount, TagId );
	}

	return TagOut;
}

EGNavGraph EGGame::SDK_GetNavGraph( eg_nav_graph_id Id ) const
{
	EGNavGraph Out( CT_Clear );
	SDK_Graph_Find( Id , &Out );
	return Out;
}

EGNavGraph EGGame::SDK_FindNearestGraph( const eg_string_crc Type, const eg_vec4& Position ) const
{
	EGNavGraph Out( CT_Clear );
	SDK_Graph_FindNearest( Type , Position , &Out );
	return Out;
}

egRaycastInfo EGGame::SDK_RayCastFromEnt( const eg_ent_id EntId, const eg_vec4& Direction ) const
{
	egRaycastInfo Out;
	zero( &Out );

	if( IsServer() )
	{
		GetServerOwner()->SDK_RayCastFromEnt( EntId , Direction , &Out );
	}

	return Out;
}

EGMenuStack* EGGame::SDK_GetMenuStack()
{
	EGMenuStack* Out = nullptr;

	assert( IsClient() );
	if( IsClient() )
	{
		Out = GetClientOwner()->SDK_GetMenuStack();
	}

	return Out;
}

eg_bool EGGame::HandleRemoteEvent( const eg_lockstep_id& SenderId , const egRemoteEvent& Event )
{
	unused( SenderId );

	return m_REHandler.ExecuteEvent( Event );
}

const EGGameMap* EGGame::GetMap() const
{
	return m_EntWorldOwner ? m_EntWorldOwner->GetPrimaryMap() : nullptr;
}

eg_bool EGGame::SDK_Graph_GetVertex( eg_nav_graph_vertex_id VertexId, class EGNavGraphVertex* VertexOut )const
{
	const EGGameMap* Map = GetMap();

	VertexOut->m_Id = VertexId;
	if( 0 == VertexId.Id || Map == nullptr )
	{
		VertexOut->m_NumEdges = 0;
		VertexOut->m_Pos = eg_vec4( 0, 0, 0, 1 );
		VertexOut->m_bValid = false;
		assert( false ); //No such vertex.
		return false;
	}


	eg_uint* Edges = Map->Graph_GetVertexById( VertexOut->m_Id.Id, &VertexOut->m_NumEdges, &VertexOut->m_Pos );
	assert( VertexOut->m_NumEdges <= countof( VertexOut->m_Edges ) );
	VertexOut->m_NumEdges = EG_Min<eg_uint>( VertexOut->m_NumEdges, countof( VertexOut->m_Edges ) );
	for( eg_uint i = 0; i<VertexOut->m_NumEdges; i++ )
	{
		VertexOut->m_Edges[i].Id = Edges[i];
	}
	VertexOut->m_bValid = true;
	return true;
}

eg_bool EGGame::SDK_Graph_GetVertexInternal( eg_uint GraphIndexInternal, eg_uint Index, class EGNavGraphVertex* VertexOut )const
{
	const EGGameMap* Map = GetMap();

	if( nullptr == Map || !(1 <= GraphIndexInternal && GraphIndexInternal <= Map->GetGraphCount() ) )
	{
		VertexOut->m_NumEdges = 0;
		VertexOut->m_Pos = eg_vec4( 0 , 0 , 0 , 1 );
		VertexOut->m_bValid = false;
		assert( false ); // No such vertex.
		return false;
	}
	eg_nav_graph_vertex_id Id( Map->Graph_GetVertex( GraphIndexInternal , Index + 1 ) );
	return SDK_Graph_GetVertex( Id, VertexOut );
}

eg_bool EGGame::SDK_Graph_Find( eg_nav_graph_id Id, class EGNavGraph* Out )const
{
	const EGGameMap* Map = GetMap();

	Out->m_Id = Id;
	Out->m_InternalGraphId = Map ? Map->FindGraph( Id.Id ) : 0;
	if( 0 == Out->m_InternalGraphId )
	{
		Out->m_Id.Id = eg_crc( "" );
		Out->m_NumVertexes = 0;
		return false;
	}
	Out->m_NumVertexes = Map->Graph_GetNumVerts( Out->m_InternalGraphId );
	return true;
}

eg_bool EGGame::SDK_Graph_FindNearest( const eg_string_crc Type, const eg_vec4& Position, class EGNavGraph* Out ) const
{
	const EGGameMap* Map = GetMap();

	if( Map == nullptr )
	{
		Out->m_Id.Id = eg_crc( "" );
		Out->m_InternalGraphId = 0;
		Out->m_NumVertexes = 0;
		return false;
	}

	eg_uint nClosest = 0;
	eg_vec4 vOrg( Position ), vPos;
	eg_real fDist = 0.0f;

	for( eg_uint g = 1; g <= Map->GetGraphCount(); g++ )
	{
		if( !( Type.IsNull() || Map->Graph_GetType( g ) == Type ) )continue;

		for( eg_uint v = 1; v <= Map->Graph_GetNumVerts( g ); v++ )
		{
			eg_uint IdVertex = Map->Graph_GetVertex( g, v );
			Map->Graph_Vertex_GetInfo( IdVertex, &vPos, nullptr, nullptr, 0 );
			eg_real DistSq = ( vOrg - vPos ).LenSqAsVec3();
			//We found a vertex, so set the closest
			if( 0 == nClosest )
			{
				fDist = DistSq;
				nClosest = g;
			}

			if( DistSq < fDist )
			{
				fDist = DistSq;
				nClosest = g;
			}
		}
	}

	if( 0 == nClosest )
	{
		Out->m_Id.Id = eg_crc( "" );
		Out->m_InternalGraphId = 0;
		Out->m_NumVertexes = 0;
		return false;
	}

	Out->m_InternalGraphId = nClosest;
	Out->m_Id.Id = Map->Graph_GetCrcId( nClosest );
	Out->m_NumVertexes = Map->Graph_GetNumVerts( Out->m_InternalGraphId );

	return true;
}

eg_uint EGGame::SDK_Graph_FindShortestPathBetween( const EGNavGraph* Graph, const eg_vec4& Start, const eg_vec4& End, eg_uint* IndexesOut, eg_size_t MaxIndexesOut )const
{
	//We actually do this backwards so that when we compute the list at the end
	//it's in the correct order.
	eg_uint StartVertexIndex = Graph->GetVertexIndexClosestTo( this, End );
	eg_uint EndVertexIndex = Graph->GetVertexIndexClosestTo( this, Start );

	static const eg_uint MAX_VERTEXES = 1000; //We're using stack memory so really this is limited to how much stack size might be available.
	if( Graph->m_NumVertexes > MAX_VERTEXES )
	{
		assert( false ); //Graph is too large to compute shortest path.
		return 0;
	}

	static const eg_real R_INF = 1e36f;
	static const eg_uint NODE_UNDEFINED = -1;

	eg_real Dist[MAX_VERTEXES];
	eg_uint Prev[MAX_VERTEXES];

	EGNavGraphVertex Vertex( CT_Clear );
	Graph->GetVertex( this, StartVertexIndex, &Vertex );

	Dist[StartVertexIndex] = 0;
	Prev[StartVertexIndex] = NODE_UNDEFINED;

	EGFixedArray<eg_uint, MAX_VERTEXES> Q;

	for( eg_uint v = 0; v < Graph->m_NumVertexes; v++ )
	{
		if( v != StartVertexIndex )
		{
			Dist[v] = R_INF;
			Prev[v] = NODE_UNDEFINED;
		}
		Q.Push( v );
	}

	while( Q.HasItems() )
	{
		eg_uint uIndex = NODE_UNDEFINED;
		{
			eg_uint MinDistUIndex = 0;
			eg_real CurDist = Dist[Q[MinDistUIndex]];
			for( eg_uint i = 1; i<Q.Len(); i++ )
			{
				if( Dist[Q[i]] < CurDist )
				{
					MinDistUIndex = i;
					CurDist = Dist[Q[i]];
				}
			}

			uIndex = Q[MinDistUIndex];
			Q.DeleteByIndex( MinDistUIndex );
		}

		EGNavGraphVertex u( CT_Clear );
		Graph->GetVertex( this, uIndex, &u );
		for( eg_uint i = 0; i<u.GetNumEdges(); i++ )
		{
			EGNavGraphVertex v = u.GetEdge( this, i );
			eg_uint vIndex = v.GetOwnerIndex( this );
			eg_real alt = Dist[uIndex] + ( v.GetPosition() - u.GetPosition() ).LenSqAsVec3();
			if( alt < Dist[vIndex] )
			{
				Dist[vIndex] = alt;
				Prev[vIndex] = uIndex;
			}
		}

	}

	eg_uint NumSpaces = 0;
	eg_uint uIndex = EndVertexIndex;

	while( Prev[uIndex] != NODE_UNDEFINED && NumSpaces < MaxIndexesOut )
	{
		if( NumSpaces < MaxIndexesOut )
		{
			IndexesOut[NumSpaces] = uIndex;
			NumSpaces++;
			uIndex = Prev[uIndex];
		}
		else
		{
			assert( false ); //The output array wasn't big enough to hold the path!
			return 0;
		}
	}

	if( NumSpaces < MaxIndexesOut )
	{
		IndexesOut[NumSpaces] = StartVertexIndex;
		NumSpaces++;
	}
	else
	{
		assert( false ); //The output array wasn't big enough to hold the path!
		return 0;
	}

	return NumSpaces;
}