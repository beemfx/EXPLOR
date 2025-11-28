// (c) 2018 Beem Media

#pragma once

#include "EGWorldObject.h"
#include "EGAssetPath.h"
#include "EGWorldObjectTerrain.reflection.h"

class EGTerrain2;

egreflect class EGWorldObjectTerrain : public egprop EGWorldObject
{
	EG_CLASS_BODY( EGWorldObjectTerrain , EGWorldObject )
	EG_FRIEND_RFL( EGWorldObjectTerrain )

protected:

	egprop eg_asset_path m_TerrainFile = "egterrain";

	mutable EGTerrain2* m_LoadedTerrain = nullptr;;

protected:

	virtual void ApplyToWorldPreload( EGEntWorld* World ) const override;
	virtual void ApplyToWorldPostLoad( EGEntWorld* World ) const override;
	virtual eg_bool IsPreLoadComplete( const EGEntWorld* World ) const override;

	virtual void AddToWorldPreview( EGEntWorld* World ) const override;
	virtual void RemoveFromWorldPreview( EGEntWorld* World ) const override;
	virtual void RefreshInWorldPreview( EGEntWorld* World , const egRflEditor& ChangedProperty ) const override;
};
