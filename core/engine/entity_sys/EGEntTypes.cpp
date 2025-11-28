// (c) 2017 Beem Media

#include "EGEntTypes.h"

const eg_region_id eg_region_id::NoRegion( nullptr , 0 );
const eg_region_id eg_region_id::AllRegions( nullptr, 0xFFFFFFFF );

eg_uint eg_region_id::GetListId() const
{
	eg_string_crc RegionCrc = eg_string_crc::HashData( &Map , sizeof(Map) );
	eg_uint32 RegionCrc32 = RegionCrc.ToUint32();
	eg_uint Out = (RegionCrc32 << 16) + MapRegion;
	return Out;
}
