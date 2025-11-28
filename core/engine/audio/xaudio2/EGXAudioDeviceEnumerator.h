// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "EGWeakPtr.h"
#include <mmdeviceapi.h>
#undef GetObject

class EGXAudioDeviceEnumerator : private IMMNotificationClient
{
private:

	ULONG m_NotifyRefs = 0;
	IMMDeviceEnumerator* m_DeviceEnumerator = nullptr;
	EGWeakPtr<EGObject> m_Owner = nullptr;
	eg_s_string_sml16 m_LastKnownDevice;

public:

	EGXAudioDeviceEnumerator( EGObject* OwnerObject );
	~EGXAudioDeviceEnumerator();

private:

	// BEGIN IMMNotificationClient
	virtual HRESULT OnDeviceStateChanged( LPCWSTR pwstrDeviceId , DWORD dwNewState ) override { unused( pwstrDeviceId , dwNewState ); return S_OK; }
	virtual HRESULT OnDeviceAdded( LPCWSTR pwstrDeviceId ) override { unused( pwstrDeviceId ); return S_OK; }
	virtual HRESULT OnDeviceRemoved( LPCWSTR pwstrDeviceId ) override { unused( pwstrDeviceId ); return S_OK; }
	virtual HRESULT OnDefaultDeviceChanged( EDataFlow flow , ERole role , LPCWSTR pwstrDefaultDeviceId ) override;
	virtual HRESULT OnPropertyValueChanged( LPCWSTR pwstrDeviceId , const PROPERTYKEY key ) override { unused( pwstrDeviceId , key ); return S_OK; }
	virtual HRESULT QueryInterface(const IID & , void **) override { return S_OK; }
	virtual ULONG AddRef() override	{ m_NotifyRefs++; return m_NotifyRefs; }
	virtual ULONG Release() override { m_NotifyRefs--; return m_NotifyRefs; }
	// END IMMNotificationClient
};
