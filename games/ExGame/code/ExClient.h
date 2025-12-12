// (c) 2016 Beem Media

#pragma once

#include "EGClient.h"
#include "ExHUD.h"
#include "ExDayNightCycles.h"

class ExGame;
class ExMenu;
class ExRosterMenuBase;

class ExClient : public EGClient
{
	EG_CLASS_BODY( ExClient , EGClient )

private:

	ExHUD m_HUD;
	eg_real m_LoadingOpacity = 0.f;
	
public:

	eg_bool m_bWaitingForInitialRepData = false;
	eg_bool m_bIsFullScreenLoading = false;

	EGRemoteEventHandler<ExClient> m_BhHandler;

public:

	EGMCDelegate<eg_real> OnLoadingOpacityChangedDelegate;
	EGMCDelegate<> ProgressSavedDelegate;

public:

	ExClient();
	virtual void Init( const egClientId& Id , EGClass* GameClass ) override final;
	virtual void Deinit() override final;
	virtual void Update( eg_real DeltaTime ) override final;
	virtual void BeginGame() override final;
	virtual void OnMapLoadComplete() override final;
	virtual void OnRemoteEvent( const egRemoteEvent& Event ) override final;
	virtual void Draw( eg_bool bForceSimple ) override final;
	virtual void SetupCamera( class EGClientView& View ) const override;

	void BhRefreshHUD( const eg_event_parms& Parms );
	void BhOpenPauseMenu( const eg_event_parms& Parms );
	void BhOnReturnToMainMenu( const eg_event_parms& Parms );
	void BhOnInitialRepDataComplete( const eg_event_parms& Parms );
	void BhShowFullScreenLoading( const eg_event_parms& Parms );
	void BhQuitFromInn( const eg_event_parms& Parms );
	void BhProgressSaved( const eg_event_parms& Parms );

	const ExGame* GetGameData() const;
	ExGame* GetGameData();
	ExHUD* GetHUD() { return &m_HUD; }

	void PushBgMusic( eg_cpstr Filename );
	void PopBgMusic();

	void HideHUD(){ m_HUD.Hide(); }
	void ShowHUD( eg_bool bImmediatePortraits ){ m_HUD.Show( bImmediatePortraits ); }

	void SetLoadingOpacity( eg_real NewValue );
	eg_real GetLoadingOpacity() const { return m_LoadingOpacity; }

};