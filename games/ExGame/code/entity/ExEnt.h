// (c) 2015 Beem Media

#pragma once

#include "EGEnt.h"
#include "ExGameTypes.h"
#include "ExEnt.reflection.h"

class ExGame;
class ExPlayerController;

egreflect class ExEnt : public egprop EGEnt
{
	EG_CLASS_BODY( ExEnt , EGEnt )
	EG_FRIEND_RFL( ExEnt )

public:

	static const eg_string_crc PlayerControllerId;

protected:

	eg_lockstep_id m_LockstepId;
	eg_s_string_sml8 m_InitId = CT_Preserve;

public:

	virtual void OnCreate( const egEntCreateParms& Parms ) override;
	virtual eg_bool SendMsg( ex_notify_t Note , eg_ent_id SrcEnt );
	virtual void AssignToPlayer( const eg_lockstep_id& InLockstepId );
	const eg_lockstep_id& GetAssignedPlayer() const { return m_LockstepId; }
	const eg_s_string_sml8& GetInitId() const { return m_InitId; }

	void HandlePlayerLookAt( ExPlayerController* PlayerEnt ) const;
	void SetAmbientVolume( eg_real NewValue );

	ExGame* GetGameData();
	const ExGame* GetGameData() const;
	ExEnt* GetEnt( const eg_ent_id& Id );
};
