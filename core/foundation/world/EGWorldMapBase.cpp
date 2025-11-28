// (c) 2007 Beem Media

#include "EGWorldMapBase.h"

///////////////////////////////////////////////////////////////////////////////
// GRAPH INFORMATION:
//
// The graph interface is not a wildly easy to understand interface, but it
// is efficient and is stored in flat memory.
//
// Basically a map can contain any number of graphs. A graph consists of the
// first vertex, and then the number of vertexes. The vertexes are just a list
// of every vertex in the map. So the graph refers to that list. When you get
// a graph's vertex, you get the vertex id. A vertex knows nothing about the
// graph it is in, it only knows what it's edges are. The egresource map
// converter should guarantee, however, that a vertex will only point at edges
// that are actually in the graph that was declared.
// Now, the vertex edges, are just another VertexId, which is it's position
// in the master list. A value of 0 for a graph or a vertex is invalid, 
// and indicates that no such vertex or graph exists.
///////////////////////////////////////////////////////////////////////////////


EGWorldMapBase::~EGWorldMapBase()
{
	assert( nullptr == m_pMem ); //Map may have still been loaded?
}

eg_bool EGWorldMapBase::IsLoaded()const
{
	return m_H.nID == MAP_ID && m_H.nVersion == MAP_VERSION;
}

const eg_uint EGWorldMapBase::GetPortalCount(const eg_uint nRegion)const
{
	return EG_IsBetween<eg_uint>(nRegion, 1, m_H.nRegionsCount)?m_pRegions[nRegion-1].nPortalCount:0;
}

const EGWorldMapBase::egPortal* EGWorldMapBase::GetPortal(const eg_uint nRegion, eg_uint nPortal)const
{
	if(EG_IsBetween<eg_uint>(nRegion, 1, m_H.nRegionsCount))
	{
		eg_uint nRPortal = m_pRegions[nRegion-1].nFirstPortal + nPortal - 1;
		return EG_IsBetween<eg_uint>(nRPortal, 1, m_H.nPortalsCount)?&m_pPortals[nRPortal-1]:nullptr;
	}
	else
	{
		return nullptr;
	}
}

const EGWorldMapBase::egRegion* EGWorldMapBase::GetRegion( eg_uint nRegion ) const
{
	return EG_IsBetween<eg_uint>( nRegion , 1 , m_H.nRegionsCount ) ? &m_pRegions[nRegion-1] : nullptr;
}

eg_uint EGWorldMapBase::GetTagCount()const
{
	return m_H.nTagsCount;
}

eg_cpstr EGWorldMapBase::GetTagType(eg_uint Id)const
{
	assert(1 <= Id && Id <= m_H.nTagsCount);
	return &m_pStrings[m_pTags[Id-1].nStrOfsType];
}

const eg_vec4& EGWorldMapBase::GetTagPos(eg_uint Id)const
{
	assert(1 <= Id && Id <= m_H.nTagsCount);
	return m_pTags[Id-1].Pos;
}

eg_cpstr EGWorldMapBase::GetTagAtts(eg_uint Id)const
{
	assert(1 <= Id && Id <= m_H.nTagsCount);
	return &m_pStrings[m_pTags[Id-1].nStrOfsAtts];
}

void EGWorldMapBase::GetMapBounds(eg_aabb* paabbOut)const
{
	*paabbOut = m_H.aabbMapBounds;
}

const eg_uint EGWorldMapBase::GetPortalCount()const
{
	return m_H.nPortalsCount;
}

const EGWorldMapBase::egPortal* EGWorldMapBase::GetPortal(const eg_uint nPort)const
{
	if(EG_IsBetween<eg_uint>(nPort, 1, m_H.nPortalsCount))
	{
		return &m_pPortals[nPort-1];
	}
	else
	{
		return nullptr;
	}
}

