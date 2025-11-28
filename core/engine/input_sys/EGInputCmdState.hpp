// EGInputCmdState (c) 2016 Beem Media

#define EGINPUTCMDSTATE_INLINE __forceinline

EGINPUTCMDSTATE_INLINE EGInputCmdState::EGInputCmdState()
	: m_Active( CT_Clear )
	, m_Pressed( CT_Clear )
	, m_Released( CT_Clear )
	, m_WillDeactivate( CT_Clear )
	, m_Repeated( CT_Clear )
{
	zero( &m_TimeSinceLastRepeat );
}

EGINPUTCMDSTATE_INLINE EGInputCmdState::~EGInputCmdState()
{
}

EGINPUTCMDSTATE_INLINE void EGInputCmdState::Init()
{

}

EGINPUTCMDSTATE_INLINE eg_bool EGInputCmdState::IsActive( eg_cmd_t Action )const
{
	return m_Active.IsSet( Action );
}

EGINPUTCMDSTATE_INLINE eg_bool EGInputCmdState::IsPressed( eg_cmd_t Action )const
{
	return m_Pressed.IsSet( Action );
}

EGINPUTCMDSTATE_INLINE eg_bool EGInputCmdState::IsReleased( eg_cmd_t Action )const
{
	return m_Released.IsSet( Action );
}

EGINPUTCMDSTATE_INLINE eg_bool EGInputCmdState::IsWillDeactivate( eg_cmd_t Action )const
{
	return m_WillDeactivate.IsSet( Action );
}

EGINPUTCMDSTATE_INLINE eg_bool EGInputCmdState::IsRepeated( eg_cmd_t Action ) const
{
	return m_Repeated.IsSet( Action );
}

EGINPUTCMDSTATE_INLINE void EGInputCmdState::SetActive( eg_cmd_t Action , eg_bool On )
{
	if( On )
	{
		m_Active.Set( Action );
	}
	else
	{
		m_Active.Unset( Action );
	}
}

EGINPUTCMDSTATE_INLINE void EGInputCmdState::SetPressed( eg_cmd_t Action , eg_bool On )
{
	if( On )
	{
		m_TimeSinceLastRepeat[Action] = 0.f;
		m_Pressed.Set( Action );
	}
	else
	{
		m_Pressed.Unset( Action );
	}
}

EGINPUTCMDSTATE_INLINE void EGInputCmdState::SetReleased( eg_cmd_t Action , eg_bool On )
{
	if( On )
	{
		m_Released.Set( Action );
	}
	else
	{
		m_Released.Unset( Action );
	}
}

EGINPUTCMDSTATE_INLINE void EGInputCmdState::SetWillDeactivate( eg_cmd_t Action , eg_bool On )
{
	if( On )
	{
		m_WillDeactivate.Set( Action );
	}
	else
	{
		m_WillDeactivate.Unset( Action );
	}
}

EGINPUTCMDSTATE_INLINE void EGInputCmdState::UpdateRepeat( eg_cmd_t Action, eg_real DeltaTime )
{
	m_TimeSinceLastRepeat[Action] += DeltaTime;

	const eg_real RepeatTime = INPUT_PRESS_REPEAT_FIRST_DELAY;
	const eg_real RepeatRestart = RepeatTime - INPUT_PRESS_REPEAT_TIME;

	if( IsActive( Action ) && m_TimeSinceLastRepeat[Action] > RepeatTime )
	{
		m_TimeSinceLastRepeat[Action] = RepeatRestart;

		m_Repeated.Set( Action );
	}
	else
	{
		m_Repeated.Unset( Action );
	}
}