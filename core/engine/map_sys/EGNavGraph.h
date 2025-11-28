// (c) 2016 Beem Media

// EGNavGraph and EGNavGraphVertex actually internally depend upon EGGame
// so they are serializable within game or entity data.

#pragma once

class EGGame;

struct eg_nav_graph_id
{
	eg_string_crc Id;

	eg_nav_graph_id() = default;
	eg_nav_graph_id( eg_string_crc InitId ): Id(InitId){ }
};

struct eg_nav_graph_vertex_id
{
	eg_uint Id;

	eg_nav_graph_vertex_id() = default;
	explicit eg_nav_graph_vertex_id( eg_uint InIntId ): Id( InIntId) { }
	eg_nav_graph_vertex_id( egInvalidId InInvalid ) { unused(InInvalid); Id = 0; }
};

class EGNavGraphVertex
{
friend class EGGame;
friend class EGNavGraph;

public:

	EGNavGraphVertex(): EGNavGraphVertex( CT_Clear ) { }
	EGNavGraphVertex( const EGNavGraphVertex& rhs ) = default;
	EGNavGraphVertex( eg_ctor_t Ct )
	{
		if( Ct == CT_Default || Ct == CT_Clear )
		{
			m_bValid = false;
			m_Owner = eg_crc("");
			m_Id = INVALID_ID;
			m_Pos = eg_vec4(0.f,0.f,0.f,1.f);
			m_NumEdges = 0;
		}
	}

	eg_bool IsValid() const { return m_bValid; }
	const eg_uint           GetIndex()const{ return m_Id.Id-1; }
	const eg_vec4&          GetPosition()const{ return m_Pos;}
	const eg_uint           GetNumEdges()const{ return m_NumEdges; }
	const EGNavGraphVertex  GetEdge( const EGGame* Engine , eg_uint Index )const;
	const eg_uint           GetOwnerIndex( const EGGame* Engine )const;

private:
	
	eg_bool                m_bValid;
	eg_nav_graph_id        m_Owner; //Not valid data right now, but may be used in the future if necessary.
	eg_nav_graph_vertex_id m_Id;
	eg_vec4                m_Pos;
	eg_uint                m_NumEdges;
	eg_nav_graph_vertex_id m_Edges[10]; //Increase if necessary, but a graph vertex shouldn't really have that many edges.

	friend inline eg_bool operator==( const EGNavGraphVertex& lhs , const EGNavGraphVertex& rhs )
	{
		eg_bool IsEqual = lhs.m_Owner.Id == rhs.m_Owner.Id && lhs.m_Id.Id == rhs.m_Id.Id;
		assert( !IsEqual || rhs.m_NumEdges == lhs.m_NumEdges ); // If these are the same these checks should pass.
		return IsEqual;
	}
};

class EGNavGraph
{
friend class EGGame;

public:

	EGNavGraph( const EGNavGraph& rhs ) = default;
	EGNavGraph( eg_ctor_t Ct )
	{
		if( Ct == CT_Clear || Ct == CT_Default )
		{
			m_Id = eg_crc("");
			m_InternalGraphId = 0;
			m_NumVertexes = 0;
		}
	}

	eg_nav_graph_id GetId()const{ return m_Id; }
	eg_bool IsValid()const{ return m_Id.Id != eg_crc("") && 0 != m_InternalGraphId; }
	eg_uint GetVertexCount()const{ return m_NumVertexes; }
	eg_bool GetVertex( const EGGame* Engine , eg_uint Index , EGNavGraphVertex* VertexOut )const;
	eg_uint GetVertexIndexClosestTo( const EGGame* Engine , const eg_vec4& Position )const;
	eg_uint FindShortestPathBetween( const EGGame* Engine , const eg_vec4& Start , const eg_vec4& End , eg_uint* IndexesOut, eg_size_t MaxIndexesOut )const; //Returns the number of indexes.

private:

	eg_nav_graph_id m_Id;
	eg_uint         m_InternalGraphId;
	eg_uint         m_NumVertexes;
};

