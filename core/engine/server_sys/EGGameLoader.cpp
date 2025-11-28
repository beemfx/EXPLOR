/******************************************************************************
EGGameLoader
(c) 2011 Blaine Myers.
******************************************************************************/
#include "EGGameLoader.h"
#include "EGLoader.h"
#include "EGEngine.h"
#include "EGGameMap.h"
#include "EGFileData.h"
#include "EGEntWorld.h"

EG_CLASS_DECL( EGGameLoader )

void EGGameLoader::InitGameLoader( eg_cpstr strFile , EGEntWorld* EntWorld )
{
	m_EntWorld = EntWorld;
	m_LoadState = eg_load_s::NOT_LOADED;
	m_SaveGame.Clear();
	LoadInternal( strFile );
}

void EGGameLoader::OnDestruct()
{
	Unload();

	Super::OnDestruct();
}

void EGGameLoader::DoLoad( eg_cpstr strFile , const eg_byte*const pMem , const eg_size_t Size )
{
	assert( eg_load_s::LOADING == m_LoadState );

	EGFileData InFile( pMem , Size );
	m_SaveGame.Load( InFile , strFile );
}

void EGGameLoader::OnLoadComplete( eg_cpstr strFile )
{
	unused( strFile );
	m_LoadState = eg_load_s::LOADING_THINGS;

	if( m_EntWorld )
	{
		m_EntWorld->SaveGameLoadCompleteDelegate.AddUnique( this , &ThisClass::OnEntWorldLoadComplete );
		m_EntWorld->LoadGame( *this );
	}
}

void EGGameLoader::LoadInternal( eg_cpstr strFile )
{
	EGLogf( eg_log_t::Server , ("EGGameLoader::Load(\"%s\")"), strFile);

	if( m_LoadState == eg_load_s::LOADING )
	{
		assert( false ); //Cannot load a file, while it is currently being loaded.
		return;
	}

	m_LoadState = eg_load_s::LOADING;

	MainLoader->BeginLoad( strFile , this , EGLoader::LOAD_THREAD_SERVER );
}

void EGGameLoader::Unload()
{
	m_SaveGame.Clear();
	m_PendingMapLoads = 0;
	m_PendingTerrainLoads = 0;
}

void EGGameLoader::OnEntWorldLoadComplete()
{
	assert( m_LoadState == eg_load_s::LOADING_THINGS );
	m_LoadState = eg_load_s::COMPLETE;
}

void EGGameLoader::HandleThingLoaded()
{
	assert( m_LoadState == eg_load_s::LOADING_THINGS );
	assert( m_PendingTerrainLoads >= 0 && m_PendingMapLoads >= 0 );
	if( m_PendingTerrainLoads == 0 && m_PendingMapLoads == 0 )
	{
		m_LoadState = eg_load_s::COMPLETE;
	}
}
