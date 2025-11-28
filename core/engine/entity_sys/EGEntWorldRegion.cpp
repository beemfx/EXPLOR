// (c) 2017 Beem Media

#include "EGEntWorldRegion.h"
#include "EGGameMap.h"

EG_CLASS_DECL( EGEntWorldRegion )

void EGEntWorldRegion::InitRegion( EGGameMap* GameMap, eg_uint MapRegion )
{
	m_Id = eg_region_id( GameMap , MapRegion );
	m_EntList.Init( m_Id.GetListId() );
	m_Map = GameMap;
	m_MapRegion = MapRegion;

	if( GameMap )
	{
		// Find all adjacent regions.
		const eg_uint NumPortals = GameMap->GetPortalCount( MapRegion );
		for( eg_uint i=0; i<NumPortals; i++ )
		{
			const EGGameMap::egPortal* Portal = GameMap->GetPortal( MapRegion , i+1 );
			if( Portal )
			{
				eg_region_id AdjRegion( GameMap , Portal->nTo );
				m_AdjRegions.Append( AdjRegion );
			}
		}

		if( MapRegion > 0 )
		{
			const EGGameMap::egRegion* pRegion = GameMap->GetRegion( MapRegion );
			if( pRegion )
			{
				m_Bounds = pRegion->aabb;
				m_Name = GameMap->GetRegionName( MapRegion );
			}
		}

		m_AmbientLight = *GameMap->GetRegAmb( MapRegion );
	}
}

void EGEntWorldRegion::InitRegion( const eg_region_id& RegionId )
{
	assert( RegionId == eg_region_id::NoRegion || RegionId == eg_region_id::AllRegions );
	m_Id = RegionId;
	m_EntList.Init( m_Id.GetListId() );
	m_Map = nullptr;
	m_MapRegion = 0;
	if( RegionId == eg_region_id::NoRegion )
	{
		m_Name = "No Region";
	}
	else if( RegionId == eg_region_id::AllRegions )
	{
		m_Name = "All Regions";
	}
}

void EGEntWorldRegion::GetCloseLights( const eg_vec4& Pos, EGEntCloseLights& Out ) const
{
	if( m_Map )
	{
		assert( m_Map->IsLoaded() ); // Should have never asked for this if the map wasn't loaded (regions wouldn't have been created yet).
		eg_uint LightIndex[RENDERER_MAX_LIGHTS];
		eg_real LightDistSq[RENDERER_MAX_LIGHTS];
		eg_uint NumLights = m_Map->GetClosestLights( Pos , m_MapRegion , LightIndex , LightDistSq , RENDERER_MAX_LIGHTS );
		for( eg_uint i=0; i<NumLights; i++ )
		{
			const EGGameMap::egLight* Light = m_Map->GetLight( LightIndex[i] );
			if( Light && !Out.IsFull() )
			{
				egEntLight NewLight;
				NewLight.Pos = Light->Pos.ToVec3();
				NewLight.DistSq = LightDistSq[i];
				NewLight.Color = eg_color32(Light->Color);
				NewLight.RangeSq = Light->fRangeSq;
				NewLight.Weight = 0.f;
				Out.Append( NewLight );
			}
			else
			{
				assert( false );
			}
		}
	}
}
