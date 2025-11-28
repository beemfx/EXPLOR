// (c) 2007 Beem Media

#pragma once

#include "EGRendererTypes.h"
#include "EGFoundationTypes.h"

class EGWorldMapBase
{
public:

	static const eg_uint MAP_ID      = 0x50414D45; //"EMAP"
	static const eg_uint MAP_VERSION = 2;

	struct egHeader
	{
		//Header Info:
		eg_uint nID = 0;
		eg_uint nVersion = 0;
		eg_uint nStrOfsName = 0;
		eg_aabb aabbMapBounds = eg_aabb( CT_Clear );
		eg_vec4 vSpawn = eg_vec4( CT_Clear );

		//Data counts:
		#define MAP_DATA_SECTION( _type_ , _var_ ) eg_uint n##_var_##Count = 0;
		#include "EGMapDataSections.inc"
		#undef MAP_DATA_SECTION

		//Data offsets (from start of m_pMem):
		#define MAP_DATA_SECTION( _type_ , _var_ ) eg_uint Ofs##_var_ = 0;
		#include "EGMapDataSections.inc"
		#undef MAP_DATA_SECTION
	};

	struct egMtrl
	{
		EGMaterialDef Def;
		egv_material  Mtrl; //The actual material (only set by the EGGameMap class).
	};

	struct egGraph
	{
		eg_uint       nStrOfsName;
		eg_string_crc Type;
		eg_uint       nFirstVert;
		eg_uint       nNumVerts;
	};

	struct egGraphVertex
	{
		eg_vec4 Pos;
		eg_uint nNumEdges;
		eg_uint anEdge[20];
	};

	struct egRasterSegment
	{
		eg_uint nFirst;
		eg_uint nTriCount;
		eg_uint nMtrlRef;

		egv_material mtrl; //The actual material (only set by the EGGameMap class).
	};

	struct egRegion
	{
		eg_uint nStrOfsName;
		eg_uint nFirstRasterSeg;
		eg_uint nRasterSegCount;
		eg_uint nFirstGeoBlock;
		eg_uint nGeoBlockCount;
		eg_uint nFirstLight;
		eg_uint nLightCount;
		eg_uint nFirstPortal;
		eg_uint nPortalCount;
		eg_aabb  aabb;
		eg_color clrAmb; //Ambient color of the region.
	};

	struct egPortal
	{
		eg_uint   nID;
		eg_t_sphere sphPortal;
		eg_uint   nFrom;
		eg_uint   nTo;
		eg_bool   bTerminal:1;
		eg_bool   bAlwaysVisible:1;
	};

	struct egLight
	{
		eg_vec4  Pos;
		eg_color Color;
		eg_real  fRangeSq;
	};

	struct egGeoBlock
	{
		eg_aabb aabb;
	};

	struct egTag
	{
		eg_uint nStrOfsType;
		eg_uint nStrOfsAtts;
		eg_vec4 Pos;
	};

	struct egGeoAlignedBox
	{
		eg_aabb Bounds;
	};

public:

	#define MAP_DATA_SECTION( _type_ , _var_ ) eg_uint Get##_var_##Count()const{ return m_H.n##_var_##Count; }
	#include "EGMapDataSections.inc"
	#undef MAP_DATA_SECTION

	#define MAP_DATA_SECTION( _type_ , _var_ ) const _type_& Get##_var_##ByIndex( eg_uint Index )const{ return m_p##_var_[Index]; }
	#include "EGMapDataSections.inc"
	#undef MAP_DATA_SECTION

protected:

	//////////////////////////
	//BINARY FILE FORMAT START
	#define MAP_DATA_SECTION( _type_ , _var_ ) _type_* m_p##_var_ = nullptr;
	#include "EGMapDataSections.inc"
	#undef MAP_DATA_SECTION
	//BINARY FILE FORMAT END
	////////////////////////

	//The memory lump (contains above data):
	eg_byte*  m_pMem = nullptr;
	eg_size_t m_nMemSize = 0;	

	egHeader  m_H = egHeader();

public:

	~EGWorldMapBase();

	eg_bool IsLoaded()const;

	void ShowInfo();

	eg_uint CheckRegions(const eg_aabb& pAABB, eg_uint* pRegions, eg_uint nMaxRegions, eg_uint m_nLastKnownR)const;
	eg_bool IsPointInRegion(const eg_vec4& vPoint, eg_uint nRegion)const;

	const eg_uint GetLightCount()const;
	const eg_uint GetRegionCount()const;
	const eg_uint GetGeoBlockCount()const{return m_pH->nGeoBlocksCount;}
	const eg_uint GetPortalCount(const eg_uint nRegion)const;
	const egPortal* GetPortal(const eg_uint nRegion, eg_uint nPortal)const;
	const egRegion* GetRegion( eg_uint nRegion ) const;

	eg_uint        GetTagCount()const;
	eg_cpstr       GetTagType(eg_uint Id)const;
	const eg_vec4& GetTagPos(eg_uint Id)const;
	eg_cpstr       GetTagAtts(eg_uint Id)const;

	eg_cpstr       GetName()const{ if(!IsLoaded())return (""); return &m_pStrings[m_H.nStrOfsName]; };

	void GetMapBounds(eg_aabb* paabbOut)const;

	const eg_uint  GetPortalCount()const;
	const egPortal* GetPortal(const eg_uint nPort)const;

	void            GetLight(egLight* pLight, eg_uint nLight)const;
	const egLight*   GetLight(eg_uint nLight)const;
	const eg_color* GetRegAmb(eg_uint nRegion)const;

	eg_uint GetClosestLights(const eg_vec4& Pos , eg_uint nRegion , eg_uint anLights[] , eg_real fDistancesSq[] , const eg_uint nMaxLights );
	const eg_vec4& GetSpawnLocation()const;
	eg_cpstr GetRegionName(eg_uint nID)const;

	//////////////////
	// Graph interface
	//////////////////
	eg_uint  FindGraph(eg_string_crc Id )const;
	eg_uint  FindGraph(eg_cpstr strGraph)const;
	eg_uint  GetGraphCount()const;
	eg_string_crc Graph_GetCrcId( eg_uint IdGraph )const;
	eg_string_crc Graph_GetType( eg_uint IdGraph )const;
	eg_uint  Graph_GetNumVerts(eg_uint IdGraph)const;
	eg_uint  Graph_GetVertex(eg_uint IdGraph, eg_uint VertIndex)const;
	eg_uint* Graph_GetVertexById( eg_uint IdVertex , eg_uint* NumEdgesOut , eg_vec4* PosOut )const;
	void     Graph_Vertex_GetInfo(eg_uint IdVertex, eg_vec4* OutPos, eg_uint* OutEdgeCount, eg_uint* OutEdges , eg_uint MaxOut)const;

protected:

	void SetPointers(const egHeader& Header);

private:

	static eg_uint GetClosestLights_InsertLightIfCloser(eg_uint nLight, const eg_real fDist, eg_uint anLights[], eg_real afDists[], eg_uint nLights, eg_uint nMaxLights);
};
