// (c) 2020 Beem Media. All Rights Reserved.

#include "EGXAudioDeviceEnumerator.h"
#include "EGXAudioDevice.h"

EGXAudioDeviceEnumerator::EGXAudioDeviceEnumerator( EGObject* OwnerObject )
: m_Owner( OwnerObject )
{
	m_NotifyRefs = 1;
	HRESULT Result = CoCreateInstance( __uuidof(MMDeviceEnumerator) , nullptr , CLSCTX_INPROC_SERVER , __uuidof(IMMDeviceEnumerator) , reinterpret_cast<void**>(&m_DeviceEnumerator) );
	if( Result == S_OK )
	{
		m_DeviceEnumerator->RegisterEndpointNotificationCallback(this);
	}
}

EGXAudioDeviceEnumerator::~EGXAudioDeviceEnumerator()
{
	if( m_DeviceEnumerator )
	{
		m_DeviceEnumerator->UnregisterEndpointNotificationCallback(this);
		m_DeviceEnumerator->Release();
		m_DeviceEnumerator = nullptr;
	}
}

HRESULT EGXAudioDeviceEnumerator::OnDefaultDeviceChanged( EDataFlow flow , ERole role , LPCWSTR pwstrDefaultDeviceId )
{
	if( flow == EDataFlow::eRender || flow == EDataFlow::eAll )
	{
		if( role == ERole::eConsole )
		{
			eg_s_string_sml16 NewDevice = pwstrDefaultDeviceId;
			if( m_LastKnownDevice != NewDevice )
			{
				m_LastKnownDevice = NewDevice;
				EGLogf( eg_log_t::Audio , "Default audio device changed to %s" , *eg_d_string8( pwstrDefaultDeviceId ) );
				EGXAudioDevice* AudioDevice = EGCast<EGXAudioDevice>( m_Owner.GetObject() );
				if( AudioDevice )
				{
					AudioDevice->HandleDefaultDeviceChanged();
				}
			}
		}
	}
	return S_OK;
}
