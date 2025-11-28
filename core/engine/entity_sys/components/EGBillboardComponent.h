// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "EGComponent.h"
#include "EGBillboardComponent.reflection.h"

egreflect enum class eg_billboarding_t
{
	AboutAllAxis,
	AboutYAxis,
};

egreflect class EGBillboardComponent : public egprop EGComponent
{
	EG_CLASS_BODY( EGBillboardComponent , EGComponent )
	EG_FRIEND_RFL( EGBillboardComponent )

protected:

	egprop eg_billboarding_t m_BillboardType;

protected:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual eg_transform GetPose( eg_string_crc JointName ) const override;
};
