// NetLocal - An interface that treats local data as network traffic.
// (c) 2015 Beem Software
#pragma once

struct egNetMsgData;

DECLARE_ID_STRUCT( egNetLocalId );

extern const egNetLocalId NET_LOCAL_ID_NOT_CONNECTED;

egNetLocalId NetLocal_Client_Connect();
void NetLocal_Client_Disconnect( egNetLocalId Id );
void NetLocal_Client_PostMsg( egNetLocalId Id , const egNetMsgData* Data );
void NetLocal_Client_HandleReceivedMsgs( egNetLocalId Id , void ( * Callback )( const egNetMsgData* Data , void* UserPointer ) , void* UserPointer );

void NetLocal_Server_BeginHosting();
void NetLocal_Server_EndHosting();
void NetLocal_Server_PostMsg( const egNetMsgData* Data );
void NetLocal_Server_HandleReceivedMsgs( void ( * Callback )( const egNetMsgData* Data , void* UserPointer ) , void* UserPointer );

eg_bool NetLocal_IsClientConnected( egNetLocalId Id );
void NetLocal_FlushAll();
