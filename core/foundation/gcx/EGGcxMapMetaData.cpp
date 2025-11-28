// (c) 2019 Beem Media

#include "EGGcxMapMetaData.h"
#include "EGCrcDb.h"

EG_CLASS_DECL( EGGcxMapMetaData )

void EGGcxMapMetaData::SetTo( const gcxExtraMapProps& rhs )
{
	MetaInfo.MapPath.Path = *rhs.Level;
	MetaInfo.TileSize = rhs.TileSize;
	MetaInfo.MapSize = rhs.MapSize;
	MetaInfo.LowerLeftOrigin = rhs.LowerLeftOrigin;
	MetaInfo.MiniMap.Path = *rhs.MiniMap;
}
