@echo off
rem This will be different depending on the installation of Visual Studio
echo Checking if MSBuild is available
MSBuild /version
if (%ERRORLEVEL%==0) goto BUILD_START
echo MSBuild was not available, attempting to use default installation of Visual Sutdio 2022 Community Edition
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_x64

MSBuild /version
if (%ERRORLEVEL%==0) goto NO_MS_BUILD

:BUILD_START

set EGTARGETPLATFORM=Default
set PATH=.\_BUILD\bin;.\tools\bin;%PATH%

MSBuild EG.sln /nologo -t:restore -p:RestorePackagesConfig=true
MSBuild EG.sln /nologo /v:m -t:"Engine\PreBuild" -p:Configuration=Release;Platform=x64
MSBuild EG.sln /nologo /v:m -t:"Games\ExGame" -p:Configuration=Release;Platform=x64
MSBuild EG.sln /nologo /v:m -t:"Games\ExGame" -p:Configuration=Debug;Platform=x64
MSBuild EG.sln /nologo /v:m -t:"Games\ExGame" -p:Configuration=ReleaseEditor;Platform=x64
MSBuild EG.sln /nologo /v:m -t:"Games\ExGame" -p:Configuration=DebugEditor;Platform=x64
egmake2_x64 DATA
egmake2_x64 CREATE_GAME_INI
egmake2_x64 BUILD_DIST -game "ExGame" -root "_BUILD"
echo Build Complete.
goto DONE

:NO_MS_BUILD
echo MSBuild was not available. The build is not possible.
goto DONE

:DONE

pause
