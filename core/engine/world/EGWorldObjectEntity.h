// (c) 2018 Beem Media

#pragma once

#include "EGWorldObject.h"
#include "EGAssetPath.h"
#include "EGWorldObjectEntity.reflection.h"

class EGEnt;

egreflect class EGWorldObjectEntity : public egprop EGWorldObject
{
	EG_CLASS_BODY( EGWorldObjectEntity , EGWorldObject )
	EG_FRIEND_RFL( EGWorldObjectEntity )

protected:

	egprop eg_asset_path m_EntityDefinition     = eg_asset_path_special_t::GameEntities;
	egprop eg_d_string_ml m_InitializationString = "";

	mutable EGEnt* m_SpawnedEnt = nullptr;

protected:

	virtual void OnDestruct() override;

	virtual void ApplyToWorldPostLoad( EGEntWorld* World ) const override;

	virtual void AddToWorldPreview( EGEntWorld* World ) const override;
	virtual void RemoveFromWorldPreview( EGEntWorld* World ) const override;
	virtual void RefreshInWorldPreview( EGEntWorld* World , const egRflEditor& ChangedProperty ) const override;
	virtual void ResetPropertiesEditor() override;
	virtual egRflEditor* GetPropertiesEditor() override;

public:

	void SetInitString( eg_cpstr InStr ) { m_InitializationString = InStr; }
	void SetEntDef( eg_cpstr InEntId ) { m_EntityDefinition.Path = InEntId; }
};
