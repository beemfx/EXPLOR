// EGInputCmdMgr (c) 2016 Beem Media
#pragma once
#include "EGInputTypes.h"
#include "EGEngineConfig.h"

enum class eg_input_device_id : eg_uint
{
	NONE,
	GP1,
	GP2,
	GP3,
	GP4,
	KBM,
	COUNT,
};

class EGInputCmdState
{
public:
	EGInputCmdState();
	~EGInputCmdState();
	void Init();
	eg_bool IsActive( eg_cmd_t Action )const;
	eg_bool IsPressed( eg_cmd_t Action)const;
	eg_bool IsReleased( eg_cmd_t Action )const;
	eg_bool IsWillDeactivate( eg_cmd_t Action )const;
	eg_bool IsRepeated( eg_cmd_t Action )const;
	void SetActive( eg_cmd_t Action , eg_bool On );
	void SetPressed( eg_cmd_t Action , eg_bool On );
	void SetReleased( eg_cmd_t Action , eg_bool On );
	void SetWillDeactivate( eg_cmd_t Action , eg_bool On );
	void UpdateRepeat( eg_cmd_t Action , eg_real DeltaTime );
private:
	EGBoolList<CMD_MAX> m_Active;
	EGBoolList<CMD_MAX> m_Pressed;
	EGBoolList<CMD_MAX> m_Released;
	EGBoolList<CMD_MAX> m_Repeated;
	EGBoolList<CMD_MAX> m_WillDeactivate;
	eg_real             m_TimeSinceLastRepeat[CMD_MAX];
};

#include "EGInputCmdState.hpp"