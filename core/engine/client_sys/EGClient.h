// (c) 2011 Beem Software

#pragma once

#include "EGList.h"
#include "EGNetCommCli.h"
#include "EGNetMsgBuffer.h"
#include "EGGameMap.h"
#include "EGClientView.h"
#include "EGBoolList.h"
#include "EGQueue.h"
#include "EGTerrainMesh.h"
#include "EGEngineConfig.h"
#include "EGInputBindings.h"
#include "EGInputCmdState.h"

class EGOverlayMgr;
class EGMenuStack;
class EGMenu;
class EGGame;
class EGSoundScape;
class EGDebugSphere;
class EGDebugBox;
class EGWorldSceneGraph;
class EGWorldRenderPipe;
class EGAmbientSoundManager;

// Client state questions (all are true or false)
enum class eg_client_state_q : eg_uint
{
	UNKNOWN,
	IS_CONNECTED,
	HAS_MAP, // true when map is loaded (textures may not be loaded yet)
	IS_SAVING,
	IS_LOADING_THREAD_ACTIVE, // true if anything is being loaded or is queued to be loaded on any thread
};

struct egClientId
{
	eg_uint Id;

	//Only want copy constructor and eg_uint One.
	egClientId() = delete;
	egClientId( const egClientId& rhs ) = default;
	explicit egClientId( eg_uint InId ): Id(InId){ }
	eg_string ToString()const{ return EGString_Format( "%u" , Id ); }

	friend inline eg_bool operator==( const egClientId& lhs , const egClientId& rhs )
	{
		eg_bool IsEqual = lhs.Id == rhs.Id;
		return IsEqual;
	}
};

class EGClient : public EGObject
{
	EG_CLASS_BODY( EGClient , EGObject )

private:

	struct egDebugItems
	{
		EGDebugSphere* DbLight = nullptr;
		EGDebugSphere* DbPortal = nullptr;
		EGDebugSphere* DbGraphVertex = nullptr;
		EGDebugBox* DbBox = nullptr;

		void Init();
		void Deinit();
		void Update( eg_real DeltaTime );
	};

	struct egRayTraceRslt
	{
		eg_phys_group nShapeType;
		eg_ent_id     nHitEnt;
		eg_vec4       vHit;
	};

	struct egDelayedRemoteEvent: public IListable
	{
		egRemoteEvent Event;
	};

	typedef EGList<egDelayedRemoteEvent> EGDelayedEvents;

protected:

	EGOverlayMgr*      m_OverlayMgr;
	EGMenuStack*       m_MenuStack;
	EGOverlayMgr*      m_TopLevelOverlayMgr;
	EGSoundScape*      m_BgScape;
	EGAmbientSoundManager* m_AmbientSoundManager;
	EGClientView       m_ClientView;
	EGClass*           m_GameClass;
	EGEntWorld*        m_EntWorld;
	EGWorldRenderPipe* m_WorldRenderPipe;
	egClientId         m_ClientId;
	eg_lockstep_id     m_LockstepId = INVALID_ID;
	eg_input_device_id m_InputDeviceId;
	EGNetCommCli       m_NetComm;
	EGNetMsgBuffer     m_NetMsgBuffer;
	eg_real            m_fTimeSinceLastComm;
	eg_real            m_TimeSinceLastLockstep;
	eg_real            m_NetRate; //Net rate is computed by how often the server asks for input.
	eg_real            m_InputRate;
	eg_uint            m_InputFrame;
	eg_real            m_InputTime;
	eg_real            m_DeltaTime;
	eg_real            m_ClientTime;
	EGDelayedEvents    m_DelayedEvents;
	EGInputBindings    m_InputBindings;
	EGInputCmdState    m_InputCmdState;
	egLockstepCmds     m_RemoteInputBuffer;
	egLockstepCmds     m_LocalInputBuffer;
	eg_real            m_TimeSinceLockstep;
	MOUSE_INPUT_M      m_MouseInputMode;
	egDebugItems       m_DbItems;
	egNetCommId        m_DbNetId; //NetId used only for debugging, not message filtering or anything like that.
	eg_uint64          m_Ping1SendTime;
	eg_uint            m_Ping1RequestId;
	eg_vec2            m_LastMousePos;
	eg_real            m_LastAspectRatio;
	eg_real            m_OneMinuteCycle;
	eg_bool            m_WantPing:1;

public:

	EGClient();
	~EGClient();

