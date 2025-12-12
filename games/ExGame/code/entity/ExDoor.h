// (c) 2015 Beem Media

#pragma once

#include "EGComponent.h"
#include "ExEnt.h"
#include "ExDoor.reflection.h"

enum class ex_door_unlock_result : eg_uint8
{
	Unlocked,
	LockedNeedsKey,
	LockedNeedsThief,
	LockedNeedsLevel,
};

egreflect enum class ex_door_movement_t
{
	Swing,
	SlideVertical,
	SlideHorizontal,
};

class ExDoor : public ExEnt
{
	EG_CLASS_BODY( ExDoor , ExEnt )

friend class ExDoorComponent;

private:

	enum ex_door_m : eg_uint { DOOR_CLOSED , DOOR_OPENIN , DOOR_OPENOUT };

	eg_transform  m_BasePose;
	eg_transform  m_OpenPoseIn;
	eg_transform  m_OpenPoseOut;
	eg_transform  m_StartPose;
	ex_door_m     m_mode;
	eg_string_crc m_HasKeyVar;
	ex_attr_value m_LockLevel;
	eg_real       m_Progress;
	eg_bool       m_bInit:1;
	eg_bool       m_bAnimating:1;
	eg_bool       m_bIsLocked;
	eg_real       m_AnimationTime = .5f;
	eg_string_crc m_OpenAction = eg_crc("DoorAction");
	eg_string_crc m_CloseAction = eg_crc("DoorAtion");
	eg_string_crc m_LockAction = eg_crc("DoorLockedAction");
	eg_string_crc m_UnlockAction = eg_crc("DoorUnlockAction");
	eg_bool       m_bStaysOpen = false;
	eg_bool       m_bIsSecretDoor = false;

public:

	virtual void OnCreate( const egEntCreateParms& Parms ) override final;
	virtual void OnEnterWorld() override final;
	virtual void OnUpdate( eg_real DeltaTime ) override final;
	virtual eg_bool SendMsg( ex_notify_t msg , eg_ent_id SrcEntId ) override final;

	eg_bool IsLocked() const { return m_bIsLocked; }
	eg_real GetAnimationTime() const { return m_AnimationTime; }
	eg_bool ShouldStayOpen() const { return m_bStaysOpen; }
	void SetLocked( eg_bool bNewValue );
	void CheckForKey();
	ex_door_unlock_result AttemptToUnlock( const class ExFighter* GuyToUnlock );
};

egreflect class ExDoorComponent : public egprop EGComponent
{
	EG_CLASS_BODY( ExDoorComponent , EGComponent )
	EG_FRIEND_RFL( ExDoorComponent )

protected:

	egprop ex_door_movement_t m_DoorMovementType = ex_door_movement_t::Swing;
	egprop eg_real m_DoorMovementAmount = 90.f;
	egprop eg_real m_DoorAnimationTime = .5f;
	egprop eg_string_crc m_OpenAction = EGCrcDb::StringToCrc("DoorAction");
	egprop eg_string_crc m_CloseAction = EGCrcDb::StringToCrc("DoorAction");
	egprop eg_string_crc m_LockAction = EGCrcDb::StringToCrc("DoorLockedAction");
	egprop eg_string_crc m_UnlockAction = EGCrcDb::StringToCrc("DoorUnlockAction");
	egprop eg_bool m_bStaysOpen = false;
	egprop eg_bool m_bIsSecretDoor = false;

public:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual void OnEnterWorld() override;
	virtual eg_bool CanHaveMoreThanOne() const override { return false; }
	void ApplyMovementType( ExDoor* Door ) const;

};
