// (c) 2018 Beem Media

#include "EGEntWorldLoader.h"
#include "EGLoader.h"
#include "EGEntWorld.h"
#include "EGWorldFile.h"
#include "EGFileData.h"

EG_CLASS_DECL( EGEntWorldLoader )

void EGEntWorldLoader::InitWorldLoader( eg_cpstr Filename, EGEntWorld* Owner )
{
	if( m_bIsLoading )
	{
		m_bCanceledLoad = true;
		MainLoader->CancelLoad( this );
		m_bIsLoading = false;
	}

	if( m_WorldFile )
	{
		EGDeleteObject( m_WorldFile );
		m_WorldFile = nullptr;
	}

	m_bCanceledLoad = false;
	m_bIsLoading = true;

	eg_d_string FullFilename = Filename;
	FullFilename.Append( ".egworld" );
	MainLoader->BeginLoad( *FullFilename , this , Owner && Owner->GetRole() == eg_ent_world_role::Server ? EGLoader::LOAD_THREAD_SERVER : EGLoader::LOAD_THREAD_MAIN );
}

void EGEntWorldLoader::OnDestruct()
{
	if( m_bIsLoading )
	{
		m_bCanceledLoad = true;
		MainLoader->CancelLoad( this );
		m_bIsLoading = false;
	}

	if( m_WorldFile )
	{
		EGDeleteObject( m_WorldFile );
		m_WorldFile = nullptr;
	}

	m_bCanceledLoad = false;
	m_bIsLoading = false;

	Super::OnDestruct();
}

void EGEntWorldLoader::DoLoad( eg_cpstr strFile , const eg_byte*const pMem , const eg_size_t Size )
{
	assert( nullptr == m_WorldFile );
	m_WorldFile = EGNewObject<EGWorldFile>();
	EGFileData FileData( eg_file_data_init_t::SetableUserPointer );
	FileData.SetData( const_cast<eg_byte*>(pMem) , Size );
	if( m_WorldFile )
	{
		m_WorldFile->Load( FileData , strFile , false );
	}
}

void EGEntWorldLoader::OnLoadComplete( eg_cpstr strFile )
{
	unused( strFile );

	m_bIsLoading = false;
	m_bIsLoaded = true;
}
