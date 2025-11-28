// (c) 2014 Beem Media

#include "EGNetRequest.h"
#include "EGFileData.h"
#include "EGThreadRequest.h"
#include "EGWorkerThreads.h"
#include "EGList.h"
#include "EGNetCore.h"
#include "EGGlobalConfig.h"

static class EGNetRequest
{
public:
	EGNetRequest();
	~EGNetRequest();

public:
	void Init( eg_cpstr GameStr );
	void Deinit();

	egNetRequestId MakeRequest( eg_cpstr strFile , INetRequestCallback* L );
	void CancelRequest( const egNetRequestId& ReqId );
private:
	struct egRequestData : public IRequestCallback , public IListable
	{
		INetRequestCallback* pLoadable;
		EGNetRequest*        Owner;
		eg_char              RequestString[128];
		egNetRequestId       Id;
		eg_string_crc        Result;
		EGFieldList*         FieldList;
		eg_bool              bWasCanceled:1;

		virtual void Action() override final;
		virtual void Callback() override final;
	};

	egNetRequestId m_NextRequestId;

	EGList<egRequestData> m_CurReqs;
	eg_string             m_GameStr;

private:
	void ObtainRequestData( egRequestData* D ); //Request thread.
	void ReleaseRequestData( EGFieldList* Data ); //Main thread.

} NetRequest;

void EGNetRequest::egRequestData::Action()
{
	Owner->ObtainRequestData( this );
}

void EGNetRequest::egRequestData::Callback()
{
	assert( EGWorkerThreads_IsThisMainThread() );
	if( bWasCanceled )
	{
		EGLogf( eg_log_t::Web , __FUNCTION__ ": Web request %u completed, but it had been canceled." , Id.Id );
	}
	else
	{
		pLoadable->Callback( Id , Result , FieldList );
	}
	Owner->ReleaseRequestData( FieldList );
	Owner->m_CurReqs.Remove( this );
	delete this;
}


EGNetRequest::EGNetRequest()
: m_NextRequestId(INVALID_ID)
, m_CurReqs( EGList<egRequestData>::DEFAULT_ID )
{

}

EGNetRequest::~EGNetRequest()
{

}

egNetRequestId EGNetRequest::MakeRequest( eg_cpstr Request , INetRequestCallback* L )
{
	assert( EGWorkerThreads_IsThisMainThread() );

	m_NextRequestId.Id++;
	if( m_NextRequestId.Id == 0 ) // We never want an id of 0 (theoretically we should never wrap around, but just in case)
	{
		m_NextRequestId.Id++;
	}
	egNetRequestId NewId = m_NextRequestId;
	egRequestData* LD = new ( eg_mem_pool::DefaultHi ) egRequestData;
	if( LD )
	{
		LD->Id = NewId;
		LD->pLoadable = L;
		LD->Owner = this;
		LD->FieldList = nullptr;
		LD->Result = eg_crc("");
		EGString_Copy( LD->RequestString , Request , countof(LD->RequestString) );
		LD->bWasCanceled = false;
		m_CurReqs.Insert( LD );
		WorkerThread->MakeRequest( LD );
	}
	return NewId;
}

void EGNetRequest::CancelRequest( const egNetRequestId& ReqId )
{
	assert( EGWorkerThreads_IsThisMainThread() );
	egRequestData* ToRemove = nullptr;
	for( egRequestData* Search : m_CurReqs )
	{
		if( ReqId == Search->Id )
		{
			ToRemove = Search;
			break;
		}
	}
	
	if( ToRemove )
	{
		EGLogf( eg_log_t::Web , __FUNCTION__ ": Cancelled web request %u." , ReqId.Id );
		ToRemove->bWasCanceled = true;
	}
	else
	{
		assert( false );
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Failed to cancel web request %u." , ReqId.Id );
	}
}


void EGNetRequest::Init( eg_cpstr GameStr )
{
	m_GameStr = GameStr;
	m_GameStr.ConvertToLower();

	EGNetCore_Init();
}

void EGNetRequest::Deinit()
{
	while( m_CurReqs.HasItems() )
	{
		egRequestData* pData = m_CurReqs.GetOne();
		WorkerThread->CancelRequest( pData );
		assert( pData->bWasCanceled ); // Requests should have been canceled before the game was quit.
		pData->bWasCanceled = true;
		pData->Callback(); // This will clean up the request.
	}

	assert( m_CurReqs.Len() == 0 );
	EGNetCore_Deinit();
}

void EGNetRequest::ObtainRequestData( egRequestData* D )
{
	assert( nullptr == D->FieldList );
	eg_d_string8 RequestStr = EGString_Format( "%s:%s" , m_GameStr.String() , D->RequestString );

	EGArray<eg_byte> WebData;
	eg_d_string16 UrlRequest = *NetConfig_DefaultServer.GetValueThreadSafe();
	eg_bool bGotData = EGNetCore_MakeRequest( *UrlRequest , RequestStr , WebData );

	if( bGotData )
	{
		D->FieldList = (EGFieldList*)EGMem2_Alloc( sizeof(EGFieldList) , eg_mem_pool::DefaultHi );
		if( D->FieldList )
		{
			D->FieldList->DataSize = WebData.Len();
			D->FieldList->Data = (eg_char*)EGMem2_Alloc( D->FieldList->DataSize , eg_mem_pool::DefaultHi );
			if( D->FieldList->Data )
			{
				D->Result = eg_crc("SUCCESS");
				EGMem_Copy( D->FieldList->Data , WebData.GetArray() , D->FieldList->DataSize );
			}
			else
			{
				assert( false ); //Ran out of memory?
				D->Result = eg_crc("FAIED:OUT_OF_MEM2");
			}
		}
		else
		{
			assert( false ); //Ran out of memory?
			D->Result = eg_crc("FAIED:OUT_OF_MEM3");
		}
	}
	else
	{
		D->Result = eg_crc("FAILED:REQUEST_FAILED");
	}

}

void EGNetRequest::ReleaseRequestData( EGFieldList* Data )
{
	if( !Data )return;

	if( Data->Data )EGMem2_Free( Data->Data );
	EGMem2_Free( Data );
}

void NetRequest_Init( eg_cpstr GameStr ){ NetRequest.Init( GameStr ); }
void NetRequest_Deinit( void ){ NetRequest.Deinit(); }
void NetRequest_Update( eg_real DeltaTime )
{
	unused( DeltaTime ); 
}
egNetRequestId NetRequest_MakeRequest( eg_cpstr Request , INetRequestCallback* Callback )
{
	return NetRequest.MakeRequest( Request , Callback ); 
}

void NetRequest_CancelRequest( const egNetRequestId& ReqId )
{
	return NetRequest.CancelRequest( ReqId );
}