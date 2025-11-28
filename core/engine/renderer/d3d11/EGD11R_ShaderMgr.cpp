// (c) 2014 Beem Media

#include "EGD11R_ShaderMgr.h"
#include "EGD11R_Base.h"
#include "EGFileData.h"
#include "EGLoader.h"
#include "EGEngineConfig.h"
#include "EGAlwaysLoaded.h"

static const eg_cpstr EXT_PS11  = "eps5";
static const eg_cpstr EXT_VS11  = "evs5";

void EGD11R_ShaderMgr::Init( ID3D11Device* Device )
{
	m_Device = Device;
	EGD11R_Texture::InitDevice( Device );
	m_VsBytes = 0;
	m_PsBytes = 0;
	m_TxBytes = 0;
	m_TxCount = 0;

	m_TxLoadAllAlways = false;

	EGAlwaysLoadedFilenameList EnumList;
	
	//
	// Find all textures
	//
	AlwaysLoaded_GetFileList( &EnumList , EXT_TEX );

	m_TxMasterListSize = EnumList.Len();
	m_TxMasterList = EGMem2_NewArray<EGD11R_Texture>( m_TxMasterListSize , eg_mem_pool::System );
	if( nullptr != m_TxMasterList )
	{
		//Gotta call constructors
		for( eg_uint i=0; i<m_TxMasterListSize; i++ )
		{
			new( &m_TxMasterList[i] ) EGD11R_Texture;
		}
	}
	else
	{
		m_TxMasterListSize = 0;
	}

	m_TxMap.Init( m_TxMasterListSize );
	for( egAlwaysLoadedFilename* FnItem : EnumList )
	{
		this->InsertTxFile( FnItem->Filename );
	}
	EnumList.ClearFilenames();

	//
	// Find all vertex shaders:
	//
	AlwaysLoaded_GetFileList(  &EnumList , EXT_VS11 );
	m_VsMap.Init( EnumList.Len() );
	for( egAlwaysLoadedFilename* FnItem : EnumList )
	{
		this->InsertVsFile( FnItem->Filename );
	}
	EnumList.ClearFilenames();

	//
	// Find all pixel shaders:
	//
	AlwaysLoaded_GetFileList( &EnumList , EXT_PS11 );
	m_PsMap.Init( EnumList.Len() );
	for( egAlwaysLoadedFilename* FnItem : EnumList )
	{
		this->InsertPsFile( FnItem->Filename );
	}
	EnumList.ClearFilenames();

	EGLogf( eg_log_t::Renderer11 , "Vertex Shaders: %u bytes and %u shaders", m_VsBytes , m_VsMap.Len() );
	EGLogf( eg_log_t::Renderer11 , "Pixel Shaders: %u bytes and %u shaders", m_PsBytes , m_PsMap.Len() );
	EGLogf( eg_log_t::Renderer11 , "Textures: %u bytes and %u textures", m_TxMasterListSize*sizeof(EGD11R_Texture), m_TxMap.Len() );
	EGLogf( eg_log_t::Renderer11 , "Actual loaded texture image bytes: %u (%uMB)" , m_TxBytes , m_TxBytes/(1024*1024) );
	EGLogf( eg_log_t::Renderer11 , "Manager: %u bytes" , sizeof(*this)-(m_TxMasterListSize*sizeof(EGD11R_Texture)) );
}

void EGD11R_ShaderMgr::Deinit( void )
{
	for( eg_uint i=0; i<m_VsMap.Len(); i++ )
	{
		m_VsMap.GetByIndex( i )->Release();
	}
	m_VsMap.Clear();

	for( eg_uint i=0; i<m_PsMap.Len(); i++ )
	{
		m_PsMap.GetByIndex( i )->Release();
	}
	m_PsMap.Clear();

	for( eg_uint i=0; i<m_TxMap.Len(); i++ )
	{
		if( nullptr != m_TxMap.GetByIndex(i)->Texture )m_TxMap.GetByIndex(i)->Texture->Release();
	}
	m_TxMap.Clear();

	m_VsMap.Deinit();
	m_PsMap.Deinit();
	m_TxMap.Deinit();
	EGMem2_Free( m_TxMasterList );
	m_TxMasterList = nullptr;

	EGD11R_Texture::InitDevice( nullptr );
	m_Device = nullptr;
}

ID3D11VertexShader* EGD11R_ShaderMgr::GetVertexShader( eg_cpstr Filename )const
{
	eg_string_crc CrcId = FilenameToCrcId( Filename , 0 );
	return m_VsMap.Get( CrcId );
}

