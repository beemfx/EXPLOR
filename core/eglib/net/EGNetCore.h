// (c) 2018 Beem Media

#pragma once


void EGNetCore_Init();
void EGNetCore_Deinit();
eg_bool EGNetCore_MakeRequest( eg_cpstr16 InUrl , const eg_d_string8& InPostData , EGArray<eg_byte>& DataOut , eg_int NumRetries = 1 , eg_real RetryDelay = 3.f );

static inline eg_d_string8 EGNetCore_MakeRequest( eg_cpstr16 InUrl , const eg_d_string8& InPostData = eg_d_string8() , eg_int NumRetries = 1 , eg_real RetryDelay = 3.f  ) { EGArray<eg_byte> GottenData; EGNetCore_MakeRequest( InUrl , InPostData , GottenData , NumRetries , RetryDelay ); GottenData.Append( '\0' ); eg_d_string8 Out; Out.Append( reinterpret_cast<eg_char8*>(GottenData.GetArray() )); return std::move(Out);  }