	virtual void Init( const egClientId& Id , EGClass* GameClass );
	virtual void Deinit();
	virtual void Update( eg_real DeltaTime );
	virtual void Draw( eg_bool bForceSimple );

	virtual void BeginGame(){ }
	virtual void OnMapLoadComplete(){ }
	virtual void OnConnected(){ }
	virtual void OnDisconnected(){ }

	virtual void SetupCamera( class EGClientView& View ) const;

	virtual void OnRemoteEvent( const egRemoteEvent& Event );

	void    SetViewport( const egScreenViewport& Vp , eg_real GameAspectRatio ){ m_ClientView.SetViewport( Vp , GameAspectRatio ); }
	void    Connect( eg_cpstr strServer );
	void    Disconnect();
	void    SetInputDevice( eg_input_device_id NewInputDevice );
	eg_bool UseGamepadHints() const{ return m_InputDeviceId != eg_input_device_id::KBM; }
	eg_bool WantsMouseCursor()const;
	void    SendCmd( eg_cpstr Cmd );
	eg_bool IsConnected()const{ return EGNetCommCli::CONN_NOT_CONNECTED != m_NetComm.GetConnectionStatus(); }
	egClientId GetId() const { return m_ClientId; }
	void BindInput( const class EGInput& Input , eg_cpstr StrAction , eg_cpstr StrGp , eg_cpstr StrKb );
	const EGInputBindings& GetInputBindings() const{ return m_InputBindings; }
	EGInputBindings& GetInputBindings() { return m_InputBindings; }
	EGInputCmdState& GetInputCmdState(){ return m_InputCmdState; }

	EGEntWorld* SDK_GetWorld() { return m_EntWorld; }
	const EGEntWorld* SDK_GetWorld() const { return m_EntWorld; }
	void SDK_ClearAllOverlays();
	EGMenu* SDK_AddOverlay( eg_string_crc OverlayId , eg_real DrawPriority , eg_bool bTopLevel = false );
	void SDK_RemoveOverlay( EGMenu* Overlay );
	void SDK_RunServerEvent( const egRemoteEvent& Event );
	void SDK_RunClientEvent( const egRemoteEvent& Event );
	eg_bool SDK_AskStateQuestion( eg_client_state_q Question );
	EGGame* SDK_GetGame();
	const EGGame* SDK_GetGame()const;
	void SDK_ClearMenuStack();
	EGMenu* SDK_PushMenu( eg_string_crc MenuId );
	void SDK_PlayBgMusic( eg_cpstr Filename );
	void SDK_SetShowMouse( eg_bool bShowMouse );
	void SDK_GetLockstepCmds( egLockstepCmds* Cmds ) const;
	const EGGameMap* SDK_GetMap() const;
	EGEnt* SDK_GetEnt( eg_ent_id Id );
	const EGEnt* SDK_GetEnt( eg_ent_id Id ) const;
	EGSoundScape* SDK_GetSoundScape() { return m_BgScape; }
	EGMenuStack* SDK_GetMenuStack() { return m_MenuStack; }
	const eg_lockstep_id& GetLockstepId() const { return m_LockstepId; }
	EGWorldRenderPipe* GetWorldRenderPipe() const { return m_WorldRenderPipe; }

private:

	EGEnt* GetEnt( eg_ent_id Id );
	const EGEnt* GetEnt( eg_ent_id Id ) const;

	void Draw_UI();
	void Draw_DebugItems();
	void Draw_DebugLights();
	void Draw_DebugZOverlay();

	void Init_DebugItems(eg_uint bInit);
	void FinalizeLoad(); //Called after the map gets loaded by the loading thread.
	
	void Update_Camera( eg_real DeltaTime );
	void Update_BroadCastInput( eg_real DeltaTime );
	
	void Net_BeginFrame();
	void Net_EndFrame();
	static void Net_PumpMsgs( const egNetMsgData* Data , void* UserPointer );
	static void Net_OnReceivedMsg( const egNetMsgData* Data , void* UserPointer );
	void OnNetMessage(eg_net_msg msg, const egNetMsgData& bc);
	void SM_Welcome( const egNetCommId& Id , eg_cpstr GameId , const eg_lockstep_id& LockstepId );
	void SM_Lockstep( const egNetMsgData& bc );
	void SM_StrCmd( const egNetMsgData& bc );

	void OnPrimaryMapChanged( EGGameMap* GameMap );
	void OnMapLoaded( EGGameMap* GameMap );
	void OnTerrainLoaded( EGTerrain2* GameTerrain );
};
