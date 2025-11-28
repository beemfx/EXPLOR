// (c) 2018 Beem Media

#pragma once

#include "EGDataAssetBase.h"
#include "EGGcxEnums.h"
#include "EGCrcDb.h"
#include "EGAssetPath.h"
#include "EGReflection.h"
#include "EGEngineConfig.h"

egreflect struct gcxBuildTileData
{
	egprop eg_d_string   TerrainType      = "DEFAULT";
	egprop eg_real       TopOffset        = 0.f;       // How far from the floor's base is the top of the ground
	egprop eg_real       BottomOffset     = -.5f;      // How far from the floor's base is the bottom of the ground
	egprop eg_real       NavGraphOffset   = 1.f;
	egprop eg_string_crc GroundMaterial   = EGCrcDb::StringToCrc("DEFAULT");
	egprop eg_string_crc CeilingMaterial  = EGCrcDb::StringToCrc("DEFAULT");
	egprop eg_real       CeilingThickness = .5f;       //How far the ceiling extends above the top of the wall.
	egprop eg_real       CeilingHeight    = 4.f;
	egprop eg_bool       bCanWalkOn       = true;
	egprop eg_bool       bHasGround       = true;

	gcx_terrain_t TerrainTypeEnum = gcx_terrain_t::DEFAULT;
};

egreflect struct gcxDoorData
{
	egprop eg_real       Height                  = 3.f;
	egprop eg_real       Width                   = 2.f;
	egprop eg_real       TorchDistanceFromGround = 3.25f;
	egprop eg_asset_path EntityDefinition        = eg_asset_path( eg_asset_path_special_t::GameEntities , "ENT_Door_Default" );
};

egreflect struct gcxBuildWallData
{
	// Key:
	egprop eg_int        PaletteIndex                = 0;

	// Basic info
	egprop eg_string_crc WallMaterial                = EGCrcDb::StringToCrc("DEFAULT");
	egprop eg_string_crc PillarMaterial              = EGCrcDb::StringToCrc("DEFAULT");
	egprop eg_real       Height                      = 4.f;
	egprop eg_real       WallThickness               = .3f;
	egprop eg_real       PillarThickness             = .3f;
	egprop eg_color32    MapWallColor                = eg_color32(252,252,84);

	// Door Info
	egprop gcxDoorData   StandardDoorData;
	egprop gcxDoorData   SecretDoorData;

	// Torch Info
	egprop eg_real       TorchRange                   = 4.4f;
	egprop eg_color32    TorchColor                   = eg_color32(eg_color(1.f,.2274f,0.f,1.f));
	egprop eg_real       TorchDistanceFromWall        = .1f;
	egprop eg_real       TorchDistanceFromGround       = 2.5f;
	egprop eg_asset_path TorchEntityDefinition        = eg_asset_path( eg_asset_path_special_t::GameEntities , "ENT_Torch_OrangeFlame" );
	egprop eg_bool       bDoorsHaveTorches            = false;

	eg_real GetTorchDistanceFromGroundForDoor( gcx_edge_t EdgeType ) const;
};

egreflect struct gcxBuildMaterialData
{
	egprop eg_string_crc MaterialId          = EGCrcDb::StringToCrc("DEFAULT");
	egprop eg_asset_path Texture0Path        = eg_asset_path( EXT_TEX , "/egdata/textures/default_white" );
	egprop eg_asset_path Texture1Path        = eg_asset_path( EXT_TEX , "/egdata/textures/default_normal" );
	egprop eg_asset_path Texture2Path        = eg_asset_path( EXT_TEX , "/egdata/textures/default_black" );
	egprop eg_asset_path Texture3Path        = eg_asset_path( EXT_TEX , "" );
	egprop eg_asset_path ShaderPath          = eg_asset_path( "evs5"  , "/game/shaders/ex_map_basic" );
	egprop eg_vec2       TextureUVScale      = eg_vec2(.5f,.5f);
	egprop eg_color32    AutomapGroundColor1 = eg_color32(eg_color(0.f,0.f,0.f,1.f));
	egprop eg_color32    AutomapGroundColor2 = eg_color32(eg_color(1.f,1.f,1.f,1.f));
};

egreflect class EGGcxBuildData : public egprop EGDataAsset
{
	EG_CLASS_BODY( EGGcxBuildData , EGDataAsset )
	EG_FRIEND_RFL( EGGcxBuildData )

private:

	static EGGcxBuildData* s_Inst;

public:

	static void Load( eg_cpstr16 Filename );
	static void Unload();
	static const EGGcxBuildData& Get() { return *s_Inst; }

private:

	egprop eg_real                       m_WallLength = 6;
	egprop eg_vec2                       m_RegionHeightDims = eg_vec2(-.5f,15.f);
	egprop eg_bool                       m_bAllTilesHaveCeilings = false;
	egprop eg_bool                       m_bAnyTileWithMarkerIsSafe = true;
	egprop eg_bool                       m_bTilesWithMonstersAreSafe = false;
	egprop EGArray<gcxBuildTileData>     m_TileInfos;
	egprop EGArray<gcxBuildWallData>     m_WallInfos;
	egprop EGArray<gcxBuildMaterialData> m_MaterialInfos;

	const gcxBuildTileData     m_DefaultTileData;
	const gcxBuildWallData     m_DefaultWallData;
	const gcxBuildMaterialData m_DefaultMaterialData;

public:

	virtual void PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor ) override;

	const gcxBuildMaterialData& GetInfoForMaterial( eg_string_crc MaterialId ) const;
	const gcxBuildTileData& GetInfoForTerrain( gcx_terrain_t TerrainType ) const;
	const gcxBuildWallData& GetInfoForWall( eg_uint PaletteIndex ) const;
	void GetAllMaterials( EGArray<gcxBuildMaterialData>& Out ) const;
	eg_real GetWallLength() const { return m_WallLength; }
	eg_vec2 GetRegionHeightDims() const { return m_RegionHeightDims; }
	eg_bool GetAllTilesHaveCeilings() const { return m_bAllTilesHaveCeilings; }
	eg_bool GetAnyTileWithMarkerIsSafe () const { return m_bAnyTileWithMarkerIsSafe; }
	eg_bool GetTilesWithMonstersAreSafe() const { return m_bTilesWithMonstersAreSafe; }

	static gcx_terrain_t StringToTerrainType( eg_cpstr String );
};
