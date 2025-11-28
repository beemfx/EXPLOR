// (c) 2018 Beem Media

#pragma once

#include "EGMeshComponent.h"
#include "EGSkyboxMeshComponent.reflection.h"


egreflect class EGSkyboxMeshComponent : public egprop EGMeshComponent
{
	EG_CLASS_BODY( EGSkyboxMeshComponent , EGMeshComponent )
	EG_FRIEND_RFL( EGSkyboxMeshComponent )

public:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual void Draw( const eg_transform& ParentPose ) const override;
};
