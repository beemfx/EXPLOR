// (c) Beem Media - Generated Code Definition for "EGGcxMapMetaData.h"

#if defined( EGRFL_INCLUDE_SOURCE_HEADER )
#include "EGGcxMapMetaData.h"
#include "EGGcxMapMetaData.reflection.h"
#endif // defined( EGRFL_INCLUDE_SOURCE_HEADER )

EGRFL_IMPL_STRUCT_BEGIN( egGcxMapMetaInfo )
	EGRFL_IMPL_STRUCT_ITEM( egGcxMapMetaInfo , eg_asset_path , MapPath )
	EGRFL_IMPL_STRUCT_ITEM( egGcxMapMetaInfo , eg_vec2 , TileSize )
	EGRFL_IMPL_STRUCT_ITEM( egGcxMapMetaInfo , eg_vec2 , MapSize )
	EGRFL_IMPL_STRUCT_ITEM( egGcxMapMetaInfo , eg_vec2 , LowerLeftOrigin )
	EGRFL_IMPL_STRUCT_ITEM( egGcxMapMetaInfo , eg_asset_path , MiniMap )
EGRFL_IMPL_STRUCT_END( egGcxMapMetaInfo )

EGRFL_IMPL_CHILD_STRUCT_BEGIN( EGGcxMapMetaData , EGDataAsset )
	EGRFL_IMPL_STRUCT_ITEM( EGGcxMapMetaData , egGcxMapMetaInfo , MetaInfo )
EGRFL_IMPL_CHILD_STRUCT_END( EGGcxMapMetaData , EGDataAsset )

