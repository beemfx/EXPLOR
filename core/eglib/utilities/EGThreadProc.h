/******************************************************************************
EGThreadProc - A Thread Process is run on a specific thread.

(c) 2015 Beem Software
******************************************************************************/

#pragma once
#include "EGList.h"

class IThreadProc: public IListable
{
public:
	virtual void OnStart(){} // Called when the process is started.
	virtual void Update( eg_real DeltaTime ){ unused( DeltaTime ); } // Repeatedly called until the thread ends.
	virtual void OnStop(){ } // Called just before the thread ends, no more calls to Update will be made.
	virtual void OnThreadMsg(eg_cpstr strParm){ unused( strParm ); } // If a message has been queued for the thread this is called just before Update.
};