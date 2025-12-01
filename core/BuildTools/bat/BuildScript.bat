@echo off
rem
rem SETTINGS (Need to be set before calling this)
rem
rem EGSRC:String - Root of depot
rem EGOUT:String - Path to build to
rem EGBUILD_GAMES:String - Space separated list of games to build
rem EGBUILD_VS_VARS_TOOL:String path to visual studio's vcvarsall.bat
rem EGBUILD_CLEAN:Bool - Should the build be cleaned
rem EGBUILD_CLEAN_3RDPARY:Bool - Should the 3rd party libraries be cleaned
rem EGBUILD_BUILD_GAMES:Bool - Should the games be built
rem EGBUILD_CLEAN_BIN_FOLDER:Bool - Should the binaries folder be cleaned of build artifacts
rem EGBUILD_BUILD_DATA:Bool - Should the game data be built
rem EGBUILD_PACKAGE_BINS:Bool - Should the binaries in the game folder be packaged and put on the sync server
rem EGBUILD_BUILD_NUMBER:String - Should be a unique number for the build
rem EGBUILD_SERVER_SYNC_FOLDER:String - The folder to which sync data should be deployed
rem EGBUILD_PLATFORM:String - Target platform (one of EGTargetPlatform.items.h)

echo [EGBUILDSCRIPT] Building Emergence Projects...

echo [EGBUILDSCRIPT] Setting Visual Studio Environment...
call "%EGBUILD_VS_VARS_TOOL%" x86_amd64

setlocal EnableDelayedExpansion
set PATH=%EGOUT%/bin;%EGSRC%/core/BuildTools/bin;%PATH%

echo [EGBUILDSCRIPT] Games set:
for %%i in (%EGBUILD_GAMES%) do echo [EGBUILDSCRIPT] %%i

rem
rem EGBUILD_CLEAN
rem

CD /D "%EGSRC%"

if not "x%EGBUILD_CLEAN%" == "xtrue" goto egbuild_clean_skip

echo [EGBUILDSCRIPT] Cleaning build...
set EGBUILD_GAME_BUILDS=
for %%i in (%EGBUILD_GAMES%) do (set EGBUILD_GAME_BUILDS=!EGBUILD_GAME_BUILDS! /T:"games\%%i:Clean")
rem echo %EGBUILD_GAME_BUILDS%
MSBuild EG.sln %EGBUILD_GAME_BUILDS% /nologo /v:m /p:Configuration=Release;Platform=x64
MSBuild EG.sln %EGBUILD_GAME_BUILDS% /nologo /v:m /p:Configuration=ReleaseEditor;Platform=x64
echo [EGBUILDSCRIPT] Cleaning complete.
goto egbuild_clean_done

:egbuild_clean_skip
echo [EGBUILDSCRIPT] Cleaning build skipped.

:egbuild_clean_done

rem
rem EGBUILD_CLEAN_3RDPARY
rem

CD /D "%EGSRC%"

if not "x%EGBUILD_CLEAN_3RDPARY%" == "xtrue" goto egbuild_clean_3rdparty_skip

echo [EGBUILDSCRIPT] Cleaning 3rd Party libraries...
MSBuild "core/lib3p/lib3p.vcxproj" /T:"Clean" /nologo /v:m /p:Configuration=Release;Platform=x64
echo [EGBUILDSCRIPT] Cleaning complete.
goto egbuild_clean_3rdparty_done

:egbuild_clean_3rdparty_skip
echo [EGBUILDSCRIPT] Cleaning 3rd party build skipped.

:egbuild_clean_3rdparty_done

rem
rem EGBUILD_BUILD_GAMES
rem

CD /D "%EGSRC%"

if not "x%EGBUILD_BUILD_GAMES%" == "xtrue" goto egbuild_build_games_skip

echo [EGBUILDSCRIPT] Building games...
echo [EGBUILDSCRIPT] Building %EGSRC% to %EGOUT%...
set EGBUILD_GAME_BUILDS=
for %%i in (%EGBUILD_GAMES%) do (set EGBUILD_GAME_BUILDS=!EGBUILD_GAME_BUILDS! /T:"games\%%i")
rem echo %EGBUILD_GAME_BUILDS%
MSBuild EG.sln %EGBUILD_GAME_BUILDS% /nologo /v:m /p:Configuration=Release;Platform=x64
MSBuild EG.sln %EGBUILD_GAME_BUILDS% /nologo /v:m /p:Configuration=ReleaseEditor;Platform=x64
echo [EGBUILDSCRIPT] Building complete.
goto egbuild_build_games_done

:egbuild_build_games_skip
echo [EGBUILDSCRIPT] Build games skipped.

:egbuild_build_games_done

rem
rem EGBUILD_CLEAN_BIN_FOLDER
rem

CD /D "%EGSRC%"

if not "x%EGBUILD_CLEAN_BIN_FOLDER%" == "xtrue" goto egbuild_clean_bin_folder_skip

echo [EGBUILDSCRIPT] Cleaning bin folder...
CD /D "%EGOUT%/bin"
CALL BinClean.cmd
DEL BinClean.cmd
rmdir obj /s /q
echo [EGBUILDSCRIPT] Cleaning complete.
goto egbuild_clean_bin_folder_done

:egbuild_clean_bin_folder_skip
echo [EGBUILDSCRIPT] Clean bin folder skipped.

:egbuild_clean_bin_folder_done

rem
rem EGBUILD_BUILD_DATA
rem

CD /D "%EGSRC%"

if not "x%EGBUILD_BUILD_DATA%" == "xtrue" goto egbuild_build_data_skip

echo [EGBUILDSCRIPT] Building game data...
CD /D "%EGOUT%"

