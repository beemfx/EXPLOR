// (c) 2016 Beem Media
#pragma once

class EGKbCharHandler
{
public:
	class IListener
	{
	public:
		virtual eg_bool HandleTypedChar( eg_char16 Char )=0;
	};

public:
	static EGKbCharHandler& Get(){ return s_Instance; }

	EGKbCharHandler(): m_Listeners( CT_Clear ){ }

	void AddListener( IListener* Listener , eg_bool bBlockOtherListeners );
	void RemoveListener( IListener* Listener );

	eg_bool ProcessChar( eg_char16 Char );

private:
	
	struct egListenerInfo
	{
		IListener* Listener;
		eg_bool    bBlockOtherListeners:1;
	};

	EGFixedArray<egListenerInfo,4> m_Listeners;

	static EGKbCharHandler s_Instance;
};