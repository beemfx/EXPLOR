// (c) 2017 Beem Media

#pragma once

#include "EGWeakPtr.h"
#include "EGEntTypes.h"

class EGEntWorld;
class EGTerrain2;
class EGGameTerrain2;
class EGTerrainMesh;

class EGEntWorldTerrain : public EGObject
{
	EG_CLASS_BODY( EGEntWorldTerrain , EGObject )

private:

	EGGameTerrain2*       m_Terrain = nullptr;
	EGWeakPtr<EGEntWorld> m_OwnerWorld = nullptr;
	EGTerrainMesh*        m_Mesh = nullptr;
	eg_transform          m_Pose = CT_Default;
	eg_string_small       m_Filename;
	eg_spawn_reason       m_SpawnReason = eg_spawn_reason::Unknown;

public:

	void InitTerrain( eg_cpstr Filename , EGEntWorld* Owner , eg_spawn_reason SpawnReason , const eg_transform& InitPose );
	virtual void OnDestruct() override;
	eg_bool IsLoaded() const;
	eg_bool IsLoading() const;
	eg_bool DidLoadFail() const;
	eg_cpstr GetFilename() const { return m_Filename; }
	EGGameTerrain2* GetTerrain() { return m_Terrain; }
	const EGGameTerrain2* GetTerrain() const { return m_Terrain; }
	const EGTerrainMesh* GetTerrainMesh() const { return m_Mesh; }
	const eg_transform& GetPose() const { return m_Pose; }
	void SetPose( const eg_transform& NewPose );
	eg_spawn_reason GetSpawnReason() const { return m_SpawnReason; }
	void UpdateLOD( eg_real DeltaTime , const eg_vec4& CameraPos );

private:

	void OnTerrainLoaded( EGTerrain2* Terrain );
};
