// (c) 2017 Beem Media

#include "EGMake.h"
#include "EGWindowsAPI.h"
#include "EGFileData.h"


static void CreateOutputDirectory(eg_cpstr16 strOut);


static EGMakeResourceCompiler GetCompilerFunction()
{
	//Compiler type table:
	static struct
	{
		eg_cpstr     strExt;
		EGMakeResourceCompiler Func;
	}
	Data[] =
	{
		{ (".hlsl"     ) , EGMakeRes_hlsl },
		{ (".raw"      ) , EGMakeRes_Copy },
		{ (".tga"      ) , EGMakeRes_Image },
		{ (".jpg"      ) , EGMakeRes_Image },
		{ (".png"      ) , EGMakeRes_Image },
		{ (".bmp"      ) , EGMakeRes_Image },
		{ (".dds"      ) , EGMakeRes_Copy },
		{ (".ttf"      ) , EGMakeRes_efont },
		{ (".h"        ) , EGMakeRes_Ignore },
		{ (".hlsli"    ) , EGMakeRes_Ignore },
		{ (".exe"      ) , EGMakeRes_Ignore },
		{ (".txt"      ) , EGMakeRes_Ignore },
		{ (".emesh"    ) , EGMakeRes_emesh },
		{ (".eskel"    ) , EGMakeRes_eskel },
		{ (".emap"     ) , EGMakeRes_emap },
		{ (".emapc"    ) , EGMakeRes_emap },
		{ (".emap_int" ) , EGMakeRes_emap },
		{ (".ebuild"   ) , EGMakeRes_ebuild },
		{ (".ogg"      ) , EGMakeRes_Copy },
		{ (".r16"      ) , EGMakeRes_Ignore },
		{ (".egterrain") , EGMakeRes_Copy },
		{ (".eloc"     ) , EGMakeRes_eloc },
		{ (".wav"      ) , EGMakeRes_wav },
		{ (".esave"    ) , EGMakeRes_Copy },
		{ (".eskin"    ) , EGMakeRes_Copy },
		{ (".elyt"     ) , EGMakeRes_Copy },
		{ (".edef"     ) , EGMakeRes_Copy },
		{ (".esound"   ) , EGMakeRes_Copy },
		{ (".ecube"    ) , EGMakeRes_ecube },
		{ (".xml"      ) , EGMakeRes_Copy },
		{ (".efont"    ) , EGMakeRes_Copy },
		{ (".gcx"      ) , EGMakeRes_gcx },
		{ (".egsm"     ) , EGMakeRes_egsm },
		{ (".egasset"  ) , EGMakeRes_Copy },
		{ (".egsnd"    ) , EGMakeRes_Copy },
		{ (".egworld"  ) , EGMakeRes_Copy },
		{ (".ms3d"     ) , EGMakeRes_ms3d },
	};

	for(eg_uint i=0; i<countof(Data); i++)
	{
		if(EGString_EqualsI(EGMake_GetInputPath(FINFO_EXT), Data[i].strExt))
		{
			return Data[i].Func;
		}
	}

	EGLogf( eg_log_t::General , "No compiler for type \"%s\", using Copy.", EGMake_GetInputPath(FINFO_EXT));
	return EGMakeRes_Copy;
}

int EGResource_main( int argc , char* argv[] )
{
	eg_cpstr strIn  = 0;
	eg_cpstr strOut = 0;

	enum NEXT_T
	{
		NEXT_PARM,
		NEXT_IN,
		NEXT_OUT,
	};

	NEXT_T Next = NEXT_PARM;

	for(int i = 0; i < argc; i++)
	{
		if(NEXT_IN == Next)
		{
			strIn = argv[i];
			Next = NEXT_PARM;
		}
		else if(NEXT_OUT == Next)
		{
			strOut = argv[i];
			Next = NEXT_PARM;
		}
		else
		{
			if(eg_string_big(argv[i]).EqualsI(("-in")))
			{
				Next = NEXT_IN;
			}
			else if(eg_string_big(argv[i]).EqualsI( ("-out") ) )
			{
				Next = NEXT_OUT;
			}
		}
	}

	if((0 == strIn) || (0 == strOut))
	{
		EGLogf( eg_log_t::Error , ("Error: Invalid input: -in \"%s\" -out \"%s\""), strIn, strOut);
		return 0;
	}

	EGRMake_InitPaths(strIn, strOut);

	EGLogf( eg_log_t::General , ("\"%s\" -> \"%s\""), EGMake_GetInputPath(), EGMake_GetOutputPath(FINFO_NOEXT_FILE));

	{
		eg_string_big OutputPath = EGMake_GetOutputPath(FINFO_DIR);
		eg_char16 OutputPath16[MAX_PATH];
		OutputPath.CopyTo(OutputPath16, countof(OutputPath16));
		CreateOutputDirectory(OutputPath16);
	}

	//Currently we only handle copys.
	EGMakeResourceCompiler Func = GetCompilerFunction();
	eg_bool bSuccess = Func();
	if(!bSuccess)
	{
		EGLogf( eg_log_t::Error , ("Error: Failed to compile \"%s\"."), strIn);
	}
	else
	{
		eg_string_big sOFiles( ::EGMake_GetActualOutputFile() );
		if(sOFiles.Len() > 0)
			EGLogf( eg_log_t::General , ("\tOutputs: \"%s\""), sOFiles.String());
	}
	return bSuccess ? 0 : -1;
}

static void CreateOutputDirectory(eg_cpstr16 strDir)
{
	SHCreateDirectory( NULL, strDir );
}