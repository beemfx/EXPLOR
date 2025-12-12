// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "EGComponent.h"
#include "EGAssetPath.h"
#include "EGEngineConfig.h"
#include "EGRendererTypes.h"
#include "ExNpcComponent.reflection.h"

egreflect class ExMapIconComponent : public egprop EGComponent
{
	EG_CLASS_BODY( ExMapIconComponent , EGComponent )
	EG_FRIEND_RFL( ExMapIconComponent )

protected:

	egprop eg_asset_path m_MapIcon = eg_asset_path( EXT_TEX , "" );
	egprop eg_bool m_bClampIconToMinimapEdge = false;

	egv_material m_MapIconMaterial = EGV_MATERIAL_NULL;

public:

	egv_material GetIconMaterial() const { return m_MapIconMaterial; }
	eg_bool IsIconClampedToMinimapEdge() const { return m_bClampIconToMinimapEdge; }

protected:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual void OnEnterWorld() override;
	virtual void OnLeaveWorld() override;
	virtual eg_bool CanHaveMoreThanOne() const { return false; }
};
