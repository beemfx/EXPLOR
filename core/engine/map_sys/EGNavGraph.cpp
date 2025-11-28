// (c) 2016 Beem Media

#include "EGNavGraph.h"
#include "EGGame.h"

const EGNavGraphVertex EGNavGraphVertex::GetEdge( const EGGame* Game, eg_uint Index ) const
{
	EGNavGraphVertex Out( CT_Clear ); 
	Out.m_Owner = m_Owner; 
	Game->SDK_Graph_GetVertex( m_Edges[Index], &Out ); 
	return Out;
}

const eg_uint EGNavGraphVertex::GetOwnerIndex( const EGGame* Game ) const
{
	//This is not very efficient right now.
	EGNavGraph Graph( CT_Clear );
	eg_bool Found = Game->SDK_Graph_Find( m_Owner, &Graph );
	if( !Found ) { assert( false ); return 0; }

	EGNavGraphVertex FirstVertex( CT_Clear );
	Found = Graph.GetVertex( Game, 0, &FirstVertex );
	if( !Found ) { assert( false ); return 0; }
	eg_int Index = m_Id.Id - FirstVertex.m_Id.Id;
	assert( Index >= 0 );
	return Index;
}

eg_bool EGNavGraph::GetVertex( const EGGame* Game, eg_uint Index, EGNavGraphVertex* VertexOut ) const
{
	VertexOut->m_Owner = m_Id; 
	return Game->SDK_Graph_GetVertexInternal( m_InternalGraphId, Index, VertexOut );
}

eg_uint EGNavGraph::GetVertexIndexClosestTo( const EGGame* Game, const eg_vec4& Position ) const
{
	if( 0 == m_NumVertexes )return 0;

	EGNavGraphVertex Vertex( CT_Clear );
	GetVertex( Game, 0, &Vertex );
	eg_uint FoundIndex = 0;
	eg_real FoundDistSq = ( Vertex.GetPosition() - Position ).LenSqAsVec3();

	for( eg_uint i = 1; i<m_NumVertexes; i++ )
	{
		GetVertex( Game, i, &Vertex );
		eg_real DistSq = ( Vertex.GetPosition() - Position ).LenSqAsVec3();
		if( DistSq < FoundDistSq )
		{
			FoundIndex = i;
			FoundDistSq = DistSq;
		}
	}

	return FoundIndex;
}

eg_uint EGNavGraph::FindShortestPathBetween( const EGGame* Game, const eg_vec4& Start, const eg_vec4& End, eg_uint* IndexesOut, eg_size_t MaxIndexesOut ) const
{
	return Game->SDK_Graph_FindShortestPathBetween( this, Start, End, IndexesOut, MaxIndexesOut );
}