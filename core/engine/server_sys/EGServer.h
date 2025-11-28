// (c) 2011 Beem Software

#pragma once

#include "EGNetMsgBuffer.h"
#include "EGInputTypes.h"
#include "EGList.h"
#include "EGGameLoader.h"
#include "EGNetCommSrv.h"
#include "EGBoolList.h"
#include "EGQueue.h"
#include "EGTimer.h"
#include "EGCircleArray.h"
#include "EGEnt.h"
#include "EGEntTypes.h"
#include "EGGameTypes.h"

class EGGame;
class EGEntWorld;

class EGServer : public EGObject
{
	EG_CLASS_BODY( EGServer , EGObject )

public:
	EGServer();
	~EGServer();
	
	//Initialization is either called from the owning thread.
	void InitGameClass( EGClass* GameClass ){ m_GameClass = GameClass; }
	void Init();
	virtual void Update( eg_real DeltaTime );
	void Deinit();
	void QueMsg( eg_cpstr Msg );

	virtual void OnRemoteEvent( const eg_lockstep_id& SenderId, const egRemoteEvent& Event );
	virtual void OnConsoleCmd( eg_cpstr Cmd ){ unused( Cmd ); }
//
// Structs and consts
//
private:

	static const eg_uint PENDING_INPUT_SIZE=2; //No pending input frames

	typedef EGCircleArray<egLockstepCmds,PENDING_INPUT_SIZE> egLsArray;

	struct egClientInfo
	{
		egNetCommId    NetCommId;
		eg_lockstep_id LockstepId;
		egLsArray      InCmds;
		eg_uint        FramesWithoutInput;
		eg_uint        ExcessInputs;
		eg_uint        nLastComm; //The last time the communication was recived from this client.
		eg_real        PingTimer; //A timer so we can ping the clients every few seconds.
		eg_vec3        vRayOrg;    // RayTraceRequest:
		eg_vec3        vRayDir;    // "             " 
		eg_bool        bPingedWaiting:1;
		eg_bool        bSwept:1;

		eg_bool IsConnected()const{ return NET_ID_NOT_CONNECTED != NetCommId; }

		void Clear()
		{
			NetCommId = INVALID_ID;
			LockstepId = INVALID_ID;
			InCmds.Clear();
			FramesWithoutInput = 0;
			ExcessInputs = 0;
			nLastComm = 0;
			PingTimer = 0.f;
			vRayOrg = eg_vec3(CT_Clear);
			vRayDir = eg_vec3(CT_Clear);
			bPingedWaiting = false;
			bSwept = false;
		}
	};

	static const eg_uint MAX_MESSAGES = 128;
	typedef EGQueue<eg_string , MAX_MESSAGES> EGMsgStack;

	//Server commands:
	typedef void ( * SRV_CMD )( EGServer* _this , const struct egParseFuncInfo* Info );
	struct egSrvCmdInfo
	{
		SRV_CMD  FnCall;
		eg_cpstr FnName;
		eg_int   NumParms;
		eg_cpstr ParmList;
		eg_cpstr ParmHelp;
	};
	typedef EGStringCrcMapFixedSize< egSrvCmdInfo , 10 > EGServerCmd;

protected:

	//
	// Server State
	//
	EGClass*       m_GameClass = nullptr;
	EGEntWorld*    m_EntWorld = nullptr;
	EGTimer        m_Timer; //This timer is only used for synchronizing the speed of the server, not for AI updates.
	eg_real        m_DeltaTime; //The time step for the current frame.
	EGMutex        m_MsgLock;
	EGMsgStack     m_MsgStack;
	eg_uint        m_nGameTime; //Game time since map was loaded (or time of save if save was loaded).
	eg_uint        m_LockstepFrame;
	eg_real        m_UpdateEntRate;
	eg_real        m_TimeSinceLastEntSend;
	egClientInfo   m_Clients[MAX_CLIENTS]; //List of clients, in use if Flags contains CI_F_ISCONNECTED.
	//Loading info:
	eg_string      m_LoadSaveFilename;
	EGGameLoader*  m_GameLoader;
	//Server commands:
	egSrvCmdInfo   m_SrvCmdDefault;
	EGServerCmd    m_Cmds;