ID3D11PixelShader* EGD11R_ShaderMgr::GetPixelShader( eg_cpstr Filename )const
{
	eg_string_crc CrcId = FilenameToCrcId( Filename , 0 );
	return m_PsMap.Get( CrcId );
}

EGD11R_Texture* EGD11R_ShaderMgr::GetTexture( eg_cpstr Filename )const
{
	eg_string_crc CrcId = FilenameToCrcId( Filename , 0 );
	return m_TxMap.Get( CrcId );
}

void EGD11R_ShaderMgr::InsertVsFile( eg_cpstr Filename )
{
	EGFileData MemFile( eg_file_data_init_t::HasOwnMemory );
	MainLoader->LoadNowTo( Filename , MemFile );
	if( 0 == MemFile.GetSize() )
	{
		assert( false ); //Couldn't read shader?
		return;
	}

	m_VsBytes += MemFile.GetSize();
	ID3D11VertexShader* pVS = nullptr;
	HRESULT Res = m_Device->CreateVertexShader( MemFile.GetDataAs<DWORD>() , MemFile.GetSize() , nullptr , &pVS );
	if( FAILED(Res) )
	{
		assert( false ); //Was shader valid?
		return;
	}

	#if defined( __DEBUG__ )
	//Check that input type for the shader is valid:
	{
		eg_bool ValidInputType = false;
		HRESULT Res;
		Res = m_Device->CreateInputLayout( D3D11R_VF_STANDARD , countof(D3D11R_VF_STANDARD) , MemFile.GetData() , MemFile.GetSize() , NULL );
		ValidInputType = ValidInputType || SUCCEEDED(Res);
		Res = m_Device->CreateInputLayout( D3D11R_VF_SIMPLE , countof(D3D11R_VF_SIMPLE) , MemFile.GetData() , MemFile.GetSize() , NULL );
		ValidInputType = ValidInputType || SUCCEEDED(Res);
		assert( ValidInputType );
	}
	#endif

	eg_string_crc CrcId = FilenameToCrcId( Filename , eg_string(EXT_VS11).Len()+1 );
	m_VsMap.Insert( CrcId , pVS );
}

void EGD11R_ShaderMgr::InsertPsFile( eg_cpstr Filename )
{
	EGFileData MemFile( eg_file_data_init_t::HasOwnMemory );
	MainLoader->LoadNowTo( Filename , MemFile );
	if( 0 == MemFile.GetSize() )
	{
		assert( false ); //Couldn't read shader?
		return;
	}

	m_PsBytes += MemFile.GetSize();
	ID3D11PixelShader* pPS = nullptr;
	HRESULT Res = m_Device->CreatePixelShader( MemFile.GetDataAs<DWORD>() , MemFile.GetSize() , nullptr , &pPS );
	if( FAILED(Res) )
	{
		assert( false ); //Was shader valid?
		return;
	}

	eg_string_crc CrcId = FilenameToCrcId( Filename , eg_string(EXT_PS11).Len()+1 );
	assert( !m_PsMap.Contains( CrcId ) );
	m_PsMap.Insert( CrcId , pPS );
}

void EGD11R_ShaderMgr::InsertTxFile( eg_cpstr Filename )
{
	if( m_TxCount >= m_TxMasterListSize )
	{
		assert( false ); //More textures than can be managed.
		return;
	}

	EGD11R_Texture* Tx = &m_TxMasterList[m_TxCount];
	Tx->Texture = nullptr;

	if( m_TxLoadAllAlways )
	{
		assert( false ); //Not supported
		#if 0
		EGFileData MemFile(5*1024*1024,__FUNCTION__);
		MainLoader->LoadNowTo( Filename , &MemFile );
		Tx->AddRef();
		if( nullptr == Tx->Texture )
		{
			assert( false ); //Invalid texture, or not loaded?
			return;
		}
		_this->m_TxBytes += MemFile.Size();
		#endif
	}

	m_TxCount++;
	Tx->Filename = Filename;
	eg_string_crc CrcId = FilenameToCrcId( Filename , eg_string(EXT_TEX).Len()+1 );
	m_TxMap.Insert( CrcId , Tx );
}

eg_string_crc EGD11R_ShaderMgr::FilenameToCrcId(  eg_cpstr Filename , eg_uint ClampSize  )
{
	eg_string String = Filename;
	String.ConvertToLower();
	if( ClampSize > 0 )String.ClampEnd( ClampSize );
	eg_string_crc Crc = eg_string_crc(String);
	return Crc;
}