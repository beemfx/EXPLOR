// (c) 2016 Beem Media

#include "EGEngineInst.h"
#include "EGInput.h"
#include "EGEngine.h"

EG_CLASS_DECL( EGEngineInst )

EGInput& EGEngineInst::GetInput()
{
	return EGInput::Get();
}

void EGEngineInst::GetClients( EGArray<EGClient*>& ClientsOut )
{
	EGEngine_GetClients( ClientsOut );
}
