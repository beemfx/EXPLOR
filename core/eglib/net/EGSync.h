// (c) 2018 Beem Media

#pragma once

class EGFileData;

void EGSync_PackBigFile( eg_cpstr InDir , eg_cpstr OutFile );
void EGSync_UnpackBigFile( eg_cpstr InFile , eg_cpstr OutDir );
void EGSync_SplitBigFile( eg_cpstr BigFile , eg_cpstr OutDir );
void EGSync_AssembleBigFile( eg_cpstr InDir , eg_cpstr OutFile );
void EgSync_ReadManifest( const EGArray<eg_byte>& FileData , EGArray<eg_s_string_sml8>& Out );