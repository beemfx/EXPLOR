// (c) 2018 Beem Media

#pragma once

class EGFileData;

void EGEngineDataAssetLib_QuaryFilenameValues( const eg_string_base& Ext , eg_bool bUseRelativePath , eg_cpstr RelativePath , EGArray<eg_string_big>& ValuesOut );
eg_bool EGEngineDataAssetLib_GameDataOpenFn( eg_cpstr16 Filename , EGFileData& FileDataOut );
eg_bool EGEngineDataAssetLib_GameDataSaveFn( eg_cpstr16 Filename , const EGFileData& FileData );
