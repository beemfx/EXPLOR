/******************************************************************************
ServerInterface - The globally accessible server.
(c) 2015 Beem Software
******************************************************************************/
#pragma once

enum SERVER_START_T
{
	SERVER_START_THREAD,
	SERVER_START_THIS_THREAD,
};

void Server_Init( class EGClass* ServerClass, class EGClass* GameClass );
void Server_Deinit();
void Server_Start( SERVER_START_T StartType );
void Server_Stop();
void Server_UpdatePart1( eg_real DeltaTime );
void Server_UpdatePart2( eg_real DeltaTime );
eg_bool Server_IsRunning();
void Server_PostMsg( eg_cpstr StrMsg );
