// (c) 2018 Beem Media

#pragma once

#include "EGItemMap.h"

class EGVisualComponent;
class EGEnt;
class EGGameMap;
class EGEntWorld;
class EGTerrainMesh;
class EGMeshShadowComponent;
class EGMeshComponent;
class EGSkelMeshComponent;
class EGDisplayList;
class EGCamera2;

struct egWorldSceneGraphComponent
{
	const EGVisualComponent* Component     = nullptr;
	const EGEnt*             EntOwner      = nullptr;
	eg_string_crc            RenderFilter  = CT_Clear;
	eg_transform             WorldPose     = CT_Default;
	eg_bool                  bIsReflective = false;
};

struct egWorldSceneGraphShadow
{
	const EGMeshShadowComponent* Component     = nullptr;
	const EGEnt*                 EntOwner      = nullptr;
	const EGMeshComponent*       OwnerMesh     = nullptr;
	const EGSkelMeshComponent*   OwnerSkelMesh = nullptr;
	eg_transform                 WorldPose     = CT_Default;
};

struct egWorldSceneGraphMapRegion
{
	const EGGameMap* GameMap     = nullptr;
	eg_uint          RegionIndex = 0;
};

struct egWorldSceneGraphTerrain
{
	const EGTerrainMesh* TerrainMesh = nullptr;
	eg_transform         WorldPose   = CT_Default;
};

class EGWorldSceneGraph : public EGObject
{
	EG_CLASS_BODY( EGWorldSceneGraph , EGObject )

public:

	typedef std::function<void()> EGPreDrawFn;

private:

	typedef EGArray<egWorldSceneGraphComponent> EGCompList;
	typedef EGArray<egWorldSceneGraphShadow> EGShadowList;
	typedef EGArray<egWorldSceneGraphMapRegion> EGMapRegionList;
	typedef EGArray<egWorldSceneGraphTerrain> EGMapTerrainList;
	typedef EGItemMap<eg_string_crc,EGCompList> EGCompMap;

private:

	EGCompMap        m_CompMap = EGCompList();
	EGCompList       m_ReflectiveCompList;
	EGShadowList     m_ShadowList;
	EGMapRegionList  m_MapRegionsList;
	EGMapTerrainList m_TerrainList;
	eg_transform     m_CameraPose = CT_Default;
	eg_bool          m_bDebugWireFrame = false;
	eg_uint          m_MaxShadows = 4;
	eg_uint          m_MaxReflectiveEnts = 1;

public:

	void ResetGraph( const EGCamera2& Camera );

	void AddItem( const egWorldSceneGraphComponent& NewComp );
	void AddItem( const egWorldSceneGraphShadow& NewComp );
	void AddItem( const egWorldSceneGraphMapRegion& NewMapRegion );
	void AddItem( const egWorldSceneGraphTerrain& NewTerrain );

	void DrawMapRegions( EGPreDrawFn PreDrawCb = nullptr ) const;
	void DrawTerrains( EGPreDrawFn PreDrawCb = nullptr ) const;
	void DrawComponents( const eg_string_crc& RenderFilter , EGPreDrawFn PreDrawCb = nullptr ) const;
	void DrawShadows( eg_real FarPln , EGPreDrawFn PreDrawCb = nullptr ) const;
	const EGEnt* GetEntForReflectiveDraw( eg_uint Index ) const;

private:

	void DrawVolumeShadow( EGDisplayList* DisplayList , const EGEnt* Ent , const EGMeshShadowComponent* ShadowComp , eg_real FarPlaneDistSq , const eg_transform& FullPose ) const;

public:

	static void SetLightingForEnt( EGDisplayList* DisplayList , const EGEnt* Ent );
};
