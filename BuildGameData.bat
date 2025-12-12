@echo off

set EG_GAME_BUILD=ExGame

set EGSRC=%cd%
set EGOUT=%cd%\_BUILD\
set EGGAME=%EG_GAME_BUILD%
set EGTARGETPLATFORM=Default
set PATH=.\_BUILD\bin;.\core\BuildTools\bin;%PATH%

egmake2_x64 DATA
egmake2_x64 CREATE_GAME_INI

pause
