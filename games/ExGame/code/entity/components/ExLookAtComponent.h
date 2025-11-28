// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "EGMeshComponent.h"
#include "ExLookAtComponent.reflection.h"

class ExPlayerController;

egreflect enum class ex_look_at_component_t
{
	LookWith ,
	LookAt ,
};

egreflect class ExLookAtComponent : public egprop EGMeshComponent
{
	EG_CLASS_BODY( ExLookAtComponent , EGMeshComponent )
	EG_FRIEND_RFL( ExLookAtComponent )

protected:

	egprop ex_look_at_component_t m_Type = ex_look_at_component_t::LookWith;
	egprop eg_real m_AdditionalLookAtTranslation = 0.f;

public:

	void ApplyLookAtToPlayer( ExPlayerController* PlayerEnt ) const;

protected:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual void OnEnterWorld() override;
	virtual eg_bool CanHaveMoreThanOne() const { return false; }
};
