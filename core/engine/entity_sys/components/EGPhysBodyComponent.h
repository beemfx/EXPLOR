// (c) 2017 Beem Media

#pragma once

#include "EGComponent.h"
#include "EGPhysBodyDef.h"
#include "EGClassName.h"
#include "EGPhysBodyComponentTypes.h"
#include "EGPhysBodyComponent.reflection.h"

extern EGClass& EGBpPhysBody_GetClass();

egreflect class EGPhysBodyComponent : public egprop EGComponent
{
	EG_CLASS_BODY( EGPhysBodyComponent , EGComponent )
	EG_FRIEND_RFL( EGPhysBodyComponent )

private:

	egprop eg_class_name          m_BodyClass = eg_class_name( &EGBpPhysBody_GetClass() , &EGBpPhysBody_GetClass() );
	egprop eg_phys_body_flags     m_BodyFlags;
	egprop EGArray<eg_phys_shape> m_Shapes;

	egPhysBodyDef m_BodyDef;

public:
	
	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual void DrawForTool( const eg_transform& ParentPose ) const override;

	virtual eg_bool CanHaveMoreThanOne() const override { return false; }
	virtual void OnPropChanged( const egRflEditor& ChangedProperty , eg_bool& bNeedsRebuildOut );
	virtual void RefreshEditableProperties();

	const egPhysBodyDef& GetPhysBodyDef() const { return m_BodyDef; }
	const EGArray<eg_phys_shape>& GetShapes() const { return m_Shapes; }
};