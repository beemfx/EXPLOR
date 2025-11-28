// NetLocal
// (c) 2015 Beem Software

#include "EGNetLocal.h"
#include "EGNetMsgBuffer.h"
#include "EGEngineConfig.h"

const egNetLocalId NET_LOCAL_ID_NOT_CONNECTED = INVALID_ID;

struct egNetLocalCliData
{
	EGNetMsgBuffer RecMsgs;
	eg_bool        IsConnected;

	egNetLocalCliData(): IsConnected( false ){ }
};

static struct egNetLocalData
{
	EGNetMsgBuffer    SrvMsgs;
	egNetLocalCliData CliData[MAX_LOCAL_CLIENTS];
	eg_bool           IsSrvHosting:1;

	egNetLocalData(): IsSrvHosting(false){ }
} NetLocal_Data;

static eg_bool NetLocal_IsConnEstablished( egNetLocalId Id )
{
	if( Id == NET_LOCAL_ID_NOT_CONNECTED )return false;

	return NetLocal_Data.CliData[egNetLocalId::IdToIndex(Id)].IsConnected && NetLocal_Data.IsSrvHosting;
}

egNetLocalId NetLocal_Client_Connect()
{
	egNetLocalId IdOut = NET_LOCAL_ID_NOT_CONNECTED;

	for( eg_uint i=0; NET_LOCAL_ID_NOT_CONNECTED == IdOut && i<countof(NetLocal_Data.CliData); i++ )
	{
		egNetLocalCliData* CliData = &NetLocal_Data.CliData[i];
		if( !CliData->IsConnected )
		{
			CliData->IsConnected = true;
			assert( CliData->RecMsgs.IsEmpty() );
			IdOut = egNetLocalId::IndexToId( i );
		}
	}

	assert( IdOut != NET_LOCAL_ID_NOT_CONNECTED ); //Too many local connections? Bad connect/disconnect logic somewhere.

	return IdOut;
}

void NetLocal_Client_Disconnect( egNetLocalId Id )
{
	if( NET_LOCAL_ID_NOT_CONNECTED == Id )return;
	egNetLocalCliData* CliData = &NetLocal_Data.CliData[egNetLocalId::IdToIndex(Id)];
	CliData->RecMsgs.FlushMsgs();
	CliData->IsConnected = false;
}

void NetLocal_Client_PostMsg( egNetLocalId Id , const egNetMsgData* Data )
{
	if( !NetLocal_IsConnEstablished(Id) )return;
	NetLocal_Data.SrvMsgs.AddMsg( Data );
}

void NetLocal_Client_HandleReceivedMsgs( egNetLocalId Id , void ( * Callback )( const struct egNetMsgData* Data , void* UserPointer ) , void* UserPointer )
{
	if( NetLocal_IsConnEstablished(Id) )
	{
		NetLocal_Data.CliData[egNetLocalId::IdToIndex(Id)].RecMsgs.PumpMsgs( Callback , UserPointer );
	}
}

void NetLocal_Server_BeginHosting()
{
	assert( !NetLocal_Data.IsSrvHosting );
	assert( NetLocal_Data.SrvMsgs.IsEmpty() );
	NetLocal_Data.IsSrvHosting = true;
}

void NetLocal_Server_EndHosting()
{
	NetLocal_Data.IsSrvHosting = false;
	NetLocal_Data.SrvMsgs.FlushMsgs();
}

void NetLocal_Server_PostMsg( const egNetMsgData* Data )
{
	if( !NetLocal_Data.IsSrvHosting )return;

	//Send the message to all connected clients:
	for( eg_uint i=0; i<countof(NetLocal_Data.CliData); i++ )
	{
		egNetLocalCliData* CliData = &NetLocal_Data.CliData[i];
		if( CliData->IsConnected )
		{
			if( Data->Header.NetId == NET_ID_ALL || Data->Header.NetId.Id == egNetCommId::IndexToId(i).Id )
			{
				CliData->RecMsgs.AddMsg( Data );
			}
		}
	}
}

void NetLocal_Server_HandleReceivedMsgs( void ( * Callback )( const struct egNetMsgData* Data , void* UserPointer ) , void* UserPointer )
{
	if( NetLocal_Data.IsSrvHosting )
	{
		NetLocal_Data.SrvMsgs.PumpMsgs( Callback , UserPointer );
	}
	else
	{
		NetLocal_Data.SrvMsgs.FlushMsgs();
	}
}

eg_bool NetLocal_IsClientConnected( egNetLocalId Id )
{
	if( NET_LOCAL_ID_NOT_CONNECTED == Id )return false;
	egNetLocalCliData* CliData = &NetLocal_Data.CliData[egNetLocalId::IdToIndex(Id)];
	return CliData->IsConnected;
}

void NetLocal_FlushAll()
{
	NetLocal_Data.SrvMsgs.FlushMsgs();
	for( eg_uint i=0; i<countof(NetLocal_Data.CliData); i++ )
	{
		egNetLocalCliData* CliData = &NetLocal_Data.CliData[i];
		CliData->RecMsgs.FlushMsgs();
	}
}