// (c) 2011 Beem Software

#pragma once

#include "EGRemoteEvent.h"
#include "EGInputTypes.h"

DECLARE_ID_STRUCT( egNetCommId );

static const egNetCommId NET_ID_ALL = egNetCommId::RawToId( 0x08000000 );
static const egNetCommId NET_ID_NOT_CONNECTED = INVALID_ID;

enum class eg_net_msg : eg_uint
{
	NET_NONE          ,

	STOC_LOCKSTEP     ,

	CTOS_LOCKSTEP     ,
	CTOS_UDP_CONNECT  ,

	NET_STRCMD        ,
	NET_REMOTE_EVENT  , // Can go to either client or server

	// Entity World Synchronization
	EW_MARK_FirstMsg        ,
	EW_Clear                ,
	EW_BeginFullReplication ,
	EW_EndFullReplication   ,
	EW_SpawnEnt             ,
	EW_DestroyEnt           ,
	EW_UpdateEnt            ,
	EW_RunEntEvent          ,
	EW_ReplicateEntData     ,
	EW_ReplicateGameData    ,
	EW_RequestEntSpawnInfo  ,
	EW_SpawnMap             ,
	EW_DestroyMap           ,
	EW_SpawnTerrain         ,
	EW_DestroyTerrain       ,
	EW_ResetGame            ,
	EW_MARK_LastMsg         ,

	COUNT     ,
};

////////////////////////////
/// Broadcast structures ///
////////////////////////////

struct egNet_Crc
{
	eg_uint32 CrcId32;

	egNet_Crc() = default;
	egNet_Crc( const eg_string_crc& InCrc ){ *this = InCrc; }
	void operator = ( const eg_string_crc& InCrc ) { CrcId32 = InCrc.ToUint32(); }
	operator eg_string_crc() const { eg_string_crc Out( CrcId32 ); return Out; }
};

struct egNet_StrCmd
{
	eg_uint8 Length;
	eg_char8 Cmd[127];
};

static_assert( sizeof(egNet_StrCmd) < 256 , "egNet_StrCmd should be small enough that it's size fits in a single byte." );

struct egNet_RemoteEvent
{
	eg_uint32 EventCrc;
	eg_int64  Int64Parm;
	
	void operator = ( const egRemoteEvent& rhs )
	{
		EventCrc = rhs.Function.ToUint32();
		Int64Parm = rhs.Parms.as_int64();
	}

	egRemoteEvent ToEvent() const
	{
		egRemoteEvent Out;
		Out.Function = eg_string_crc(EventCrc);
		Out.Parms = Int64Parm;
		return Out;
	}
};

static_assert( sizeof(egNet_RemoteEvent) == sizeof(egRemoteEvent) , "egNet_RemoteEvent needs to match egRemoteEvent." );

struct egNet_UdpConnect
{
	eg_uint16 Port;
};

struct egNet_FromAddr
{
	eg_byte RawBytes[16];
};

struct egNet_Trans
{
	eg_byte dummy[sizeof(eg_transform)];

	void operator=( const eg_transform& rhs) { EGMem_Copy( &dummy , &rhs , sizeof(dummy) ); }
	operator eg_transform() const { eg_transform Out; CopyTo( &Out ); return Out; }
	void CopyTo( eg_transform* Out )const { EGMem_Copy( Out , &dummy , sizeof(*Out) ); }
};

struct egNet_Filename
{
	eg_char8 Filename[128];

	void operator=( eg_cpstr rhs ) { EGString_Copy( Filename , rhs , countof(Filename) ); }
	operator eg_cpstr() const { return Filename; }
};

struct egNet_Bounds
{
	eg_real Raw[6];

	void operator=( const eg_aabb& rhs )
	{
		Raw[0] = rhs.Min.x;
		Raw[1] = rhs.Min.y;
		Raw[2] = rhs.Min.z;

		Raw[3] = rhs.Max.x;
		Raw[4] = rhs.Max.y;
		Raw[5] = rhs.Max.z;
	}

	void CopyTo( eg_aabb* Out )const
	{
		Out->Min.x = Raw[0];
		Out->Min.y = Raw[1];
		Out->Min.z = Raw[2];
		Out->Min.w = 1.f;

		Out->Max.x = Raw[3];
		Out->Max.y = Raw[4];
		Out->Max.z = Raw[5];
		Out->Max.w = 1.f;
	}

	operator eg_aabb() const { eg_aabb Out; CopyTo( &Out ); return Out; }
};

static_assert( sizeof(egNet_Trans)==sizeof(eg_transform) , "egNet_Transform not the right size." );

#include "EGNetMsgs_EntWorld.h"

struct SS_Lockstep
{
	eg_uint Frame;
	eg_real EntUpdateRate; //The rate at which updates are currently coming. This is usuall a fixed value, but may change depending on how many clients are connected, so the client must maintain it.
	eg_real InputRate; //The rate at which the server wants the client to submit input (in seconds). Usually a fixed rate, but the server sometimes runs at a slower framerate.
};

//SC_ Struct From Client.

struct SC_Lockstep
{
	eg_uint Frame;
	egLockstepCmds Cmds;
	struct egRayTrace
	{
		eg_real Org[3];
		eg_real Dir[3];
	} RayTrace;
};

//Server to Client structures:
struct egNetMsgData
{	
	struct egHeader
	{
		eg_net_msg     nMsg:8;
		eg_uint     nSize:8;	//Size of data (not including header).
		egNetCommId NetId; //Either the source of the message on the server side, or the intendend recipient on the client.
	};

	struct egFooter
	{
		egNet_FromAddr FromAddr;
	};
	
	egHeader Header;
	
	//Third part is the structure (one of the following should be filled in):
	union
	{
		eg_uint          DataStart;
		egNet_StrCmd     StrCmd;
		egNet_UdpConnect UdpConnect;

		SS_Lockstep       SrvLockstep;
		egNet_RemoteEvent RemoteEvent;
		
		SC_Lockstep        CliLockstep;

		egNet_EW_SpawnEnt             EW_SpawnEnt;
		egNet_EW_DestroyEnt           EW_DestroyEnt;
		egNet_EW_UpdateEnt            EW_UpdateEnt;
		egNet_EW_RunEntEvent          EW_RunEntEvent;
		egNet_EW_ReplicateEntData     EW_ReplicateEntData;
		egNet_EW_ReplicateGameData    EW_ReplicateGameData;
		egNet_EW_RequestEntSpawnInfo  EW_RequestEntSpawnInfo;
		egNet_EW_SpawnFile            EW_SpawnMap;
		egNet_EW_SpawnFile            EW_DestroyMap;
		egNet_EW_SpawnFile            EW_SpawnTerrain;
		egNet_EW_SpawnFile            EW_DestroyTerrain;
	};

	egFooter Footer;
};

static_assert( sizeof(egNetMsgData) < 256 , "BcData should be less than 256 in size so the size fits in a single byte." );
