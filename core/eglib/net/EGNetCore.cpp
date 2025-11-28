// (c) 2018 Beem Media

#include "EGNetCore.h"
#include "EGWindowsAPI.h"
#include "EGThread.h"
#include <winhttp.h>

class EGNetCore
{
public:

	void Init()
	{
		
	}

	void Deinit()
	{

	}

	eg_bool InitSession( eg_cpstr16 Server , eg_bool bSecureConnection , HINTERNET& Session , HINTERNET& Connect )
	{
		assert( Session == nullptr && Connect == nullptr );

		Session = WinHttpOpen( L"EGNetCore" , WINHTTP_ACCESS_TYPE_DEFAULT_PROXY , WINHTTP_NO_PROXY_NAME , WINHTTP_NO_PROXY_BYPASS , 0 );
		if( Session )
		{
			Connect = WinHttpConnect( Session , Server , bSecureConnection ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT , 0 );
			if( Connect )
			{
				// All is good.
			}
			else
			{
				EGLogf( eg_log_t::Error , __FUNCTION__ ": Failed to create connection." );
				WinHttpCloseHandle( Session );
				Session = nullptr;
			}
		}
		else
		{
			EGLogf( eg_log_t::Error , __FUNCTION__ ": Failed to create session." );
		}

		return Session != nullptr && Connect != nullptr;
	}

	void DeinitSession( HINTERNET& Session , HINTERNET& Connect )
	{
		assert( nullptr != Session && nullptr != Connect );
		if( Connect )
		{
			WinHttpCloseHandle( Connect );
			Connect = nullptr;
		}
		if( Session )
		{
			WinHttpCloseHandle( Session );
			Session = nullptr;
		}
	}

	eg_bool MakeRequestInternal( eg_cpstr16 Server , eg_bool bSecure , eg_cpstr16 ObjectName , const eg_d_string8& PostData , EGArray<eg_byte>& DataOut )
	{
		EGArray<eg_byte> RecBuffer;

		eg_bool bWasSuccessful = false;

		HINTERNET Session = nullptr;
		HINTERNET Connect = nullptr;
		eg_bool bCreatedSession = InitSession( Server , bSecure , Session , Connect );

		if( !bCreatedSession )
		{
			return bWasSuccessful;
		}

		eg_cpstr16 AdditionalHeaders = L"content-type:text/plain";

		HINTERNET Request = WinHttpOpenRequest( Connect , PostData.Len() > 0 ? L"POST" : L"GET" , ObjectName , L"HTTP/1.1" , WINHTTP_NO_REFERER , WINHTTP_DEFAULT_ACCEPT_TYPES , bSecure ? WINHTTP_FLAG_SECURE : 0 );

		if( Request )
		{
			EGArray<eg_char8> RequestStringBuffer;
			RequestStringBuffer.Append( *PostData , PostData.Len() );
			const DWORD RequestStringLen = RequestStringBuffer.LenAs<DWORD>();
			const BOOL bSentRequest = WinHttpSendRequest( Request , AdditionalHeaders , -1 , reinterpret_cast<LPVOID>(RequestStringBuffer.GetArray()) , RequestStringLen , RequestStringLen , 0 );//WINHTTP_NO_REQUEST_DATA , 0 , 0 , 0 );
			if( bSentRequest )
			{
				const BOOL bReceivedResponse = WinHttpReceiveResponse( Request , NULL );
				if( bReceivedResponse )
				{
					DWORD StatusCode = 0;
					DWORD StatusCodeSize = sizeof(StatusCode);

					BOOL bQueriedHeaders = WinHttpQueryHeaders( Request , WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER , WINHTTP_HEADER_NAME_BY_INDEX , &StatusCode , &StatusCodeSize, WINHTTP_NO_HEADER_INDEX );

					if( bQueriedHeaders && (StatusCode == HTTP_STATUS_OK || StatusCode == HTTP_STATUS_BAD_REQUEST || StatusCode == HTTP_STATUS_DENIED) )
					{
						bWasSuccessful = true;
						// Keep checking for data until there is nothing left.
						DWORD AvailableSize = 0;
						do 
						{
							// Check for available data.
							const BOOL bGotAvailableSize = WinHttpQueryDataAvailable( Request, &AvailableSize );
							if( bGotAvailableSize )
							{
								if( bWasSuccessful && AvailableSize > 0 )
								{
									DWORD dwDownloaded = 0;
									RecBuffer.Resize( AvailableSize );
									if( RecBuffer.Len() == AvailableSize )
									{
										const BOOL bReadData = WinHttpReadData( Request , RecBuffer.GetArray() , RecBuffer.LenAs<DWORD>() , &dwDownloaded );

										if( bReadData )
										{
											DataOut.Append( RecBuffer.GetArray() , dwDownloaded );
										}
										else
										{
											EGLogf( eg_log_t::Error , "Failed to read data." );
											bWasSuccessful = false;
										}
									}
									else
									{
										EGLogf( eg_log_t::Error , "Ran out of buffer memory." );
										bWasSuccessful = false;
									}
								}
							}
							else
							{
								EGLogf( eg_log_t::Error , "Failed to query data." );
								bWasSuccessful = false;
							}

						} while( bWasSuccessful && AvailableSize > 0 );
					}
					else
					{
						EGLogf( eg_log_t::Warning , "Failed to obtain data, request error %d." , StatusCode );
					}
				}
				else
				{
					EGLogf( eg_log_t::Warning , "Failed to receive response." );
				}
			}
			else
			{
				EGLogf( eg_log_t::Warning , "Failed to send request." );
			}

			WinHttpCloseHandle( Request );
		}
		else
		{
			EGLogf( eg_log_t::Warning , "Failed to open request." );
		}

		DeinitSession( Session , Connect );

		return bWasSuccessful;
	}

