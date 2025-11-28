set EGSRC=%cd%
set EGOUT=%cd%\_BUILD\
set EGGAME=ExGame
set EGTARGETPLATFORM=Default
set PATH=.\_BUILD\bin;.\tools\bin;%PATH%

egmake2_x64 DATA
egmake2_x64 CREATE_GAME_INI

pause
