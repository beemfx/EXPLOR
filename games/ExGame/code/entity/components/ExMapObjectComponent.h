// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "EGWeakPtr.h"
#include "EGComponent.h"
#include "ExMapObjectComponent.reflection.h"

class ExGame;

egreflect class ExMapObjectComponent : public egprop EGComponent
{
	EG_CLASS_BODY( ExMapObjectComponent , EGComponent )
	EG_FRIEND_RFL( ExMapObjectComponent )

protected:

	EGWeakPtr<ExGame> m_OwnerGame;

protected:

	virtual void OnEnterWorld() override;
	virtual void OnLeaveWorld() override;
	virtual eg_bool CanHaveMoreThanOne() const { return false; }
};