eg_uint EGWorldMapBase::FindGraph( eg_string_crc Id )const
{
	for(eg_uint i=0; i<m_H.nGraphsCount; i++)
	{
		if( Id == eg_string_crc(&m_pStrings[m_pGraphs[i].nStrOfsName]))return (i+1);
	}
	return 0;
}

eg_uint EGWorldMapBase::FindGraph(eg_cpstr strGraph)const
{
	eg_string s(strGraph);
	for(eg_uint i=0; i<m_H.nGraphsCount; i++)
	{
		if(!s.CompareI(&m_pStrings[m_pGraphs[i].nStrOfsName]))return (i+1);
	}
	return 0;
}

eg_uint EGWorldMapBase::GetGraphCount()const
{
	return m_H.nGraphsCount;
}

eg_string_crc EGWorldMapBase::Graph_GetCrcId( eg_uint IdGraph )const
{
	assert( 1 <= IdGraph && IdGraph <= m_H.nGraphsCount);
	if(!(1 <= IdGraph && IdGraph <= m_H.nGraphsCount))return eg_crc(""); 

	return eg_string_crc(&m_pStrings[m_pGraphs[IdGraph-1].nStrOfsName]);
}

eg_string_crc EGWorldMapBase::Graph_GetType( eg_uint IdGraph )const
{
	assert( 1 <= IdGraph && IdGraph <= m_H.nGraphsCount);
	if(!(1 <= IdGraph && IdGraph <= m_H.nGraphsCount))return CT_Clear; 

	return m_pGraphs[IdGraph-1].Type;
}

eg_uint EGWorldMapBase::Graph_GetNumVerts(eg_uint IdGraph)const
{
	assert( 1 <= IdGraph && IdGraph <= m_H.nGraphsCount);
	if(!(1 <= IdGraph && IdGraph <= m_H.nGraphsCount))return 0;

	return m_pGraphs[IdGraph-1].nNumVerts;
}

eg_uint EGWorldMapBase::Graph_GetVertex(eg_uint IdGraph, eg_uint VertIndex)const
{
	assert( 1 <= IdGraph && IdGraph <= m_H.nGraphsCount);
	if(!(1 <= IdGraph && IdGraph <= m_H.nGraphsCount))return 0;

	assert( 1 <= VertIndex && VertIndex <= Graph_GetNumVerts(IdGraph));
	if(!(1 <= VertIndex && VertIndex <= Graph_GetNumVerts(IdGraph)))return 0;

	return m_pGraphs[IdGraph-1].nFirstVert+VertIndex;
}

eg_uint* EGWorldMapBase::Graph_GetVertexById( eg_uint IdVertex , eg_uint* NumEdgesOut , eg_vec4* PosOut )const
{
	assert( 1 <= IdVertex && IdVertex <= m_H.nGraphVertsCount);
	if(!( 1 <= IdVertex && IdVertex <= m_H.nGraphVertsCount))
	{
		if(PosOut)*PosOut = eg_vec4(0,0,0, 1);
		if(NumEdgesOut)*NumEdgesOut=0;
		return nullptr;
	}

	if(PosOut)*PosOut = m_pGraphVerts[IdVertex-1].Pos;
	if(NumEdgesOut)*NumEdgesOut = m_pGraphVerts[IdVertex-1].nNumEdges;
	return m_pGraphVerts[IdVertex-1].anEdge;
}

