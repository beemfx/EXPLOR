// (c) 2017 Beem Media

#include "EGMake.h"

#include "EGStdLibAPI.h"
#include "EGWindowsAPI.h"
#include "EGExtProcess.h"
#include "EGPath2.h"

eg_bool EGMake_ProcessImage( eg_cpstr InPath , eg_cpstr OutPath )
{
	// texconv doesn't like for the directory to end with a \ since it does wierd things with the quotes.
	eg_string_big OutputDir = *EGPath2_BreakPath( OutPath ).GetDirectory( '\\' );
	if( OutputDir.Len() > 0 && OutputDir[OutputDir.Len()-1] == '\\' )
	{
		OutputDir.ClampEnd( 1 );
	}

	// In the past we had -pow2 option but it tends to downscale the image if it's not already pow2 so we took it out (modern video cards don't seem to care).
	eg_string_big InputPath = *EGPath2_CleanPath( InPath , '\\' );

	eg_string_big strCmd = EGString_Format(("texconv.exe -nologo -srgb -y -fl 9.3 -ft DDS -f R8G8B8A8_UNORM -o \"%s\" \"%s\""), OutputDir.String(), InputPath.String());
	// EGLogf( eg_log_t::General , "%s" , strCmd.String() );
	eg_bool Res = EGExtProcess_Run( strCmd , nullptr );

	if (Res)
	{
		//texconv always gives the file the same name as the input file, so we want to rename it.
		eg_string_big StrActualOutput = EGString_Format("%s\\%s.DDS", OutputDir.String(), EGMake_GetInputPath(FINFO_SHORT_NOEXT));
		eg_string_big StrWantedOutput = EGString_Format("%s\\%s.DDS", OutputDir.String(), EGMake_GetOutputPath(FINFO_SHORT_NOEXT));
		if (!StrActualOutput.EqualsI(StrWantedOutput))
		{
			::DeleteFileA(StrWantedOutput);
			bool WasRenamed = 0 != ::MoveFileA(StrActualOutput, StrWantedOutput);
			Res = WasRenamed;
		}
	}

	return Res;
}

eg_bool EGMakeRes_Image()
{
	return EGMake_ProcessImage( EGMake_GetInputPath() , EGMake_GetOutputPath() );
}

eg_bool EGMakeRes_wav()
{
	eg_cpstr strOpts = "-Q";
	eg_uint32 SerialNumber = eg_string_crc( EGMake_GetInputPath( FINFO_SHORT_NOEXT ) ).ToUint32();
	eg_string_big strCmd = EGString_Format( ("oggenc2.exe %s -s %u -o \"%s.ogg\" \"%s\"") , strOpts , SerialNumber , EGMake_GetOutputPath( FINFO_NOEXT_FILE ) , EGMake_GetInputPath() );
	eg_bool MadeFile = EGExtProcess_Run( strCmd , nullptr );
	return MadeFile;
}

eg_bool EGMakeRes_emap()
{
	eg_string_big In = EGMake_GetInputPath();
	eg_string_big OutEmap = EGString_Format( ("%s.emap") , EGMake_GetOutputPath( FINFO_NOEXT_FILE ) );
	//eg_string_big OutPhysx32 = EGString_Format( ("%s.emapphysx32") , EGR_GetOutputPath( FINFO_NOEXT_FILE ) );
	eg_string_big OutPhysx64 = EGString_Format( ("%s.emapphysx64") , EGMake_GetOutputPath( FINFO_NOEXT_FILE ) );

	//eg_string_big Cmd32 = EGString_Format( ( "egmake2_x64.exe MAP -in \"%s\" -emap \"%s\" -physx \"%s\"" ) , In.String() , OutEmap.String() , OutPhysx32.String() );
	//eg_string_big Cmd64 = EGString_Format( ( "egmake2_x64.exe MAP -in \"%s\" -emap \"%s\" -physx \"%s\"" ) , In.String() , OutEmap.String() , OutPhysx64.String() );

	//The command can be quit long, so don't use an eg_string_big
	//eg_char Cmd32[1024];
	eg_char Cmd64[1024];

	//EGString_FormatToBuffer( Cmd32 , countof(Cmd32) , ( "egmake2_x86.exe MAP -in \"%s\" -emap \"%s\" -physx \"%s\"" ) , In.String() , OutEmap.String() , OutPhysx32.String() );
	EGString_FormatToBuffer( Cmd64 , countof(Cmd64) , ( "egmake2_x64.exe MAP -in \"%s\" -emap \"%s\" -physx \"%s\"" ) , In.String() , OutEmap.String() , OutPhysx64.String() );

	eg_bool Succ = true;
	//Succ = Succ && EGR_External(Cmd32);
	Succ = Succ && EGExtProcess_Run( Cmd64 , nullptr );
	return Succ;
}
