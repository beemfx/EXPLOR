// (c) 2019 Beem Media

#pragma once

#include "EGDataAssetBase.h"
#include "EGGcxBuildTypes.h"
#include "EGReflection.h"
#include "EGAssetPath.h"
#include "EGEngineConfig.h"

egreflect struct egGcxMapMetaInfo
{
	egprop eg_asset_path MapPath         = "emap";
	egprop eg_vec2       TileSize        = eg_vec2( 6.f, 6.f );
	egprop eg_vec2       MapSize         = eg_vec2( 4.f, 4.f );
	egprop eg_vec2       LowerLeftOrigin = eg_vec2( 0.f, 0.f );
	egprop eg_asset_path MiniMap         = EXT_TEX;
};

egreflect class EGGcxMapMetaData : public egprop EGDataAsset
{
	EG_CLASS_BODY( EGGcxMapMetaData , EGDataAsset )
	EG_FRIEND_RFL( EGGcxMapMetaData )

private:

	egprop egGcxMapMetaInfo MetaInfo;

public:

	void SetTo( const gcxExtraMapProps& rhs );
	const egGcxMapMetaInfo& GetMetaInfo() const { return MetaInfo; }
};

