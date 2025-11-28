// EGNetCommCli - An interface for network communication from a client.
// (c) 2015 Beem Software
#pragma once
#include "EGNetCommBase.h"

class EGNetCommCli: public EGNetCommBase
{
public:
	enum CONN_S
	{
		CONN_NOT_CONNECTED,
		CONN_CONNECTING,
		CONN_CONNECTED,
	};

public:
	EGNetCommCli();
	~EGNetCommCli();

	void    Connect( eg_cpstr Address );
	void    Disconnect();
	CONN_S  GetConnectionStatus()const;
	void    BeginFrame();
	void    EndFrame();
	void    PostMsg( const struct egNetMsgData* Data );
	eg_bool IsLocal()const;

private:
	struct egData;

	egData* m_D;
	eg_byte m_DMem[496];

private:
	virtual void ProcessOutgoingMsg( const struct egNetMsgData* Data ) override;
	static void HandleRecMsg( const struct egNetMsgData* Data , void* UserPointer );
};