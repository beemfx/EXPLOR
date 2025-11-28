/******************************************************************************
File: InputCmdBase.h
Purpose: Basic command functionality. Usable by both client and server.

(c) 2011 Beem Software
******************************************************************************/
#pragma once

enum MOUSE_INPUT_M
{
	MOUSE_INPUT_NONE,
	MOUSE_INPUT_CURSOR,
};

typedef eg_uint eg_cmd_t;

static const eg_uint CMD_INTERNAL_START=64;
static const eg_uint CMD_MAX=128;
static const eg_uint CMD_BIT_COUNT=32;
static const eg_uint CMD_ARRAY_SIZE=CMD_MAX/CMD_BIT_COUNT+1;

class ISdkInputRegistrar
{
public:
	virtual void SDK_RegisterInput( eg_cmd_t Cmd , eg_cpstr StrName , eg_cpstr DefaultGpButton , eg_cpstr DefaultKbButton )=0;
};

struct egCmdInfo
{
	eg_uint  IntId;
	eg_cpstr StrId;
	eg_cpstr GpKey;
	eg_cpstr KbKey;
};

// Built in commands
static const eg_cmd_t CMDA_UNK               = 0;
static const eg_cmd_t CMDA_MENU_PRIMARY      = CMD_INTERNAL_START+0;
static const eg_cmd_t CMDA_MENU_PRIMARY2     = CMD_INTERNAL_START+1;
static const eg_cmd_t CMDA_MENU_BACK         = CMD_INTERNAL_START+2;
static const eg_cmd_t CMDA_MENU_BACK2        = CMD_INTERNAL_START+3;
static const eg_cmd_t CMDA_MENU_LMB          = CMD_INTERNAL_START+4;
static const eg_cmd_t CMDA_MENU_RMB          = CMD_INTERNAL_START+5;

static const eg_cmd_t CMDA_MENU_UP           = CMD_INTERNAL_START+6;
static const eg_cmd_t CMDA_MENU_UP2          = CMD_INTERNAL_START+7;
static const eg_cmd_t CMDA_MENU_DOWN         = CMD_INTERNAL_START+8;
static const eg_cmd_t CMDA_MENU_DOWN2        = CMD_INTERNAL_START+9;
static const eg_cmd_t CMDA_MENU_LEFT         = CMD_INTERNAL_START+10;
static const eg_cmd_t CMDA_MENU_LEFT2        = CMD_INTERNAL_START+11;
static const eg_cmd_t CMDA_MENU_RIGHT        = CMD_INTERNAL_START+12;
static const eg_cmd_t CMDA_MENU_RIGHT2       = CMD_INTERNAL_START+13;

static const eg_cmd_t CMDA_MENU_SCRLUP       = CMD_INTERNAL_START+14;
static const eg_cmd_t CMDA_MENU_SCRLDOWN     = CMD_INTERNAL_START+15;
static const eg_cmd_t CMDA_MENU_BTN2         = CMD_INTERNAL_START+16;
static const eg_cmd_t CMDA_MENU_BTN3         = CMD_INTERNAL_START+17;
static const eg_cmd_t CMDA_MENU_NEXT_PAGE    = CMD_INTERNAL_START+18;
static const eg_cmd_t CMDA_MENU_PREV_PAGE    = CMD_INTERNAL_START+19;
static const eg_cmd_t CMDA_MENU_NEXT_SUBPAGE = CMD_INTERNAL_START+20;
static const eg_cmd_t CMDA_MENU_PREV_SUBPAGE = CMD_INTERNAL_START+21;

static const eg_cmd_t CMDA_SYS_CONTOGGLE     = CMD_INTERNAL_START+22;
static const eg_cmd_t CMDA_SYS_CONPAGEUP     = CMD_INTERNAL_START+23;
static const eg_cmd_t CMDA_SYS_CONPAGEDOWN   = CMD_INTERNAL_START+24;
static const eg_cmd_t CMDA_SYS_CONHISTORY    = CMD_INTERNAL_START+25;

struct egLockstepCmds
{
	eg_uint32 m_Active[CMD_ARRAY_SIZE];
	eg_uint32 m_Pressed[CMD_ARRAY_SIZE];
	eg_uint32 m_Released[CMD_ARRAY_SIZE];
	eg_uint32 m_Repeated[CMD_ARRAY_SIZE];
	eg_vec2   m_Axis1; // Right axis/mouse
	eg_vec2   m_Axis2; // Left axis
	eg_vec2   m_MousePos;
	
	static const eg_bool MENU_PRESS_IS_RELEASE = true;	

	eg_real GetMouseX()const
	{
		return m_MousePos.x;
	}

	eg_real GetMouseY()const
	{
		return m_MousePos.y;
	}

	eg_real GetAxisX() const { return m_Axis1.x; }
	eg_real GetAxisY() const { return m_Axis1.y; }

	eg_real GetRightAxisX() const { return m_Axis1.x; }
	eg_real GetRightAxisY() const { return m_Axis1.y; }
	eg_real GetLeftAxisX() const { return m_Axis2.x; }
	eg_real GetLeftAxisY() const { return m_Axis2.y; }

