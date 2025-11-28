// (c) 2021 Beem Media. All Rights Reserved.

#pragma once

#include "EGVisualComponent.h"
#include "EGDebugDrawCloseLightsComponent.reflection.h"

class EGDebugSphere;

egreflect class EGDebugDrawCloseLightsComponent : public egprop EGVisualComponent
{
	EG_CLASS_BODY( EGDebugDrawCloseLightsComponent , EGVisualComponent )

protected:
	
	EGDebugSphere* m_DbLight = nullptr;

protected:

	virtual void OnConstruct() override;
	virtual void OnDestruct() override;
	virtual void RelevantUpdate( eg_real DeltaTime ) override;
	virtual void Draw( const eg_transform& ParentPose ) const override;
};
