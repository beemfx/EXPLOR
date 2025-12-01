@echo off
echo [DATASYNC] ===== EG Data Sync ======
echo [DATASYNC] Starting data sync...

CD /D %~1%
set EGSRC=%~1%
set EGOUT=%~1%_BUILD\

echo Building from "%EGSRC%" to "%EGOUT%".

rem MSBuild EG.sln -nologo -t:"Engine\Tools\egmake2" -v:m -p:Configuration=ReleaseEditor;Platform=x64
rem 
rem exit 0

if "x%EGSRC%" == "x" goto had_pathserror
if "x%EGOUT%" == "x" goto had_pathserror

set PATH=%EGOUT%\bin;%EGSRC%\core\BuildTools\bin;%EGSRC%\core\BuiltTools\bat;%PATH%

echo [DATASYNC] Creating directories...
if not exist "%EGOUT%\bin" @md "%EGOUT%\bin"
if not exist "%EGOUT%\PreBuild" @md "%EGOUT%\PreBuild"
if not exist "%EGOUT%\libs" @md "%EGOUT%\libs"
if not exist "%EGOUT%\libs\x64_Debug" @md "%EGOUT%\libs\x64_Debug"
if not exist "%EGOUT%\libs\x64_Release" @md "%EGOUT%\libs\x64_Release"

echo [PRELIBS] Copying prebuilt librarys
if exist "%EGOUT%\libs\x64_Release\steam_api64.lib" goto prelibs_done

copy "%EGSRC%\core\lib3p\steamworks\sdk\redistributable_bin\win64\steam_api64.lib" "%EGOUT%\libs\x64_Debug\"
copy "%EGSRC%\core\lib3p\steamworks\sdk\redistributable_bin\win64\steam_api64.lib" "%EGOUT%\libs\x64_Release\"
copy "%EGSRC%\core\lib3p\steamworks\sdk\redistributable_bin\win64\steam_api64.dll" "%EGOUT%\bin\"

copy "%EGSRC%\core\lib3p\xaudio2\release\lib\x64\xaudio2_9redist.lib" "%EGOUT%\libs\x64_Debug\"
copy "%EGSRC%\core\lib3p\xaudio2\release\lib\x64\xaudio2_9redist.lib" "%EGOUT%\libs\x64_Release\"
copy "%EGSRC%\core\lib3p\xaudio2\release\bin\x64\xaudio2_9redist.dll" "%EGOUT%\bin\"

copy "%EGSRC%\core\lib3p\xinput\release\lib\x64\XInput.lib" "%EGOUT%\libs\x64_Debug\"
copy "%EGSRC%\core\lib3p\xinput\release\lib\x64\XInput.lib" "%EGOUT%\libs\x64_Release\"
copy "%EGSRC%\core\lib3p\xinput\release\bin\x64\xinput1_3.dll" "%EGOUT%\bin\"

:prelibs_done
echo [PRELIBS] Done.

echo [PREBIN] Copying prebuilt librarys
if exist "%EGOUT%\bin\steam_api64.dll" goto prebins_done

copy "%EGSRC%\core\lib3p\steamworks\sdk\redistributable_bin\win64\steam_api64.dll" "%EGOUT%\bin\"

:prebins_done
echo [PREBIN] Done.

echo [PREBUILT BINARIES] Copying prebuilt binaries...
call "DiffDir.exe" -Q -D "%EGSRC%\core\libs\bin" -O "%EGOUT%\PreBuild\diffs_binaries.txt"
if %ERRORLEVEL% == 0 goto binaries_nochanges

goto binaries_done
:binaries_nochanges
echo [PREBUILT BINARIES] No changes detected.
:binaries_done
echo [PREBUILT BINARIES] Done.

set MSBUILDOPTS=/nologo /v:m

echo [LIB3P] Building 3rd party libraries...
call "DiffDir.exe" -Q -D "%EGSRC%\core\lib3p" -O "%EGOUT%\PreBuild\diffs_lib3p.txt"
if %ERRORLEVEL% == 0 goto lib3p_nochanges

rem Build all 4 versions of lib3p
MSBuild EG.sln %MSBUILDOPTS% -t:"Engine\Libs\lib3p" -p:Configuration=Release;Platform=x64
MSBuild EG.sln %MSBUILDOPTS% -t:"Engine\Libs\lib3p" -p:Configuration=Debug;Platform=x64

goto lib3p_done
:lib3p_nochanges
echo [LIB3P] No changes detected.
:lib3p_done
echo [LIB3P] Done.

echo [EGLIB] Building...
MSBuild EG.sln %MSBUILDOPTS% -t:"Engine\eglib" -p:Configuration=Release;Platform=x64
MSBuild EG.sln %MSBUILDOPTS% -t:"Engine\eglib" -p:Configuration=Debug;Platform=x64
echo [EGLIB] Done.

echo [EGFOUNDATION] Building...
MSBuild EG.sln %MSBUILDOPTS% -t:"Engine\egfoundation" -p:Configuration=Release;Platform=x64
MSBuild EG.sln %MSBUILDOPTS% -t:"Engine\egfoundation" -p:Configuration=Debug;Platform=x64
echo [EGFOUNDATION] Done.

echo [TOOLS] Building...
MSBuild EG.sln %MSBUILDOPTS% -t:"Engine\Tools\egmake2" -p:Configuration=Release;Platform=x64
rem MSBuild EG.sln %MSBUILDOPTS% -t:"Engine\Tools\EGScc" -p:Configuration=Release;Platform=x64
MSBuild EG.sln %MSBUILDOPTS% -t:"Engine\Tools\EGEdConfig" -p:Configuration=Release;Platform=x64
MSBuild EG.sln %MSBUILDOPTS% -t:"Engine\Tools\EGEdApp" -p:Configuration=Release;Platform=x64
echo [TOOLS] Done.

echo [COPYING CLEANUP FILE]
xcopy "%EGSRC%\core\PreBuild\BinClean.cmd" "%EGOUT%\bin\" /e /y

echo [REFLECTION GENERATOR]
call egmake2_x64.exe RFLGEN -all


echo [PREPARING PLATFORM]
call egmake2_x64.exe PREP_PLATFORM -platform "%EGTARGETPLATFORM%"

goto all_done

:had_pathserror

echo [DATASYNC] Certain environment variables must be set before Emergence can be built.

:had_error
echo [DATASYNC] An error occured in the data sync.
echo [DATASYNC] Please make sure the environment variables EGSRC and EGOUT are set.
echo [DATASYNC] echo EGOUT=%EGOUT%
echo [DATASYNC] echo EGSRC=%EGSRC%
set ERRORLEVEL=-1

:all_done

echo [DATASYNC] Done.