void EGWorldMapBase::Graph_Vertex_GetInfo(eg_uint IdVertex, eg_vec4* OutPos, eg_uint* OutEdgeCount, eg_uint* OutEdges, eg_uint MaxOut)const
{
	assert( MaxOut <= countof(m_pGraphVerts[0].anEdge) );
	MaxOut = EG_Min<eg_uint>( MaxOut , countof(m_pGraphVerts[0].anEdge) );

	assert( 1 <= IdVertex && IdVertex <= m_H.nGraphVertsCount);
	if(!( 1 <= IdVertex && IdVertex <= m_H.nGraphVertsCount))
	{
		if(OutPos)*OutPos = eg_vec4(0,0,0, 1);
		if(OutEdgeCount)*OutEdgeCount=0;
		return;
	}

	if(OutPos)*OutPos = m_pGraphVerts[IdVertex-1].Pos;
	assert(nullptr == OutEdges || m_pGraphVerts[IdVertex-1].nNumEdges <= MaxOut);
	if(OutEdgeCount)*OutEdgeCount = EG_Min(m_pGraphVerts[IdVertex-1].nNumEdges, MaxOut);
	for(eg_uint i=0; i<m_pGraphVerts[IdVertex-1].nNumEdges && i< MaxOut; i++)
	{
		if(OutEdges)OutEdges[i] = m_pGraphVerts[IdVertex-1].anEdge[i];
	}
}

const eg_uint EGWorldMapBase::GetLightCount()const
{
	return m_H.nLightsCount;
}

const eg_uint EGWorldMapBase::GetRegionCount()const
{
	return m_H.nRegionsCount;
}


const eg_vec4& EGWorldMapBase::GetSpawnLocation()const
{
	return m_H.vSpawn;
}

eg_cpstr EGWorldMapBase::GetRegionName(eg_uint nID)const
{
	nID = ::EG_Clamp<eg_uint>(nID, 0, m_H.nRegionsCount);

	if(0 == nID)
		return ("Unidentified Region");
	else
		return &m_pStrings[this->m_pRegions[nID-1].nStrOfsName];
}

void EGWorldMapBase::GetLight(egLight* pLight, eg_uint nLight)const
{
	//Hopefully the light was in range, or we'll crash.
	*pLight = this->m_pLights[nLight-1];
}

const EGWorldMapBase::egLight* EGWorldMapBase::GetLight(eg_uint nLight)const
{
	return &m_pLights[nLight-1];
}
const eg_color* EGWorldMapBase::GetRegAmb(eg_uint nRegion)const
{
	return &m_pRegions[nRegion-1].clrAmb;
}


eg_uint EGWorldMapBase::GetClosestLights_InsertLightIfCloser( eg_uint nLight , const eg_real fDist , eg_uint anLights[] , eg_real afDists[] , eg_uint nLights , eg_uint nMaxLights )
{
	//The complexity of this is nMaxLights*nMaxLights;

	eg_uint nPos   = 0;

	//Compute the next position, but if the light is already in the list, bail.
	//nPos is computed inside the for statement.
	for(nPos = 0; nPos < ::EG_Min(nLights, nMaxLights) && fDist>=afDists[nPos]; nPos++)
	{
		if(anLights[nPos] == nLight)
			return nLights;
	}

	if(nLights < nMaxLights)
	{
		//If we have less lights than the max lights, we always insert the
		//light.
		nLights++;
	}

	//We now insert he light at the specified position, moving everything else
	//down the line.
	if(nPos < nMaxLights)
	{
		for(eg_uint nMove = nMaxLights-1; nMove>nPos; nMove--)
		{
			anLights[nMove] = anLights[nMove-1];
			afDists[nMove]  = afDists[nMove-1];
		}
		anLights[nPos] = nLight;
		afDists[nPos]  = fDist;
	}

	return nLights;
}

