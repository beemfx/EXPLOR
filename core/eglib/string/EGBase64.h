// (c) 2019 Beem Media

#pragma once

class EGFileData;

eg_bool EGBase64_Encode( const void* DataIn , eg_size_t DataInSize , eg_d_string8& StringOut );
eg_size_t EGBase64_Decode( eg_cpstr8 StringIn , void* DataOut , eg_size_t DataOutSize , eg_bool bStrict = false );
eg_bool EGBase64_Encode( const EGArray<eg_byte>& DataIn , eg_d_string8& StringOut );
eg_bool EGBase64_Decode( const eg_d_string& StringIn , EGArray<eg_byte>& DataOut , eg_bool bStrict = false );

static inline eg_d_string8 EGBase64_Encode( const void* DataIn , eg_size_t DataInSize ) { eg_d_string8 Out; EGBase64_Encode( DataIn , DataInSize , Out ); return std::move( Out ); }
static inline eg_d_string8 EGBase64_Encode( const EGArray<eg_byte>& DataIn ) { eg_d_string8 Out; EGBase64_Encode( DataIn , Out ); return std::move( Out ); }
static inline EGArray<eg_byte> EGBase64_Decode( const eg_d_string& StringIn ) { EGArray<eg_byte> Out; EGBase64_Decode( StringIn , Out ); return std::move( Out ); }
eg_d_string8 EGBase64_Encode( const EGFileData& DataIn );
void EGBase64_Decode( const eg_d_string& StringIn , EGFileData& DataOut );
