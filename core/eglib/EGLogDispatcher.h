// (c) 2017 Beem Media

#pragma once

#include "EGCircleArray.h"
#include "EGDelegate.h"

typedef EGMCDelegate<eg_log_channel,const eg_string_base&> EGLogDelegate;

class EGLogDispatcher
{
public:

	typedef void (EGObject::*egCallbackFn)(eg_log_channel,const eg_string_base&);

private:

	struct egLogEntry
	{
		eg_string_crc Channel = CT_Clear;
		eg_string_big LogText = CT_Clear;

		egLogEntry() = default;
		egLogEntry( eg_log_channel InChannel , eg_cpstr InLogText ): Channel( InChannel.GetId() ) , LogText( InLogText ) { }
	};

private:

	EGMutex m_Lock;
	EGCircleArray<egLogEntry,200> m_Queue;

public:

	EGLogDelegate Delegate;

public:

	void QueueLog( eg_log_channel Channel , eg_cpstr LogText );
	void DispatchLogs();
	void CopyTo( EGLogDispatcher& Dest );
};