eg_uint EGWorldMapBase::GetClosestLights( const eg_vec4& Pos , eg_uint nRegion , eg_uint anLights[] , eg_real afDistsSq[] , const eg_uint nMaxLights )
{
	//This algorithm includes the sub algorithms in InsertLightIfCloser and
	//GetCloseLtToPort. Defining the following variables we may calculate the
	//complexity.
	//
	//N - The number of lights in a region (this varies, so subscripts are specified).
	//M - The maximum number of lights to be found.
	//
	//R - The number of portals in the region (number of adjacent regions).
	//A - The number of lights in the adjacent region.
	//
	//The complexity of this algorithm is:
	//O(N_o*M*M + R*(M + N_a))
	//
	//Possible optimizations N_a could be made linear by precomputing this value
	//for each portal when the map is loaded.

	if(!EG_IsBetween<eg_uint>(nRegion, 1, m_H.nRegionsCount))
	{
		return 0;
	}

	EGFixedArray<eg_uint,50> RegionsOfInterest( CT_Clear );

	// We want the region we are in and any adjacent regions
	{
		RegionsOfInterest.Append( nRegion );
		const egRegion* InRegion = &m_pRegions[nRegion-1];
		for( eg_uint PortOffset = 0; PortOffset < InRegion->nPortalCount; PortOffset++ )
		{
			eg_uint PortIdx = InRegion->nFirstPortal + PortOffset;
			if( EG_IsBetween<eg_uint>( PortIdx , 1 , m_H.nPortalsCount ) )
			{
				const egPortal* Portal = &m_pPortals[PortIdx-1];
				if( EG_IsBetween<eg_uint>( Portal->nTo , 1 , m_H.nRegionsCount ) )
				{
					RegionsOfInterest.AppendUnique( Portal->nTo );
				}
			}
		}
	}

	/*
	This function gets the lights closest to the position specified, the return
	value is the number of lights found. The array anLights is filled with the
	data, and nMaxLights is the number of lights that the user wants to find
	(though less lights may be found). nRegion is where the checking will begin.
	*/
	eg_uint nLights=0;

	for( eg_uint RegionIdx : RegionsOfInterest )
	{
		//The process used is to check the lights in the current region, and it's
		//adjacent regions. There is really no reason to check any further than that.
		const egRegion* pReg = &m_pRegions[RegionIdx-1];
		for(eg_uint LightOffset = 0; LightOffset < pReg->nLightCount; LightOffset++)
		{
			eg_uint LightIdx = pReg->nFirstLight + LightOffset;
			const egLight* pLight = &m_pLights[LightIdx-1];
			//Caculate the distance to this light.
			eg_vec4 vDist = pLight->Pos - Pos;
			eg_real fDist  = vDist.LenSqAsVec3();
			nLights = GetClosestLights_InsertLightIfCloser( LightIdx , fDist , anLights , afDistsSq , nLights , nMaxLights );
		}
	}

	return nLights;
}

void EGWorldMapBase::ShowInfo()
{
	EGLogf( eg_log_t::General , ("Map \"%s\":"), &m_pStrings[m_H.nStrOfsName]);

	EGLogf(eg_log_t::General , ("%u bytes allocated."), m_nMemSize);
	EGLogf(eg_log_t::General , ("%u vertexes."), m_H.nVertexesCount);

	EGLogf(eg_log_t::General , ("%u materials:"), m_H.nMtrlsCount);
	for(eg_uint i=0; i<m_H.nMtrlsCount; i++)
	{
		EGLogf(eg_log_t::General , ("%u: %s (%s)."), i+1, m_pMtrls[i].Def.m_strTex[0], m_pMtrls[i].Def.m_strTex[1]);
	}

	EGLogf(eg_log_t::General , ("%u segments."), m_H.nSegmentsCount);
	EGLogf(eg_log_t::General , ("%u regions:"), m_H.nRegionsCount);
	for(eg_uint i=0; i<m_H.nRegionsCount; i++)
	{
		EGLogf(eg_log_t::General , ("\tRegion %u: %s"), i+1, this->GetRegionName(i+1));
	}
	EGLogf(eg_log_t::General , ("%u geo blocks."), m_H.nGeoBlocksCount);
	EGLogf(eg_log_t::General , ("%u lights."), m_H.nLightsCount);

	EGLogf(eg_log_t::General , ("%u Tags:"), m_H.nTagsCount);
	for(eg_uint t=0; t<m_H.nTagsCount; t++)
	{
		eg_vec4 Pos = GetTagPos(t+1);
		EGLogf(eg_log_t::General , ("\tTag %u: \"%s\" (%g %g %g %g) -> %s"), t+1, GetTagType(t+1), Pos.x, Pos.y, Pos.z, Pos.w, GetTagAtts(t+1));
	}

	//Show the graphs:
	EGLogf(eg_log_t::General , ("%u Graphs:"), m_H.nGraphsCount);
	for(eg_uint g=0; g<m_H.nGraphsCount; g++)
	{
		egGraph& G = m_pGraphs[g];
		EGLogf(eg_log_t::General , ("Graph %i: %s"), g+1, &m_pStrings[G.nStrOfsName]);
		for(eg_uint v = 0; v < G.nNumVerts; v++)
		{
			egGraphVertex& V = m_pGraphVerts[G.nFirstVert+v];
			EGLogf(eg_log_t::General , ("Vertex[%i] @ (%g %g %g) ->"), v+1, V.Pos.x, V.Pos.y, V.Pos.z);
			for(eg_uint e=0; e<V.nNumEdges; e++)
			{
				EGLogf(eg_log_t::General , ("\t%i"), V.anEdge[e]);
			}
		}
	}

}

