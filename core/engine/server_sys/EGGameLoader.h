/******************************************************************************
EGGameLoader
(c) 2011 Blaine Myers.
******************************************************************************/
#pragma once

#include "EGEngineConfig.h"
#include "EGLoader_Loadable.h"
#include "EGSaveGame.h"

class EGEntWorld;

class EGGameLoader : public EGObject , public ILoadable
{
	EG_CLASS_BODY( EGGameLoader , EGObject )

public:

	enum class eg_load_s
	{
		NOT_LOADED,
		LOADING,
		LOADING_THINGS,
		COMPLETE,
		FAILURE,
	};

public:

	void InitGameLoader( eg_cpstr strFile , EGEntWorld* EntWorld );
	virtual void OnDestruct() override;
	eg_bool IsLoaded()const{ return eg_load_s::COMPLETE == m_LoadState || eg_load_s::FAILURE == m_LoadState; } //Any access to the data should be only if this is returning true, otherwise the loading thread may be modifying the data, and there are no mutexes.

public:

	EGEntWorld* m_EntWorld;
	eg_load_s   m_LoadState;
	EGSaveGame  m_SaveGame;
	eg_string   m_SkyboxDefCrcStr;
	eg_int      m_PendingMapLoads = 0;
	eg_int      m_PendingTerrainLoads = 0;

private:

	void LoadInternal( eg_cpstr strFile );
	void Unload();
	void OnEntWorldLoadComplete();

	void HandleThingLoaded();

	virtual void DoLoad( eg_cpstr strFile , const eg_byte*const  pMem , const eg_size_t Size ) override final;
	virtual void OnLoadComplete( eg_cpstr strFile ) override final;

};
