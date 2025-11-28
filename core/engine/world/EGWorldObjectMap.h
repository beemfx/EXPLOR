// (c) 2018 Beem Media

#pragma once

#include "EGWorldObject.h"
#include "EGAssetPath.h"
#include "EGWorldObjectMap.reflection.h"

class EGGameMap;

egreflect class EGWorldObjectMap : public egprop EGWorldObject
{
	EG_CLASS_BODY( EGWorldObjectMap , EGWorldObject )
	EG_FRIEND_RFL( EGWorldObjectMap )

protected:

	egprop eg_asset_path m_MapFile           = "emap";
	egprop eg_bool       m_bSpawnMapEntities = true;

	mutable EGGameMap* m_LoadedMap = nullptr;

protected:

	virtual void RefreshEditableProperties() override;
	virtual void ApplyToWorldPreload( EGEntWorld* World ) const override;
	virtual void ApplyToWorldPostLoad( EGEntWorld* World ) const override;
	virtual eg_bool IsPreLoadComplete( const EGEntWorld* World ) const override;

	virtual void AddToWorldPreview( EGEntWorld* World ) const override;
	virtual void RemoveFromWorldPreview( EGEntWorld* World ) const override;
	virtual void RefreshInWorldPreview( EGEntWorld* World , const egRflEditor& ChangedProperty ) const override;
};
