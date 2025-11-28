// (c) 2017 Beem Media

#pragma once

#include "EGEntTypes.h"

class EGEntWorldRegion : public EGObject
{
	EG_CLASS_BODY( EGEntWorldRegion , EGObject )

private:

	eg_region_id          m_Id = eg_region_id::NoRegion;
	EGGameMap*            m_Map = nullptr;
	eg_uint               m_MapRegion = 0;
	eg_aabb               m_Bounds = eg_aabb(  CT_Default );
	eg_color              m_AmbientLight = eg_color(0.f,0.f,0.f,1.f);
	EGEntList             m_EntList;
	EGArray<eg_region_id> m_AdjRegions;
	eg_string_small       m_Name = "";

public:

	void InitRegion( EGGameMap* GameMap , eg_uint MapRegion );
	void InitRegion( const eg_region_id& RegionId );

	EGEntList& GetEntList() { return m_EntList; }
	const EGEntList& GetEntList() const { return m_EntList; }

	const eg_region_id& GetId() const { return m_Id; }
	EGGameMap* GetMap() { return m_Map; }
	const EGGameMap* GetMap() const { return m_Map; }
	eg_uint GetRegionIndex() const { return m_MapRegion; }
	eg_cpstr GetRegionName() const { return m_Name; }
	const eg_aabb& GetBounds() const { return m_Bounds; }
	const EGArray<eg_region_id>& GetAdjacentRegions() const { return m_AdjRegions; }
	void GetCloseLights( const eg_vec4& Pos , EGEntCloseLights& Out ) const;
	const eg_color& GetAmbientLight() const { return m_AmbientLight; }
};
