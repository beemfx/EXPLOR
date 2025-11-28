// (c) 2013 Beem Software

#pragma once

#include "EGRendererTypes.h"
#include "EGGameTerrain2.h"
#include "EGItemMap.h"

class EGVDynamicBuffer;

struct egTerrainMeshSegDims
{
private:

	const eg_uint SegDim;

public:
	
	egTerrainMeshSegDims( eg_uint InSegDim ): SegDim( InSegDim ) { }

	eg_uint GetDim() const { return SegDim; }
	eg_uint GetNumVerts() const { return EGMath_Square( SegDim + 1 ); }
	eg_uint GetNumTris() const { return 2 * EGMath_Square( SegDim ); }
};

struct egTerrainMeshSegment
{
	egv_vert_terrain* Verts   = nullptr;
	egv_index*        Indexes = nullptr;
	eg_uint           FirstVertIdx = 0;
	eg_uint           FirstIndexIdx = 0;
	eg_bool           bIsHi = false;

	eg_bool IsValid() const { return nullptr != Verts && nullptr != Indexes; }
};

struct egTerrainMeshSegId
{
	eg_ivec2 Loc;

	egTerrainMeshSegId() = default;
	egTerrainMeshSegId( eg_ctor_t Ct ): Loc( Ct ) { }
	egTerrainMeshSegId( const eg_ivec2& rhs ): Loc(rhs) { }

	eg_bool operator < ( const egTerrainMeshSegId& rhs ) const { return GetHashValue() < rhs.GetHashValue(); }
	eg_bool operator == ( const egTerrainMeshSegId& rhs ) const { return Loc == rhs.Loc; }
	eg_bool operator != ( const egTerrainMeshSegId& rhs ) const { return !(*this == rhs); }

	eg_int GetHashValue() const { return (Loc.x << 16) | Loc.y; }
};

typedef EGItemMap<egTerrainMeshSegId,egTerrainMeshSegment> EGTerrainMeshSegMap;
typedef EGArray<egTerrainMeshSegment> EGTerrainMeshSegArray;

class EGTerrainMesh
{
public:
	EGTerrainMesh( EGGameTerrain2* Terrain );

	~EGTerrainMesh(){ Unload(); assert( nullptr == m_MeshBuffer ); }
	void Draw( const eg_transform& WorldPose ) const;
	void UpdateLOD( eg_real DeltaTime , const eg_vec4& CameraPos , const eg_transform& WorldPose );

private:

	EGGameTerrain2*       m_Terrain;
	EGVDynamicBuffer*     m_MeshBuffer = nullptr;
	eg_string_small       m_Filename;
	egv_material          m_mtrl;
	const eg_uint         m_SegDivs;
	egTerrainMeshSegDims  m_SegDimHi;
	egTerrainMeshSegDims  m_SegDimLo;
	const eg_real         m_LODThresholdPct;
	eg_bool               m_IsFinalized:1;
	eg_vec2               m_Dims = CT_Clear;
	eg_vec2               m_Min = CT_Clear;
	eg_vec2               m_LODPos = CT_Clear;
	eg_ivec2              m_LODSeg2X = CT_Clear;
	eg_ivec2              m_LODSegIn = CT_Clear;
	eg_ivec2              m_LODSegDiag = CT_Clear;
	EGTerrainMeshSegMap   m_HiSegs = egTerrainMeshSegment();
	EGTerrainMeshSegMap   m_LoSegs = egTerrainMeshSegment();
	EGTerrainMeshSegArray m_FreeSegs;
	egv_index*            m_SeamInds = nullptr;
	eg_uint               m_NumSeamInds = 0;
	mutable EGArray<egTerrainMeshSegId> m_SegsToChange;

private:

	void FinalizeLoad();
	void Unload();
	void GenerateMesh();
	void GenerateMesh_GenerateSeams();

	eg_bool UpdateLODSegments( eg_bool bIgnoreThreshold );
};