	EGNetCommSrv       m_NetComm;
	EGNetMsgBuffer     m_NetMsgBuffer;
	eg_string          m_NextMapToLoad;
	eg_bool            m_bHasMapToLoad;

///////////////////////////////////
/// Server thread functionality ///
///////////////////////////////////
private:	
	void Update_ClientInfo();
	void Update_PingClients();
	void Update_ProcessMsgs();
	void HandleMsg( eg_cpstr sMsg );

	void BeginLoad(eg_cpstr strFile);
	void BeginLoad_Finalize();
	void DoSave(eg_cpstr strFile);
	void ResetLoad();

	EGEnt* SpawnEnt( const eg_string_crc& EntDef , const eg_transform& Pose = CT_Default , eg_cpstr InitString = "" );
	void SpawnMapEnts();
	void DestroyEnt(eg_ent_id EntId);
	
	egClientInfo* GetClientInfo( const egNetCommId& nID );
	void          ConnectClient( const egNetCommId& NetId );

public:

	EGEntWorld* SDK_GetWorld() { return m_EntWorld; }
	const EGEntWorld* SDK_GetWorld() const { return m_EntWorld; }
	void         SDK_PostCmd(eg_cpstr sCmd);
	void         SDK_GetClientMouseTarget( eg_lockstep_id LockStepId , eg_vec3& OrgOut , eg_vec3& DirOut ) const;
	void         SDK_GetCommandsForPlayer( eg_lockstep_id LockstepId , egLockstepCmds* Out ) const;
	EGEnt*       SDK_GetEnt(eg_ent_id nUEID);
	EGEnt*       SDK_SpawnEnt( const eg_string_crc& EntDef , const eg_transform& Pose = CT_Default , eg_cpstr InitString = "" );
	void         SDK_DestroyEnt( eg_ent_id EntId );
	void         SDK_RayCast( const eg_vec4& Position , const eg_vec4& Direction , egRaycastInfo* OutInfo );
	void         SDK_RayCastFromEnt( const eg_ent_id EntId , const eg_vec4& Direction , egRaycastInfo* OutInfo ) const;
	void         SDK_RunClientEvent( const egRemoteEvent& Event );
	void         SDK_RunServerEvent( const eg_lockstep_id& SrcLockstepId , const egRemoteEvent& Event );
	void         SDK_SaveGameState( eg_cpstr Filename );
	void         SDK_ClearGlobalData();
	void         SDK_ResetWorld();
	void         SDK_LoadLevel( eg_cpstr Filename, bool PrependUserPath = false );
	EGGame*      SDK_GetGame();
	virtual void SDK_LoadWorld( eg_cpstr Filename );

private:

	EGEnt* GetEnt( eg_ent_id Id )const;

//
// Server Commands
//
private:
	void SrvCmd_Init();
	static void SrvCmd_ShowCmds( EGServer* _this , const struct egParseFuncInfo* Info );
	static void SrvCmd_Drop( EGServer* _this , const struct egParseFuncInfo* Info );
	static void SrvCmd_Load( EGServer* _this , const struct egParseFuncInfo* Info );
	static void SrvCmd_LoadWorld( EGServer* _this , const struct egParseFuncInfo* Info );
	static void SrvCmd_Spawn( EGServer* _this , const struct egParseFuncInfo* Info );
	static void SrvCmd_Kill( EGServer* _this , const struct egParseFuncInfo* Info );
	static void SrvCmd_MapOp( EGServer* _this , const struct egParseFuncInfo* Info );
	static void SrvCmd_ShowInfo( EGServer* _this , const struct egParseFuncInfo* Info );
	static void SrvCmd_Listen( EGServer* _this , const struct egParseFuncInfo* Info );

//Net message handling:
private:
	void Net_BeginFrame();
	void Net_EndFrame();
	void Net_EnumerateClients();
	void Net_BroadcastDirtyEnts();
	static void Net_PumpMsgs( const egNetMsgData* Data , void* UserPointer );
	static void Net_OnReceivedMsg( const egNetMsgData* Data , void* UserPointer );
	void OnNetMessage(eg_net_msg msg, const egNetMsgData& bc);
	void CM_Lockstep( const egNetMsgData& bcs );
	void CM_StrCmd( const egNetMsgData& bc );
	void CM_RemoteEvent( const egNetMsgData& bc );
};
