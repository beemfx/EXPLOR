// (c) 2016 Beem Media. All Rights Reserved.

#pragma once

class ExClient;
class ExMenu;

class ExHUD
{
private:

	ExClient*const m_Owner;
	ExMenu*   m_CharacterPortraits = nullptr;
	ExMenu*   m_Automap = nullptr;
	ExMenu*   m_Compass = nullptr;
	ExMenu*   m_BaseHud = nullptr;
	ExMenu*   m_ProgressSavedOverlay = nullptr;
	eg_int    m_HideCount = 0;

public:

	ExHUD( ExClient* Owner );
	~ExHUD();

	void InitClientComponents();
	void DeinitClientComponents();
	void InitGameComponents();
	void DeinitGameComponents();
	void Update( eg_real DeltaTime );
	void Refresh();
	void Hide();
	void Show( eg_bool bImmediatePortraits );

private:

	void SetWidgetVisible( ExMenu* Widget , eg_bool bVisible , eg_bool bAnimate );
};