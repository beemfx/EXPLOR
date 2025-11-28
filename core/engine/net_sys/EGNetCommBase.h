// EGNetComm2 - An interface for network communication between devices.
// (c) 2015 Beem Software
#pragma once

struct egNetCommId;

//
// EGNetCommBase - Shared functionality between EGNetCommSrv and EGNetCommCli
//

class EGNetCommBase
{
public:
	EGNetCommBase();
	~EGNetCommBase();

	void Init();
	void Deinit();

	void BeginFrame();
	void EndFrame();
	void PostMsg( const struct egNetMsgData* Data );
	void ProcessReceivedMsgs( void ( * Callback )( const struct egNetMsgData* Data , void* UserPointer ) , void* UserPointer );

protected:
	void PostRecMsg( const struct egNetMsgData* Data );
	virtual void ProcessOutgoingMsg( const struct egNetMsgData* Data )=0;
private:
	struct egNetComm2Data;

	egNetComm2Data* m_DN2;
	eg_byte         m_DN2Mem[320];
private:
	static void ProcessOutgoingMsgs( const struct egNetMsgData* Data , void* UserPointer );
};