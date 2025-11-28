// (c) 2017 Beem Media

#pragma once

struct egNet_EW_SpawnEnt
{
	eg_ent_id   EntId; //ID of the entity.
	egNet_Crc   DefId; //Entity template, that entity is based upon.
	egNet_Trans Pose;
};

struct egNet_EW_DestroyEnt
{
	eg_ent_id EntId;
};

struct egNet_EW_UpdateEnt
{
	eg_ent_id    EntId;
	egNet_Trans  Pose;
	egNet_Bounds Bounds;
	eg_uint      TimeMs; //Time of the update.

	static const eg_uint TIME_IS_NO_SMOOTH = 0;
};

struct egNet_EW_RunEntEvent
{
	eg_ent_id EntId;
	egNet_Crc EventId;
};

struct egNet_EW_ReplicateEntData
{
	eg_ent_id EntId;   //ID of entity for this data.
	eg_uint64 Offset;
	eg_uint8  Size;
	eg_byte   Data[120]; //Could go bigger if we need to...
};

static_assert( sizeof(egNet_EW_ReplicateEntData) < 256 , "egNet_EntRepData should be small enough that it's size fits in a single byte." );

struct egNet_EW_ReplicateGameData
{
	eg_uint64 Offset;
	eg_uint8  Size;
	eg_byte   Data[120]; //Could go bigger if we need to...
};

static_assert( sizeof(egNet_EW_ReplicateGameData) < 256 , "egNet_DataChunk should be small enough that it's size fits in a single byte." );

struct egNet_EW_RequestEntSpawnInfo
{
	eg_ent_id EntId;
};

struct egNet_EW_SpawnFile
{
	egNet_Filename Filename;
	egNet_Trans    Pose;
};

static_assert( sizeof(egNet_EW_SpawnFile) < 256 , "egNet_EW_SpawnFile should be small enough that it's size fits in a single byte." );
