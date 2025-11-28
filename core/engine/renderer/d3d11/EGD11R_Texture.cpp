#include "EGD11R_Texture.h"
#include "EGLoader.h"
#include <DirectXTKMar2015/DDSTextureLoader11.h>

ID3D11Device* EGD11R_Texture::s_Device = nullptr;

void EGD11R_Texture::AddRef()
{
	RefCount++;
	if( 1 == RefCount && Filename.Len() > 0 )
	{
		EGLogf( eg_log_t::RendererActivity , "(%s)->Create()" , Filename.String() );
		assert( LOAD_NOT_LOADED == m_LoadState );
		m_LoadState = LOAD_LOADING;
		MainLoader->BeginLoad( this->Filename , this , EGLoader::LOAD_THREAD_RENDER );
	}
}

void EGD11R_Texture::Release()
{ 
	assert( RefCount > 0 ); if( RefCount > 0 ){ RefCount--; }

	if( 0 == RefCount )
	{
		EGLogf( eg_log_t::RendererActivity , "(%s)->Destroy()" , Filename.String() );
		if( LOAD_LOADING == m_LoadState )
		{
			MainLoader->CancelLoad( this );
			if( nullptr != TempLoadData )
			{
				EGMem2_Free( TempLoadData );
				TempLoadData = nullptr;
			}
		}
		assert( nullptr == TempLoadData );
		EG_SafeRelease( Texture );
		EG_SafeRelease( View );
		Texture = nullptr;
		m_LoadState = LOAD_NOT_LOADED;
	}
}

void EGD11R_Texture::DoLoad(eg_cpstr strFile, const eg_byte*const  pMem, const eg_size_t Size)
{
	unused( strFile );

	assert( nullptr == TempLoadData );
	TempLoadDataSize = Size;
	TempLoadData = EGMem2_Alloc( Size , eg_mem_pool::DefaultHi );
	EGMem_Copy( TempLoadData , pMem , TempLoadDataSize );
}

void EGD11R_Texture::OnLoadComplete(eg_cpstr strFile)
{
	unused( strFile );

	assert( nullptr != TempLoadData );
	if( RefCount > 0 )
	{
		CreateTextureFromData( s_Device , TempLoadData , TempLoadDataSize );
	}
	else
	{
		//Texture was deleted before load completed!
	}
	EGMem2_Free( TempLoadData );
	TempLoadData = nullptr;
	TempLoadDataSize = 0;
	m_LoadState = LOAD_LOADED;
}

void EGD11R_Texture::CreateTextureFromData( ID3D11Device* Device , const void* Data , eg_size_t Size )
{
	if(0 == Size)
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": \"%s\" had no data." , Filename.String() );
		return;
	}

	HRESULT Res = DirectX::CreateDDSTextureFromMemory( Device , static_cast<const uint8_t*>(Data) , Size , &Texture , &View );
	assert( SUCCEEDED(Res) );
	if( SUCCEEDED(Res) )
	{

	}
	else
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ " Couldn't get image from \"%s\"." , Filename.String());

		EG_SafeRelease( Texture );
		EG_SafeRelease( View );
	}
}


//
// DDSTextureLoader Lib
//
// Some weirdness to make sure it builds appropriately, and also allocates
// memory the way we want it to.

#pragma warning( disable : 4530 )
#pragma warning( disable : 4062 )
#define NO_D3D11_DEBUG_NAME


#undef assert
#define assert( b )
#include <DirectXTKMar2015/DDSTextureLoader11.cpp>
//#include <DirectXTKMar2015/XboxDDSTextureLoader.cpp>
//#include <DirectXTKMar2015/WICTextureLoader.cpp>