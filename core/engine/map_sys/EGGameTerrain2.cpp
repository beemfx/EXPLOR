// (c) 2018 Beem Media

#include "EGGameTerrain2.h"
#include "EGLoader.h"
#include "EGFileData.h"
#include "EGRenderer.h"

void EGGameTerrain2::DoLoad( eg_cpstr strFile , const eg_byte*const pMem , const eg_size_t Size )
{
	EGFileData AsMemFile( pMem , Size );
	LoadData( AsMemFile , strFile );
}

void EGGameTerrain2::OnLoadComplete( eg_cpstr strFile )
{
	if( !IsValid() )
	{
		m_LoadState = LOAD_NOT_LOADED;
		EGLogf( eg_log_t::Error , "The terrain \"%s\" was not loaded properly." , strFile );
	}
	else
	{
		m_LoadState = LOAD_LOADED;
	}

	TerrainLoadedDelegate.Broadcast( this );
}

void EGGameTerrain2::Load( eg_cpstr sFile , eg_bool bServer )
{
	Unload();
	assert(LOAD_NOT_LOADED == m_LoadState);
	m_LoadState = LOAD_LOADING;
	MainLoader->BeginLoad( BuildFinalFilename( sFile ) , this , bServer ? EGLoader::LOAD_THREAD_SERVER : EGLoader::LOAD_THREAD_MAIN );
}

void EGGameTerrain2::LoadOnThisThread( eg_cpstr strFile )
{
	Unload();
	m_LoadState = LOAD_LOADING;
	MainLoader->LoadNow( BuildFinalFilename( strFile ) , this );
}

void EGGameTerrain2::Unload()
{
	if( LOAD_LOADING == m_LoadState )
	{
		MainLoader->CancelLoad( this );
	}

	m_LoadState = LOAD_NOT_LOADED;
}