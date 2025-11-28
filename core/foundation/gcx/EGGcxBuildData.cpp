// (c) 2018 Beem Media

#include "EGGcxBuildData.h"
#include "EGGcxRegion.h"

EG_CLASS_DECL( EGGcxBuildData )

EGGcxBuildData* EGGcxBuildData::s_Inst = nullptr;

void EGGcxBuildData::Load( eg_cpstr16 Filename )
{
	assert( s_Inst == nullptr );
	s_Inst = EGCast<EGGcxBuildData>(EGDataAsset::LoadDataAsset( Filename , eg_mem_pool::Default , true , egRflEditor() ));
	Get(); // Will crash if it wasn't loaded.
}

void EGGcxBuildData::Unload()
{
	if( s_Inst )
	{
		EGDeleteObject( s_Inst );
		s_Inst = nullptr;
	}
}

void EGGcxBuildData::PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor )
{
	Super::PostLoad( Filename , bForEditor , RflEditor );

	{
		for( gcxBuildTileData& TileData : m_TileInfos )
		{
			TileData.TerrainTypeEnum = StringToTerrainType( *TileData.TerrainType );
		}
	}
}

const gcxBuildMaterialData& EGGcxBuildData::GetInfoForMaterial( eg_string_crc MaterialId ) const
{
	for( const gcxBuildMaterialData& Material : m_MaterialInfos )
	{
		if( Material.MaterialId == MaterialId )
		{
			return Material;
		}
	}

	return m_DefaultMaterialData;
}

const gcxBuildTileData& EGGcxBuildData::GetInfoForTerrain( gcx_terrain_t TerrainType ) const
{
	for( const gcxBuildTileData& TileData : m_TileInfos )
	{
		if( TileData.TerrainTypeEnum == TerrainType )
		{
			return TileData;
		}
	}

	return m_DefaultTileData;
}

const gcxBuildWallData& EGGcxBuildData::GetInfoForWall( eg_uint PaletteIndex ) const
{
	for( const gcxBuildWallData& WallData : m_WallInfos )
	{
		if( WallData.PaletteIndex == EG_To<eg_int>(PaletteIndex) )
		{
			return WallData;
		}
	}
	return m_DefaultWallData;
}

void EGGcxBuildData::GetAllMaterials( EGArray<gcxBuildMaterialData>& Out ) const
{
	Out.Append( m_MaterialInfos );
}

gcx_terrain_t EGGcxBuildData::StringToTerrainType( eg_cpstr String )
{
	#define DECL_GCT( _name_ , _value_ ) else if( EGString_EqualsI( String , #_name_ ) ){ return gcx_terrain_t::_name_; }
	#define DECL_CUSTOM( _name_ , _file_ )
	if( false )
	{
	}
	#include "EGGcxCustomTiles.items"

	#define DECL_GCT( _name_ , _value_ )
	#define DECL_CUSTOM( _name_ , _file_ ) else if( EGString_EqualsI( String , #_name_ ) ){ return gcx_terrain_t::CUSTOM_##_name_; }
	#include "EGGcxCustomTiles.items"

	EGLogf( eg_log_t::Error, __FUNCTION__ ": %s is not recognized as a terrain type.", String );
	return gcx_terrain_t::DEFAULT;
}

eg_real gcxBuildWallData::GetTorchDistanceFromGroundForDoor( gcx_edge_t EdgeType ) const
{
	if( EGGcxRegion::IsHiddenDoor( EdgeType ) )
	{
		return SecretDoorData.TorchDistanceFromGround;
	}
	return StandardDoorData.TorchDistanceFromGround;
}
