// (c) 2017 Beem Media

#pragma once

class EGFileData;

eg_string_big EGMake_GetGameToBuildFromReg();
eg_cpstr EGMake_GetEGSRC();
eg_cpstr EGMake_GetEGOUT();

//Helper functions:
eg_bool  EGMake_WriteOutputFile( eg_cpstr strFile , const eg_byte* pData , eg_size_t Size );
eg_bool  EGMake_WriteOutputFile( eg_cpstr strFile , const EGFileData& MemFile );
eg_bool  EGMake_ReadInputFile( eg_cpstr strFile , EGFileData& MemFile );
eg_bool EGMake_HasDataChanged( eg_cpstr Directory , eg_cpstr CheckpointFile );

//Resource compilers:
eg_bool EGMakeRes_Copy(eg_cpstr ExtOverride);
eg_bool EGMakeRes_Copy();
eg_bool EGMakeRes_hlsl();
eg_bool EGMakeRes_efont();
eg_bool EGMakeRes_Image();
eg_bool EGMakeRes_wav();
eg_bool EGMakeRes_Ignore();
eg_bool EGMakeRes_emesh();
eg_bool EGMakeRes_eskel();
eg_bool EGMakeRes_emap();
eg_bool EGMakeRes_eterrain();
eg_bool EGMakeRes_eloc();
eg_bool EGMakeRes_ecube();
eg_bool EGMakeRes_ebuild();
eg_bool EGMakeRes_egsm();
eg_bool EGMakeRes_ms3d();
eg_bool EGMakeRes_gcx();

eg_bool EGMake_gcx( eg_cpstr strIn , eg_cpstr strOutEmap , eg_bool bCompress );

eg_bool EGMake_ProcessImage(eg_cpstr InPath, eg_cpstr OutPath);

typedef eg_bool (  * EGMakeResourceCompiler )();

void EGRMake_InitPaths( eg_cpstr strIn , eg_cpstr strOut );

enum EGR_FINFO_T
{
	FINFO_FULL_FILE,
	FINFO_NOEXT_FILE,
	FINFO_EXT,
	FINFO_DIR,
	FINFO_SHORT,
	FINFO_SHORT_NOEXT,
};

eg_cpstr EGMake_GetInputPath(EGR_FINFO_T t = FINFO_FULL_FILE);
eg_cpstr EGMake_GetOutputPath(EGR_FINFO_T t = FINFO_FULL_FILE);

void     EGMake_SetActualOutputFile(eg_cpstr strFilename);
eg_cpstr EGMake_GetActualOutputFile();

