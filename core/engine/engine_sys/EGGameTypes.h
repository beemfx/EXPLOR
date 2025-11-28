// (c) 2017 Beem Media

#pragma once

struct egRaycastInfo
{
	eg_uint   nGroup;
	eg_ent_id nEntID;
	eg_vec4   vFrom; //Keeps track of where the trace actually started from.
	eg_vec4   vHit;
};

struct egMapTag
{
	eg_transform  Pose;
	eg_string_crc Type;
	eg_cpstr      Attrs; //Pointer to string within map data
};