egmake2_x64.exe CREATE_GAME_INI

for %%i in (%EGBUILD_GAMES%) do (
egmake2_x64.exe SET_GAME_BUILD %%i
egmake2_x64.exe DATA
)

set BUILD_DIST_PARMS=-includetools

if "x%EGBUILD_NO_TOOLS%" == "xtrue" set BUILD_DIST_PARMS=-notools

echo Building distribution with %BUILD_DIST_PARMS%...

for %%i in (%EGBUILD_GAMES%) do (
egmake2_x64.exe BUILD_DIST -game %%i %BUILD_DIST_PARMS% -root "%EGOUT%"
)

echo [EGBUILDSCRIPT] Gathering binaries...
MKDIR "dist_d"
set ERRORLEVEL=0
XCOPY "%EGOUT%/bin/*.exe" "%EGOUT%/dist_d/" /f /i /r /y
XCOPY "%EGOUT%/bin/*.dll" "%EGOUT%/dist_d/" /f /i /r /y
XCOPY "%EGOUT%/bin/*.ini" "%EGOUT%/dist_d/" /f /i /r /y
set ERRORLEVEL=0

echo [EGBUILDSCRIPT] Gathering game data...
MKDIR "%EGOUT%/dist_d/egdata"
set ERRORLEVEL=0
XCOPY "%EGOUT%/bin/egdata" "%EGOUT%/dist_d/egdata" /f /i /r /y
for %%i in (%EGBUILD_GAMES%) do (
MKDIR "%EGOUT%/dist_d/%%i"
set ERRORLEVEL=0
XCOPY "%EGOUT%/bin/%%i" "%EGOUT%/dist_d/%%i" /f /i /r /y
)

echo [EGBUILDSCRIPT] Data build complete.
goto egbuild_build_data_done

:egbuild_build_data_skip
echo [EGBUILDSCRIPT] Data build skipped.

:egbuild_build_data_done

rem
rem EGBUILD_PACKAGE_BINS
rem

CD /D "%EGSRC%"

if not "x%EGBUILD_PACKAGE_BINS%" == "xtrue" goto eg_build_package_bins_skip

set PATH=%EGOUT%/bin;%PATH%


echo [EGBUILDSCRIPT] Gathering binaries...
CD /D "%EGOUT%"
MKDIR "dist_b"
set ERRORLEVEL=0
XCOPY "%EGOUT%/bin/*.exe" "%EGOUT%/dist_b/" /f /i /r /y
XCOPY "%EGOUT%/bin/*.dll" "%EGOUT%/dist_b/" /f /i /r /y
XCOPY "%EGOUT%/bin/*.ini" "%EGOUT%/dist_b/" /f /i /r /y
set ERRORLEVEL=0

echo [EGBUILDSCRIPT] Packaging binaries...
CD /D "%EGOUT%"
MKDIR "temp"
set ERRORLEVEL=0
set EGBUILD_BIN_PACKAGE_NAME=bin.lpk
egmake2_x64 PACK -source "%EGOUT%/dist_b/" -dest "%EGOUT%/temp/%EGBUILD_BIN_PACKAGE_NAME%"
echo [EGBUILDSCRIPT] Copying file to sync directory (%EGBUILD_SERVER_SYNC_FOLDER%)...
MKDIR "%EGBUILD_SERVER_SYNC_FOLDER%/bin_%EGBUILD_BUILD_NUMBER%"
XCOPY "%EGOUT%/temp/%EGBUILD_BIN_PACKAGE_NAME%" "%EGBUILD_SERVER_SYNC_FOLDER%/bin_%EGBUILD_BUILD_NUMBER%" /f /i /r /y
echo [EGBUILDSCRIPT] Writing package manifest...
echo DistID:%EGBUILD_BUILD_NUMBER%>"%EGBUILD_SERVER_SYNC_FOLDER%/bindistid.html"
XCOPY "%EGOUT%/bin/EGEdConfig_x64.exe" "%EGBUILD_SERVER_SYNC_FOLDER%" /f /i /r /y
echo ^<html^>^<head^>^<title^>Emergence Engine Sync^</title^>^</head^>^<body^>^</body^>^<a href=^"EGEdConfig_x64.exe^"^>EG Editor Config Executable Download^</a^>^</html^>>"%EGBUILD_SERVER_SYNC_FOLDER%/index.html"

echo [EGBUILDSCRIPT] Complete.
goto eg_build_package_bins_done

:eg_build_package_bins_skip
echo [EGBUILDSCRIPT] Package binaries skipped.

:eg_build_package_bins_done

rem
rem EGBUILD_PACKAGE_DIST
rem

CD /D "%EGSRC%"

if not "x%EGBUILD_PACKAGE_DIST%" == "xtrue" goto egbuild_package_dist_skip

echo [EGBUILDSCRIPT] Packaging distribution...

MKDIR "%EGBUILD_SERVER_SYNC_FOLDER%/dist_%EGBUILD_BUILD_NUMBER%"
egmake2_x64.exe BUILDSYNC -in "%EGOUT%/dist_d/" -out "%EGBUILD_SERVER_SYNC_FOLDER%/dist_%EGBUILD_BUILD_NUMBER%"
echo Writing distribution manfiest to %EGBUILD_BUILD_NUMBER%...
echo DistID:%EGBUILD_BUILD_NUMBER%>"%EGBUILD_SERVER_SYNC_FOLDER%/fulldistid.html"

echo [EGBUILDSCRIPT] Package distribution complete.
goto egbuild_package_dist_done

:egbuild_package_dist_skip
echo [EGBUILDSCRIPT] Package distribution skipped.

:egbuild_package_dist_done