// ConDraw - A module for drawing the contents of MainConsole.
#pragma once

void ConDraw_Init( void );
void ConDraw_Deinit( void );
void ConDraw_ToggleActive( void );
bool ConDraw_IsActive( void );
void ConDraw_Update( eg_real DeltaTime , const struct egLockstepCmds* Cmds );
void ConDraw_OnChar( eg_char c );
void ConDraw_Draw(  eg_real AspectRatio  );
