// EGThreadRequest - A system for making requests on a thread.
// (c) 2015 Beem Software
#pragma once
#include "EGThreadProc.h"
#include "EGList.h"

class IRequestCallback
{
public:
	virtual void Action() = 0;   //Called on the thread.
	virtual void Callback() = 0; // Called from EGThreadRequest::ProcessCompletedRequests.
};

class EGThreadRequest : public IThreadProc
{
public:

	EGThreadRequest( eg_cpstr strId );
	~EGThreadRequest();

public:

	void MakeRequest( IRequestCallback* L );
	void CancelRequest( IRequestCallback* L ); //CancelRequest only gaurantees that upon return the request will not be in the request Que, it is possible that the request was completed though.
	//Called only within the thread that owns the thread request:
	void ProcessCompletedRequests();

protected:

	void PurgeRequests(); //Forces callback on all requests (Action will not be peformed, though it may have already been called), should be called from the owning thread..

private:

	virtual void Update( eg_real DeltaTime ) override final;
	virtual void OnThreadMsg(eg_cpstr /*strParm*/) override {assert(false);};

private:

	struct egRequestData: public IListable
	{
		IRequestCallback* Request;
	};

	typedef EGList<egRequestData> EGLoadQue;

	EGMutex m_LockAction;
	EGMutex m_LockQue;
	EGMutex m_LockUpdate;

	//Every item is always in one of these ques at all times.
	EGLoadQue     m_LoadQue;
	egRequestData*m_LoadItem;
	EGLoadQue     m_LoadCompleteQue;
	eg_real       m_SleepTimeWhenNoProc;

private:
	eg_bool RemoveFromQueue( IRequestCallback* L , EGLoadQue* Que );
};