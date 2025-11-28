// (c) 2017 Beem Media

#pragma once

#include "EGList.h"
#include "EGLocText.h"

class EGEnt;

class EGEntListHook : public IListable
{
public:

	EGEnt*const Owner;

public:

	EGEntListHook( EGEnt* InOwner ): Owner( InOwner ){ }
};

typedef EGList<EGEntListHook> EGEntList;

static const eg_uint EG_ENT_LIST_UPDATE_ON_ID  = 1;
static const eg_uint EG_ENT_LIST_UPDATE_OFF_ID = 2;

enum class eg_ent_role
{
	Unknown,
	Authority,
	Replicated,
	EditorPreview,
};

enum class eg_ent_world_role
{
	Unknown,
	Server,
	Client,
	EditorPreview,
};

enum class eg_spawn_reason
{
	Unknown,
	GameLoad,
	Spawned,
	Replicated,
	MapEntity,
};

struct egTextOverride
{
	eg_string_crc NodeId;
	eg_loc_text   Text;

	egTextOverride()
	: NodeId( CT_Clear )
	, Text( CT_Clear )
	{

	}
};

struct egBoneOverride
{
	eg_string_crc BoneId;
	eg_transform  Transform;
};

struct egEntCreateParms
{
	eg_transform    Pose;
	eg_spawn_reason Reason;
	eg_ent_role     Role;
	eg_string_crc   EntDefId;
	eg_d_string     InitString;
	const void*     SerializedMem;
	eg_size_t       SerializedMemSize;

	egEntCreateParms() = delete;
	egEntCreateParms( eg_ctor_t Ct )
	: Pose( Ct )
	, EntDefId( Ct )
	, InitString( Ct )
	, Reason(eg_spawn_reason::Unknown)
	, Role( eg_ent_role::Unknown )
	, SerializedMem(nullptr)
	, SerializedMemSize(0)
	{ 
		assert( Ct == CT_Clear ); 
	}
};

struct eg_region_id
{
public:

	static const eg_region_id NoRegion;
	static const eg_region_id AllRegions;

private:

	class EGGameMap* Map = nullptr;
	eg_uint          MapRegion = 0;
	
public:

	eg_region_id() = default;
	eg_region_id( class EGGameMap* InMap , eg_uint InMapRegion ): Map( InMap ) , MapRegion( InMapRegion ) { }

	eg_uint GetListId() const;

	eg_bool operator == ( const eg_region_id& Rhs ) const { return Map == Rhs.Map && MapRegion == Rhs.MapRegion; }
	eg_bool operator != ( const eg_region_id& Rhs ) const { return !(*this == Rhs); }
	eg_bool operator < ( const eg_region_id& Rhs ) const
	{
		if( Map != Rhs.Map )
		{
			return reinterpret_cast<eg_uintptr_t>(Map) < reinterpret_cast<eg_uintptr_t>(Rhs.Map);
		}

		return MapRegion < Rhs.MapRegion;
	}
};

struct egEntLight
{
	eg_vec3         Pos;
	eg_real         RangeSq;
	eg_color32      Color;
	eg_real         DistSq;
	mutable eg_real Weight; 
};

typedef EGFixedArray<egEntLight,4> EGEntCloseLights;