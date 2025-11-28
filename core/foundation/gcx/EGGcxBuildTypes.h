// (c) 2016 Beem Media

#pragma once

#include "EGGcxTypes.h"

struct gcxLight
{
	eg_uint  Region;
	eg_transform Pose;
	eg_vec3  Normal;
	eg_color Color;
	eg_real  Range;

	gcxLight()
	: Region(0xFFFFFFFF)
	{

	}

	gcxLight( eg_uint InRegion , const eg_transform& InPose , const eg_color& InColor , eg_real InRange )
	: Region( InRegion )
	, Pose( InPose )
	, Color( InColor )
	, Range( InRange )
	{
		Normal = (eg_vec4(0.f,0.f,1.f,0.f) * InPose).XYZ_ToVec3();
		Normal.NormalizeThis();
	}
};

struct gcxTag
{
	/*<tag id="2" pos="CtdzQgAAAAB7FD5C"  type="entity" rotation="0 -90 0" entity="door"/>*/
	eg_vec4       Pos;
	eg_vec4       Rot;
	eg_string_big Type;
	eg_uint       ExtraInfoDataSize;
	eg_string_big ExtraInfo;
};

struct gcxTagEntity
{
	eg_vec4   Pos;
	eg_vec4   Rot;
	eg_string_big EntDef;
	eg_string_big ExtraStr;
};

struct gcxGraphVertex
{
	eg_uint Id;
	eg_vec4 Pos;
	eg_uint EdgesCount;
	eg_uint Edges[4]; //Only up to 4 edges (N, E, S, W).

	gcxGraphVertex() = default;

	gcxGraphVertex( eg_ctor_t Ct )
	: Pos( Ct )
	{
		if( Ct == CT_Clear || Ct == CT_Default )
		{
			Id = 0;
			EdgesCount = 0;
		}
	}
};

struct gcxWorldGeometry
{
	enum class gcx_t
	{
		BOX,
	};

	gcx_t         Type;
	eg_aabb       Bounds; // For BOX
	eg_string_crc Material;
	eg_uint       Region;
};

struct gcxEMapSegment
{
	eg_uint    FirstVertex;
	eg_uint    TriangleCount;
	eg_uint    MtrlIndex;

	gcxEMapSegment() = default;

	gcxEMapSegment( eg_ctor_t Ct )
	{
		if( Ct == CT_Default || Ct == CT_Clear )
		{
			FirstVertex = 0;
			TriangleCount = 0;
			MtrlIndex = 0;
		}
	}
};


struct gcxEMapRegion
{
	eg_uint Name;
	eg_uint FirstSegment;
	eg_uint NumSegments;
	eg_uint FirstGeoBrush;
	eg_uint NumGeoBrushes;
	eg_uint FirstPortal;
	eg_uint NumPortals;
	eg_uint FirstLight;
	eg_uint NumLights;
	eg_aabb Bounds;

	gcxEMapRegion() = default;

	gcxEMapRegion( eg_ctor_t Ct )
		: Bounds( Ct )
	{
		if( Ct == CT_Clear || Ct == CT_Default )
		{
			Name = 0;
			FirstSegment = 0;
			NumSegments = 0;
			FirstGeoBrush = 0;
			NumGeoBrushes = 0;
			FirstPortal = 0;
			NumPortals = 0;
			FirstLight = 0;
			NumLights = 0;
		}
	}
};

struct gcxBrush
{
	eg_aabb  Bounds;
	eg_hull6 Hull;
};

struct gcxVertex
{
	egv_vert_mesh Vertex;
	eg_int HullIdx;
};

struct gcxEMapPortal
{
	eg_uint ToRegion;
};

struct gcxExtraMapProps
{
	eg_d_string Name = CT_Clear;
	eg_d_string Level = CT_Clear;
	eg_d_string MiniMap = CT_Clear;
	eg_vec2 TileSize = CT_Clear;
	eg_vec2 LowerLeftOrigin = CT_Clear;
	eg_vec2 MapSize = CT_Clear;
};
