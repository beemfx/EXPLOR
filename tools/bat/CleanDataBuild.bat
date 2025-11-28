@echo off
echo [DATABUILD] Cleaning files...
if exist "%EGOUT%/databuild/" @rd /s /q "%EGOUT%/databuild/"
echo [DATABUILD] Done.