	eg_bool IsActive( const eg_cmd_t cmd )const
	{
		return (m_Active[cmd/CMD_BIT_COUNT]&(1<<(cmd%CMD_BIT_COUNT)))!=0;
	}

	eg_bool WasPressed( const eg_cmd_t cmd )const
	{
		return (m_Pressed[cmd/CMD_BIT_COUNT]&(1<<(cmd%CMD_BIT_COUNT)))!=0;
	}

	eg_bool WasReleased( const eg_cmd_t cmd )const
	{
		return (m_Released[cmd/CMD_BIT_COUNT]&(1<<(cmd%CMD_BIT_COUNT)))!=0;
	}

	eg_bool WasMenuPressed( const eg_cmd_t cmd )const
	{
		return MENU_PRESS_IS_RELEASE ? WasReleased( cmd ) : WasPressed( cmd ); //Could be either released or pressed depending.
	}

	eg_bool WasRepeated( const eg_cmd_t cmd )const
	{
		return (m_Repeated[cmd/CMD_BIT_COUNT]&(1<<(cmd%CMD_BIT_COUNT)))!=0;
	}

	eg_bool WasPressedOrRepeated( const eg_cmd_t cmd ) const
	{
		return WasPressed( cmd ) || WasRepeated( cmd );
	}

	void SetIsActive( const eg_cmd_t cmd , eg_bool Active )
	{
		if( Active )
		{
			m_Active[cmd/CMD_BIT_COUNT] |= (1<<((cmd%CMD_BIT_COUNT)));
		}
		else
		{
			m_Active[cmd/CMD_BIT_COUNT] &= ~(1<<((cmd%CMD_BIT_COUNT)));
		}
	}

	void SetWasPressed( const eg_cmd_t cmd , eg_bool Pressed )
	{
		if( Pressed )
		{
			m_Pressed[cmd/CMD_BIT_COUNT] |= (1<<((cmd%CMD_BIT_COUNT)));
		}
		else
		{
			m_Pressed[cmd/CMD_BIT_COUNT] &= ~(1<<((cmd%CMD_BIT_COUNT)));
		}
	}

	void SetWasReleased( const eg_cmd_t cmd , eg_bool Released )
	{
		if( Released )
		{
			m_Released[cmd/CMD_BIT_COUNT] |= (1<<((cmd%CMD_BIT_COUNT)));
		}
		else
		{
			m_Released[cmd/CMD_BIT_COUNT] &= ~(1<<((cmd%CMD_BIT_COUNT)));
		}
	}

	void SetWasRepeated( const eg_cmd_t cmd , eg_bool Released )
	{
		if( Released )
		{
			m_Repeated[cmd/CMD_BIT_COUNT] |= (1<<((cmd%CMD_BIT_COUNT)));
		}
		else
		{
			m_Repeated[cmd/CMD_BIT_COUNT] &= ~(1<<((cmd%CMD_BIT_COUNT)));
		}
	}

	void CatenateNextFrame( const egLockstepCmds& cmds )
	{
		for(eg_uint i=0; i<CMD_ARRAY_SIZE; i++)
		{
			m_Active[i]    = cmds.m_Active[i];
			//Cumulate button presses:
			m_Pressed[i]  |= cmds.m_Pressed[i];
			m_Released[i] |= cmds.m_Released[i];
			m_Repeated[i] |= cmds.m_Repeated[i];
		}
		//Cumulate mouse axis:
		m_Axis1 += cmds.m_Axis1;
		m_Axis2 += cmds.m_Axis2;
		//Mouse position is just updated:
		m_MousePos = cmds.m_MousePos;
	}

	void ClearPressesAndReleases()
	{
		for(eg_uint i=0; i<CMD_ARRAY_SIZE; i++)
		{
			//Cumulate button presses:
			m_Pressed[i]  = 0;
			m_Released[i] = 0;
			m_Repeated[i] = 0;
		}

		m_Axis1 = CT_Clear;
		m_Axis2 = CT_Clear;
	}

	void CatenateDeviceCmds( const egLockstepCmds& cmds )
	{
		for(eg_uint i=0; i<CMD_ARRAY_SIZE; i++)
		{
			m_Active[i]   |= cmds.m_Active[i];
			//Cumulate button presses:
			m_Pressed[i]  |= cmds.m_Pressed[i];
			m_Released[i] |= cmds.m_Released[i];
			m_Repeated[i] |= cmds.m_Repeated[i];
		}
		//Cumulate mouse axis:
		m_Axis1 += cmds.m_Axis1;
		m_Axis2 += cmds.m_Axis2;
	}

	void ClearButtonState()
	{
		zero( &m_Active );
		zero( &m_Pressed );
		zero( &m_Released );
		zero( &m_Repeated );
	}

	void ClearMouseState()
	{
		m_Axis1 = CT_Clear;
		m_Axis2 = CT_Clear;
		m_MousePos = CT_Clear;
	}

	void Clear()
	{
		ClearButtonState();
		ClearMouseState();
	}
};