eg_uint EGWorldMapBase::CheckRegions(const eg_aabb& pAABB, eg_uint* pRegions, eg_uint nMaxRegions, eg_uint nLastKnownR)const
{
	//CheckRegions checks to see which regions the entity is currently occupying.
	//it checks the catenated aabb against each regions aabb, so this method is meant
	//to be called after ProcessAI, but before world and entity collision testing.
	eg_uint nNumRegions=0;

	//If no map is set then we aren't occupying any rooms.	
	if(!IsLoaded() || 0 == nMaxRegions)
		return 0;

	eg_bool bFound = false;
	if(nLastKnownR > 0)
	{
		if(pAABB.Intersect( m_pRegions[nLastKnownR-1].aabb, nullptr))
		{
			pRegions[nNumRegions] = nLastKnownR;
			nNumRegions++;
			bFound = true;
		}

		//Check adjacent regions:
		for(eg_uint i=0; i<GetPortalCount(nLastKnownR) && (nNumRegions<nMaxRegions); i++)
		{
			const egPortal*const Portal = GetPortal(nLastKnownR, i+1);
			if(pAABB.Intersect(m_pRegions[Portal->nTo-1].aabb, nullptr))
			{
				pRegions[nNumRegions] = GetPortal(nLastKnownR, i+1)->nTo;
				nNumRegions++;
				bFound = true;
			}
		}
	}

	if(bFound)return nNumRegions;

	//Since it wasn't in the last known region, just search through all regions.

	for(eg_uint i=0; (i<m_H.nRegionsCount) && (nNumRegions<nMaxRegions); i++)
	{
		if(pAABB.Intersect(m_pRegions[i].aabb, nullptr))
		{
			pRegions[nNumRegions]=i+1;
			nNumRegions++;
		}
	}
	return nNumRegions;
}

eg_bool EGWorldMapBase::IsPointInRegion(const eg_vec4& vPoint, eg_uint nRegion)const
{
	assert( 1<= nRegion && nRegion <= GetRegionCount());
	const eg_aabb aabb = {vPoint, vPoint};
	return aabb.Intersect(m_pRegions[nRegion-1].aabb, nullptr);
}

void EGWorldMapBase::SetPointers(const egHeader& Header)
{
	assert( nullptr != m_pMem );
	//Create a copy just in case we are passing m_pH to this:
	egHeader H = Header;

#define MAP_DATA_SECTION( _type_ , _var_ ) m_p##_var_ = reinterpret_cast<_type_*>( &m_pMem[H.Ofs##_var_] );
#include "EGMapDataSections.inc"
#undef MAP_DATA_SECTION

	//Do some checking
#define MAP_DATA_SECTION( _type_ , _var_ ) assert_aligned( m_p##_var_ );
#include "EGMapDataSections.inc"
#undef MAP_DATA_SECTION

	m_H = H;
}