	eg_bool MakeRequest( eg_cpstr16 Server , eg_bool bSecure , eg_cpstr16 ObjectName , const eg_d_string8& PostData , EGArray<eg_byte>& DataOut , eg_int NumRetries , eg_real RetryDelay )
	{
		for( eg_int i=0; i<=NumRetries; i++ ) // <= because we still want to do this even if NumRetries is 0
		{
			eg_bool bReqResult = MakeRequestInternal( Server , bSecure , ObjectName , PostData , DataOut );
			if( bReqResult )
			{
				return true;
			}
			if( i != NumRetries )
			{
				EGLogf( eg_log_t::Warning , "Retrying net request %d more times in %g seconds." , NumRetries-i , RetryDelay );
				EGThread_Sleep( RetryDelay );
			}
		}
		return false;
	}

	eg_bool MakeRequest( eg_cpstr16 InUrl, const eg_d_string8& InPostData, EGArray<eg_byte>& DataOut , eg_int NumRetries , eg_real RetryDelay )
	{
		URL_COMPONENTSW UrlComp;
		zero( &UrlComp );
		eg_char16 HostName[MAX_PATH];
		eg_char16 UrlPath[MAX_PATH*5];
		UrlComp.dwStructSize = sizeof( UrlComp );
		UrlComp.lpszHostName = HostName;
		UrlComp.dwHostNameLength = MAX_PATH;
		UrlComp.lpszUrlPath = UrlPath;
		UrlComp.dwUrlPathLength = MAX_PATH*5;
		UrlComp.dwSchemeLength = 1; // None zero
		const BOOL bCrackedUrl = WinHttpCrackUrl( InUrl , EG_To<DWORD>( EGString_StrLen( InUrl ) ), 0, &UrlComp );
		eg_bool bSucceeded = false;
		if( bCrackedUrl )
		{
			bSucceeded = MakeRequest( HostName , UrlComp.nScheme == INTERNET_SCHEME_HTTPS , UrlPath , InPostData , DataOut , NumRetries , RetryDelay );
		}

		return bSucceeded;
	}
};

static EGNetCore EGNetCore_Inst;

void EGNetCore_Init()
{
	EGNetCore_Inst.Init();
}

void EGNetCore_Deinit()
{
	EGNetCore_Inst.Deinit();
}

eg_bool EGNetCore_MakeRequest( eg_cpstr16 InUrl , const eg_d_string8& InPostData , EGArray<eg_byte>& DataOut , eg_int NumRetries /*= 0*/ , eg_real RetryDelay /*= 3.f*/ )
{
	return EGNetCore_Inst.MakeRequest( InUrl , InPostData , DataOut , NumRetries , RetryDelay );
}
