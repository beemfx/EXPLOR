//
// Grid Cartographer Types
// (c) 2016 Beem Media
//

#pragma once

#include "EGGcxEnums.h"

typedef eg_char gcx_name[64];
typedef eg_char gcx_path[256];

typedef std::function<eg_bool(eg_cpstr FullFilename,const class EGFileData& FileData)> EGGcxSaveCb;
typedef std::function<eg_string_big(eg_uint Index,const struct gcxBuildMaterialData& Material)> EGGcxWriteMtrlCb;
typedef std::function<void(eg_cpstr PropsFilename,const struct gcxExtraMapProps& Props)> EGGcxProcessPropsCb;

struct gcxNote
{
	eg_d_string NoteBody;
	eg_ivec2    TileCoord;
};

struct gcxAutoTransition
{
	eg_bool       bHasAutoTransition = false;
	gcx_direction Directon = gcx_direction::UNKNOWN;
};

struct gcxTile
{
	gcx_edge_t        Right:16;
	gcx_edge_t        Bottom:16;
	eg_uint           RightColor:16;
	eg_uint           BottomColor:16;
	gcx_terrain_t     Terrain:16;
	gcx_marker_t      MarkerType:16;
	gcx_dark_effect_t DarkEffect:16;
	gcx_ceiling_t     CeilingType:16;
	eg_uint           CustomTileIndex:15;
	eg_bool           bHasCustomTile:1;
	eg_bool           bIsSafe:1;

	eg_string_crc     ExRightExtraCrc;
	eg_string_crc     ExBottomExtraCrc;
	eg_int            ExRightExtraInt;
	eg_int            ExBottomExtraInt;
	gcxAutoTransition ExAutoTransition;

	gcxTile() = default;

	gcxTile( eg_ctor_t Ct )
	{
		if( Ct == CT_Default || Ct == CT_Clear )
		{
			Clear();
		}
	}

	void Clear()
	{
		Right   = gcx_edge_t::NONE;
		Bottom  = gcx_edge_t::NONE;
		RightColor = 0;
		BottomColor = 0;
		Terrain = gcx_terrain_t::DEFAULT;
		MarkerType = gcx_marker_t::NONE;
		DarkEffect = gcx_dark_effect_t::NONE;
		CeilingType = gcx_ceiling_t::NONE;
		CustomTileIndex = 0;
		bHasCustomTile = false;
		bIsSafe = false;
		ExRightExtraCrc = CT_Clear;
		ExBottomExtraCrc = CT_Clear;
		ExRightExtraInt = 0;
		ExBottomExtraInt = 0;
		ExAutoTransition = gcxAutoTransition();
	}
};

struct gcxCustomTile
{
	eg_uint       Index;
	gcx_terrain_t TerrainType;
	gcx_name      File;
};
