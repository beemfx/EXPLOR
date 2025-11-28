// (c) 2018 Beem Media

#pragma once

#include "EGGcxTypes.h"
#include "EGGcxFloor.h"
#include "EGGcxBuildTypes.h"

struct gcxBuildWallData;

class EGGcxRegion
{
protected:

	static const eg_uint MAP_REGION_TILE_DIM = 16;

protected:

	eg_d_string               m_Name;
	eg_int                    m_LowestFloor;
	gcx_grid_t                m_GridShape;
	gcx_tile_origin           m_Origin;
	eg_int                    m_MapMinX;
	eg_int                    m_MapMinY;
	eg_int                    m_MapMaxX;
	eg_int                    m_MapMaxY;
	eg_int                    m_MapWidth;
	eg_int                    m_MapHeight;
	eg_int                    m_RegionWidth;
	eg_int                    m_RegionHeight;

	EGArray<gcxFloor>         m_Floors;

	EGArray<gcxLight>         m_Lights;
	EGArray<gcxTag>           m_Tags;
	EGArray<gcxTagEntity>     m_MapEnts;
	EGArray<gcxGraphVertex>   m_NavGraph;
	EGArray<gcxWorldGeometry> m_WorldGeometry;

	// For building
	EGArray<gcxEMapSegment>   m_EMapSegments;
	EGArray<gcxEMapRegion>    m_EMapRegions;
	EGArray<egv_vert_mesh>    m_EMapVertexes;
	EGArray<eg_int>           m_EMapVertexesHullIdxs;
	EGArray<gcxBrush>         m_EMapGeoBrushes;
	EGArray<gcxEMapPortal>    m_EMapPortals;
	EGArray<gcxLight>         m_EMapLights;

	// For XML Reading:
	eg_int                    m_CurFloorIndex;
	eg_int                    m_CurRowIndex;
	eg_int                    m_CurColIndex;

	eg_d_string m_SourceDir;

private:

	void Clear();
	void Init( eg_uint NewFloorsCount , eg_int NewLowestFloor , gcx_grid_t NewGridShape );
	void SetName( eg_cpstr NewName ) { m_Name = NewName; }
	void ComputeMapRegions();
	eg_uint FloorIndexToArrayIndex( eg_int FloorIndex ) { return FloorIndex - m_LowestFloor; }
	void AddMapEntity( eg_cpstr EntDef, eg_cpstr Extra, const eg_vec4& Pos, const eg_vec4& Rot );
	void AddNoteTag( eg_int x, eg_int y, eg_cpstr Type, eg_cpstr Name );
	void AddFunctionStringTag( eg_int FloorArrayIndex , eg_int x , eg_int y , const eg_string_big& CallStr );
	void AddFunctionCallTag( eg_int Floor , eg_int x, eg_int y , const struct egParseFuncInfoAsEgStrings& CallInfo );
	eg_uint GetEMapRegionForTilePos( eg_int TileX, eg_int TileY ) const;
	void GetTileBoundsForEMapRegion( eg_uint RegionIndex , eg_ivec2& MinTileOut , eg_ivec2& MaxTileOut );
	eg_uint EMapRegionCoordToIndex( eg_uint TileRegionX, eg_uint TileRegionY ) const;
	eg_ivec2 EMapRegionIndexToCoord( eg_uint Region ) const;
	eg_uint GetEMapRegionCount() const;
	void AddWorldBox( eg_int TileX, eg_int TileY, const eg_aabb& Bounds, eg_string_crc Material );
	void WriteBox( const eg_aabb& Bounds , eg_string_crc Material );
	void ComputeTxCoords( egv_vert_mesh* V, eg_size_t Count , const eg_vec2& TxAdjust );
	void WriteDoorFrame( eg_int TileX, eg_int TileY, const eg_vec4& Min, const eg_vec4& Max, eg_string_crc MaterialType, eg_real DoorWidth, eg_real DoorHeight , eg_real BottomAdjustment );
	void WriteWall( eg_int TileX, eg_int TileY, gcx_edge_t Type , const eg_vec4& Min , const eg_vec4& Max , const gcxBuildWallData* WallData , eg_real BottomAdjustment , eg_string_crc Material );
public:
	static eg_bool HasSouthOrEastSideTorch( gcx_edge_t Type , eg_bool bDoorsHaveTorches );
	static eg_bool HasNorthOrWestSideTorch( gcx_edge_t Type , eg_bool bDoorsHaveTorches );
	static eg_bool IsDoor( gcx_edge_t Type );
	static eg_bool IsHiddenDoor( gcx_edge_t Type );
private:
	eg_int CreateFace( eg_real Width, eg_real Height, egv_vert_mesh* Out, eg_int OutSize );
	void GenerateVertexLighting_HandleVertex( egv_vert_mesh& v , eg_int VertexHullIdx );
	void GenerateVertexLighting();
	void GenerateAutoMap( eg_cpstr Filename , EGGcxSaveCb& SaveCb );
	static eg_bool CanPassThrough( gcx_edge_t TileType );
	static eg_bool CanPassThrough( gcx_terrain_t TerrainType );
	void GenerateGraphs();
	static void FORMAT_VEC4_AS_VEC3( eg_cpstr8 Fmt, const eg_vec4& v, eg_bool bBase64, eg_string_base& strLine );
	void GenerateWorldGeometry();
	eg_bool SaveAsXmlEmap( eg_cpstr OutDir , EGFileData& Out , EGGcxSaveCb& SaveCb , EGGcxWriteMtrlCb& WriteMaterialCb , gcxExtraMapProps& ExtraPropsOut );

public:

	void SetSourceDir( eg_cpstr SourceDirIn ) { m_SourceDir = SourceDirIn; }
	void HandleXmlTag( const eg_string_base& Tag , const EGXmlAttrGetter& Atts );
	void HandleXmlData( const eg_string_base& Tag, eg_cpstr Data, eg_uint Len );
	void HandleSetDoorKey( const egParseFuncInfoAsEgStrings& CallInfo , eg_int FloorIndex , const gcxNote& RawNote );
	void HandleNoteFunctionCall( const egParseFuncInfoAsEgStrings& CallInfo , eg_int FloorIndex , const gcxNote& RawNote );
	void HandlePropertyFunctionCall( const egParseFuncInfoAsEgStrings& CallInfo , eg_int FloorIndex , const gcxNote& RawNote );
	void ProcessNotes();
	eg_d_string GetNoteAt( eg_int FloorIndex , eg_int x , eg_int y ) const;
	void ProcessProperties();
	void ProcessMarkers();
	eg_bool SaveToDir( eg_cpstr OutDir , EGGcxSaveCb& SaveCb , EGGcxWriteMtrlCb& WriteMaterialCb , gcxExtraMapProps& ExtraPropsOut , eg_bool bCompress );
	void AddTorch( eg_uint Region , const eg_vec4& TorchPos , const eg_real RotAboutY , const gcxBuildWallData& ThisWallInfo );
	void SetCustomTiles( const EGArray<gcxCustomTile>& CustomTiles );
	const eg_d_string& GetName() const { return m_Name; }

	static eg_string_big GetCleanName( eg_cpstr Data , eg_size_t Len );
};
