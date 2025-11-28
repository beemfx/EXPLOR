// (c) 2016 Beem Media

#pragma once

class EGInput;
class EGClient;

class EGEngineInst : public EGObject
{
	EG_CLASS_BODY( EGEngineInst , EGObject )

private:

	eg_string_small m_GameName = CT_Clear;
	eg_d_string16   m_ExeDir = CT_Clear;
	eg_d_string16   m_UserDir = CT_Clear;
	eg_d_string16   m_SysCfgDir = CT_Clear;
	eg_bool         m_bIsTool = false;
	eg_bool         m_bIsServerOnly = false;

public:

	enum class eg_game_event
	{
		INIT, //Happens during game splash screen this is where any always loaded data should be loaded.
		DEINIT, // Clean up for anything in GAME_EVENT_INIT
	};

public:

	virtual void OnGameEvent( eg_game_event Event ){ unused( Event ); }
	virtual void OnRegisterInput( class ISdkInputRegistrar* Registrar ){ unused( Registrar ); }
	virtual void SetupViewports( eg_real ScreenAspect, egScreenViewport* aVpOut, eg_uint VpCount )const{ unused( ScreenAspect , aVpOut , VpCount ); }

	void SetIsTool( eg_bool bIsToolIn ) { m_bIsTool = bIsToolIn; }
	eg_bool IsTool() const { return m_bIsTool; }
	void SetIsServerOnly( eg_bool bIsServerOnlyIn ) { m_bIsServerOnly = bIsServerOnlyIn; }
	eg_bool IsServerOnly() const { return m_bIsServerOnly; }
	void SetGameName( eg_cpstr GameNameIn ) { m_GameName = GameNameIn; }
	eg_cpstr GetGameName() const { return m_GameName; }
	void SetPlatformExeDir( eg_cpstr16 ExeDirIn ) { m_ExeDir = ExeDirIn; }
	eg_cpstr16 GetPlatformExeDir() const { return *m_ExeDir; }
	void SetPlatformUserDir( eg_cpstr16 UserDirIn ) { m_UserDir = UserDirIn; }
	eg_cpstr16 GetPlatformUserDir() const { return *m_UserDir; }
	void SetPlatformSysCfgDir( eg_cpstr16 DirIn ) { m_SysCfgDir = DirIn; }
	eg_cpstr16 GetPlatformSysCfgDir() const { return *m_SysCfgDir; }
	EGInput& GetInput();
	void GetClients( EGArray<EGClient*>& ClientsOut );
};
