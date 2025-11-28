// (c) 2017 Beem Media

#pragma once

#include "EGWeakPtr.h"
#include "EGEntTypes.h"

class EGEntWorld;
class EGGameMap;

class EGEntWorldMap : public EGObject
{
	EG_CLASS_BODY( EGEntWorldMap , EGObject )

private:

	EGGameMap*            m_Map = nullptr;
	EGWeakPtr<EGEntWorld> m_OwnerWorld = nullptr;
	eg_string_small       m_Filename;
	eg_spawn_reason       m_SpawnReason = eg_spawn_reason::Unknown;

public:

	void InitMap( eg_cpstr Filename , EGEntWorld* Owner , eg_spawn_reason SpawnReason );
	virtual void OnDestruct() override;
	eg_bool IsLoaded() const;
	eg_bool IsLoading() const;
	eg_bool DidLoadFail() const;
	eg_cpstr GetFilename() const { return m_Filename; }
	EGGameMap* GetMap() { return m_Map; }
	const EGGameMap* GetMap() const { return m_Map; }
	eg_spawn_reason GetSpawnReason() const { return m_SpawnReason; }

private:

	void OnMapLoaded( EGGameMap* GameMap );
};
