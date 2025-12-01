@echo off
echo [PreBuild] Cleaning files...

CD /D %~1%
set EGSRC=%~1%
set EGOUT=%~1%_BUILD\

rem if not exist "%EGOUT%/" goto clean_done
cd %EGOUT%
for /d %%i in ("*") do if /i not "%%~nxi"=="lib3p_build" rd /s /q "%%i"
:clean_done
echo [PreBuild] lib3p builds are not cleaned, if they need to be cleaned they should be deleted manually.
echo [PreBuild] Done (lib3p)