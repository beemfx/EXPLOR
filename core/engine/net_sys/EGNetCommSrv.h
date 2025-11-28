// EGNetCommSrv - An interface for network communication from a server.
// (c) 2015 Beem Software
#pragma once
#include "EGNetCommBase.h"

DECLARE_ID_STRUCT( egNetSrvIntro );
static const egNetSrvIntro NET_SRV_INTRO_NONE = INVALID_ID;

class EGNetCommSrv: public EGNetCommBase
{
public:
	EGNetCommSrv();
	~EGNetCommSrv();

	void    BeginHosting();
	void    EndHosting();
	void    Listen( eg_bool On );
	void    BeginFrame();
	void    EndFrame();
	eg_uint GetConnectedClients( egNetCommId* Out , eg_uint SizeOut )const; //Returns the number of connected clients.
	void    DropConnectedClient( const egNetCommId& Id );
	eg_bool HasOnlineClients()const;
private:
	struct egData;

	egData* m_D;
	eg_byte m_DMem[8720];

private:
	virtual void ProcessOutgoingMsg( const struct egNetMsgData* Data ) override;
	static void HandleRecMsg( const struct egNetMsgData* Data , void* UserPointer );
};
