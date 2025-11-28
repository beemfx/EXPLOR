// (c) 2018 Beem Media

#pragma once

class EGFileData;

void EGEdUtility_Init();
void EGEdUtility_Deinit();
eg_bool EGEdUtility_SaveFile( eg_cpstr16 Filename , const EGFileData& OutFile );
eg_bool EGEdUtility_CheckoutFile( eg_cpstr16 Filename );
eg_bool EGEdUtility_RevertFile( eg_cpstr16 Filename , eg_bool bOnlyIfUnchanged );
eg_bool EGEdUtility_AddFile( eg_cpstr16 Filename );
eg_bool EGEdUtility_DeleteFile( eg_cpstr16 Filename );
static inline eg_bool EGEdUtility_SaveFile( eg_cpstr8 Filename , const EGFileData& OutFile ) { return EGEdUtility_SaveFile( EGString_ToWide(Filename) , OutFile ); }
static inline eg_bool EGEdUtility_RevertFile( eg_cpstr8 Filename , eg_bool bOnlyIfUnchanged ) { return EGEdUtility_RevertFile( EGString_ToWide(Filename) , bOnlyIfUnchanged ); }
static inline eg_bool EGEdUtility_AddFile( eg_cpstr8 Filename ) { return EGEdUtility_AddFile( EGString_ToWide(Filename) ); }
void EGEdUtility_CleanGameData( eg_bool bConfirm );
eg_bool EGEdUtility_SaveIfUnchanged( eg_cpstr Filename , const EGFileData& FileData , eg_bool bCheckInToScc = false );
eg_bool EGEdUtility_SaveIfUnchanged( eg_cpstr16 Filename , const EGFileData& FileData , eg_bool bCheckInToScc = false );
