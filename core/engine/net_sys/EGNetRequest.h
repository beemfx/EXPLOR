/******************************************************************************
EGNetRequest - A Module for making http requests to the beemsoft server.

(c) 2014 Beem Software
******************************************************************************/

#pragma once

DECLARE_ID_STRUCT( egNetRequestId );

class EGFieldList
{
public:
	eg_char* Data;
	eg_size_t DataSize;
};

class INetRequestCallback
{
public:
	//Main thread functionality:
	virtual void Callback( const egNetRequestId& ReqId , eg_string_crc Result , class EGFieldList* Data )=0;
};


void NetRequest_Init( eg_cpstr GameStr );
void NetRequest_Deinit( void );
void NetRequest_Update( eg_real DeltaTime );
egNetRequestId NetRequest_MakeRequest( eg_cpstr Request , INetRequestCallback* Callback );
void NetRequest_CancelRequest( const egNetRequestId& ReqId );