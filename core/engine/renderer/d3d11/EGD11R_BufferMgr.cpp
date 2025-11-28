// (c) 2018 Beem Media

#include "EGD11R_BufferMgr.h"
#include "EGD11R_Base.h"
#include "EGAlwaysLoaded.h"

EGD11R_AllocBuffer::EGD11R_AllocBuffer( eg_string_crc InDeclType, ID3D11Device* InOwnerDevice, ID3D11DeviceContext* InOwnerContext, ID3D11InputLayout* InInputLayout , D3D11_BIND_FLAG InBindType, const UINT InItemSize, const UINT InMaxItems, eg_flags InFlags )
: m_OwnerDevice( InOwnerDevice )
, m_OwnerContext( InOwnerContext )
, m_InputLayout( InInputLayout )
, m_Buffer( nullptr )
, m_DeclType( InDeclType )
, m_BindType( InBindType )
, m_ItemSize( InItemSize )
, m_MaxItems( InMaxItems )
, m_BufferByteSize( InItemSize * InMaxItems )
, m_MemChunk( nullptr )
, m_MemChunkSize( 0 )
, m_DynamicBufferSize( 0 )
, m_Flags( InFlags )
, m_NumCreated( 0 )
, m_bIsDirty( true )
, m_bIsValid( false )
{
	m_OwnerDevice->AddRef();
	m_OwnerContext->AddRef();
	if( m_InputLayout )
	{
		m_InputLayout->AddRef();
	}

	if( !m_Flags.IsSet( F_D11RAB_DYNAMICBUFFER ) )
	{
		m_MemChunkSize = m_BufferByteSize;
		m_MemChunk = EGMem2_NewArrayA<eg_byte>( m_MemChunkSize, eg_mem_pool::RenderResource, m_ItemSize );
		m_MemHeap.Init( m_MemChunk, m_MemChunkSize, m_ItemSize, m_ItemSize );
	}
	else
	{
		m_DynamicBufferSize = m_BufferByteSize;
	}

	EGLogf( eg_log_t::Renderer11, "Buffer Size: %gMB", (eg_real)m_BufferByteSize / ( 1024 * 1024 ) );

	Validate( true );
}

EGD11R_AllocBuffer::~EGD11R_AllocBuffer()
{
	Validate( false );

	if( !m_Flags.IsSet( F_D11RAB_DYNAMICBUFFER ) )
	{
		m_MemHeap.Deinit();
		EGMem2_Free( m_MemChunk );
		m_MemChunk = nullptr;
		m_MemChunkSize = 0;
	}

	if( m_InputLayout )
	{
		m_InputLayout->Release();
	}
	m_OwnerContext->Release();
	m_OwnerDevice->Release();
}

void EGD11R_AllocBuffer::Validate( eg_bool bInit )
{
	if( bInit && !m_bIsValid )
	{
		// Fill in a buffer description.
		D3D11_BUFFER_DESC bufferDesc;
		if( m_Flags.IsSet( F_D11RAB_DYNAMICBUFFER ) )
		{
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.ByteWidth = m_BufferByteSize;
			bufferDesc.BindFlags = m_BindType;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = m_ItemSize;
		}
		else
		{
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.ByteWidth = m_BufferByteSize;
			bufferDesc.BindFlags = m_BindType;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = m_ItemSize;
		}

		// Create the vertex buffer.
		assert( nullptr == m_Buffer );
		HRESULT nRes = m_OwnerDevice->CreateBuffer( &bufferDesc, nullptr, &m_Buffer );
		assert( SUCCEEDED( nRes ) );
	}
	else if( !bInit && m_bIsValid )
	{
		EG_SafeRelease( m_Buffer );
	}

	m_bIsValid = bInit;
	m_bIsDirty = true;
}

egv_vbuffer EGD11R_AllocBuffer::CreateVB( eg_uint NumVerts, const void* pVerts )
{
	assert( !m_Flags.IsSet( F_D11RAB_DYNAMICBUFFER ) );
	assert( m_BindType == D3D11_BIND_VERTEX_BUFFER );

	const eg_size_t DATA_SIZE = m_ItemSize*NumVerts;
	void* pBuffer = m_MemHeap.Alloc( DATA_SIZE, __FUNCTION__, __FILE__, __LINE__ );
	if( nullptr == pBuffer )
	{
		assert( false );
		EGLogf( eg_log_t::Error, __FUNCTION__ " Error: Failed to create a vertex buffer." );
		return egv_vbuffer( CT_Clear );
	}

	if( pVerts )
	{
		EGMem_Copy( pBuffer, pVerts, DATA_SIZE );
	}
	else
	{
		EGMem_Set( pBuffer, 0, DATA_SIZE );
	}
	m_bIsDirty = true;
	m_NumCreated++;
	eg_uintptr_t Addr = m_MemHeap.GetRelativeAddress( pBuffer );
	assert( 0 != Addr );//There should be some overhead so the beginning of memory should never be the buffer.
	eg_uintptr_t VBOffset = Addr / m_ItemSize;
	eg_uintptr_t BufferAddr = reinterpret_cast<eg_uintptr_t>( pBuffer );
	eg_uintptr_t MemChunkAddr = reinterpret_cast<eg_uintptr_t>( &m_MemChunk[VBOffset*m_ItemSize] );
	assert( BufferAddr == MemChunkAddr );
	egv_vbuffer Out;
	Out.IntV1 = Addr;
	Out.IntV2 = m_ItemSize;
	Out.IntV3 = m_DeclType.ToUint32();
	return Out;
}

void EGD11R_AllocBuffer::DestroyVB( const egv_vbuffer& Buffer )
{
	eg_uintptr_t Addr = Buffer.IntV1;
	void* Data = m_MemHeap.GetAbsoluteAddress( Addr );
	m_MemHeap.Free( Data );
	m_NumCreated--;
}

egv_ibuffer EGD11R_AllocBuffer::CreateIB( eg_uint NumInds, const egv_index* pIndices )
{
	assert( !m_Flags.IsSet( F_D11RAB_DYNAMICBUFFER ) );
	assert( m_BindType == D3D11_BIND_INDEX_BUFFER );
	assert( m_ItemSize == sizeof( egv_index ) );

	const eg_uint DATA_SIZE = m_ItemSize*NumInds;
	void* pBuffer = m_MemHeap.Alloc( DATA_SIZE, __FUNCTION__, __FILE__, __LINE__ );
	if( nullptr == pBuffer )
	{
		assert( false );
		EGLogf( eg_log_t::Error, __FUNCTION__ " Error: Failed to create a vertex buffer." );
		return EGV_IBUFFER_NULL;
	}

	if( pIndices )
	{
		EGMem_Copy( pBuffer, pIndices, DATA_SIZE );
	}
	else
	{
		EGMem_Set( pBuffer, 0, DATA_SIZE );
	}
	m_bIsDirty = true;
	m_NumCreated++;
	eg_uintptr_t Addr = m_MemHeap.GetRelativeAddress( pBuffer );
	assert( 0 != Addr );//There should be some overhead so the beginning of memory should never be the buffer.
	eg_uintptr_t IBOffset = Addr / m_ItemSize;
	eg_uintptr_t BufferAddr = reinterpret_cast<eg_uintptr_t>( pBuffer );
	eg_uintptr_t MemChunkAddr = reinterpret_cast<eg_uintptr_t>( &m_MemChunk[IBOffset*m_ItemSize] );
	assert( BufferAddr == MemChunkAddr );
	return static_cast<egv_ibuffer>( Addr );
}

void EGD11R_AllocBuffer::DestroyIB( const egv_ibuffer& Buffer )
{
	eg_uintptr_t Addr = Buffer;
	void* Data = m_MemHeap.GetAbsoluteAddress( Addr );
	m_MemHeap.Free( Data );
	m_NumCreated--;
}

void EGD11R_AllocBuffer::SetBufferData( eg_uintptr_t Addr , const void* SourceV , const eg_size_t SourceVSize )
{
	void* Data = m_MemHeap.GetAbsoluteAddress( Addr );
	EGMem_Copy( Data, SourceV, SourceVSize );
	m_bIsDirty = true;
}

void EGD11R_AllocBuffer::UpdateDynamicResource( const void* Data, eg_size_t DataSize )
{
	assert( m_Flags.IsSet( F_D11RAB_DYNAMICBUFFER ) );

	if( DataSize <= m_DynamicBufferSize )
	{
		D3D11_MAPPED_SUBRESOURCE Resource;
		HRESULT hRes = m_OwnerContext->Map( m_Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource );
		if( S_OK == hRes )
		{
			EGMem_Copy( Resource.pData, Data, DataSize );
			m_OwnerContext->Unmap( m_Buffer, 0 );
		}
		else
		{
			assert( false );
		}
	}
	else
	{
		assert( false ); // Data too big
	}
}

void EGD11R_AllocBuffer::PreFrameUpdate()
{
	if( m_bIsDirty )
	{
		EGLogf( eg_log_t::RendererActivity, "Vertex Buffer changed." );

		if( m_Buffer && m_MemChunk )
		{
			m_OwnerContext->UpdateSubresource( m_Buffer, 0, nullptr, m_MemChunk, 0, 0 );
		}

		m_bIsDirty = false;
	}
}

void EGD11R_AllocBuffer::SetAsCurrentVertexBuffer()
{
	UINT Offset = 0;
	m_OwnerContext->IASetVertexBuffers( 0, 1, &m_Buffer, &m_ItemSize, &Offset );
	m_OwnerContext->IASetInputLayout( m_InputLayout );
}

void EGD11R_AllocBuffer::SetAsCurrentIndexBuffer()
{
	assert( m_BindType == D3D11_BIND_INDEX_BUFFER );
	static_assert( sizeof( egv_index ) == 2, "Bigger indexes?" );
	m_OwnerContext->IASetIndexBuffer( m_Buffer, DXGI_FORMAT_R16_UINT, 0 );
}

EGD11R_BufferMgr::EGD11R_BufferMgr( ID3D11Device* InOwnerDevice, ID3D11DeviceContext* InOwnerContext )
: m_OwnerDevice( InOwnerDevice )
, m_OwnerContext( InOwnerContext )
, m_LayoutDatas( CT_Clear , egD11R_VertexLayoutData() )
{
	m_OwnerDevice->AddRef();
	m_OwnerContext->AddRef();

	for( const egD11R_VertexFormat& Format : D11R_VertexFormats )
	{
		assert( !m_LayoutDatas.Contains( Format.VertexType ) );
		if( !m_LayoutDatas.Contains( Format.VertexType ) )
		{
			egD11R_VertexLayoutData NewLayoutData;
			NewLayoutData.Format = &Format;

			eg_size_t TemplateShaderSize = 0;
			void* TemplateShader = AlwaysLoaded_GetFile( NewLayoutData.Format->TemplateShader , &TemplateShaderSize );

			HRESULT Res = m_OwnerDevice->CreateInputLayout( NewLayoutData.Format->ElementDesc , NewLayoutData.Format->ElementCount , TemplateShader , TemplateShaderSize , &NewLayoutData.Layout );
			assert(SUCCEEDED(Res));
			if( SUCCEEDED(Res) )
			{
				m_LayoutDatas.Insert( Format.VertexType , NewLayoutData );
			}
		}
	}

	// Create the buffers
	assert( nullptr == m_MeshVB );
	m_MeshVB = new ( eg_mem_pool::RenderResource ) EGD11R_AllocBuffer ( eg_crc("egv_vert_mesh") , m_OwnerDevice , m_OwnerContext , GetInputLayoutForType( eg_crc("egv_vert_mesh") ) , D3D11_BIND_VERTEX_BUFFER , sizeof(egv_vert_mesh) , MAX_VERTEXES , 0 );

	assert( nullptr == m_TerrainVB );
	m_TerrainVB = new ( eg_mem_pool::RenderResource ) EGD11R_AllocBuffer ( eg_crc("egv_vert_terrain") , m_OwnerDevice , m_OwnerContext , GetInputLayoutForType( eg_crc("egv_vert_terrain") ) , D3D11_BIND_VERTEX_BUFFER , sizeof(egv_vert_terrain) , MAX_VERTEXES , 0 );

	assert( nullptr == m_SimpleVB );
	m_SimpleVB = new (eg_mem_pool::RenderResource ) EGD11R_AllocBuffer ( eg_crc("egv_vert_simple") , m_OwnerDevice , m_OwnerContext , GetInputLayoutForType( eg_crc("egv_vert_simple") ) , D3D11_BIND_VERTEX_BUFFER , sizeof(egv_vert_simple) , MAX_RAWTRI_VERTS , F_D11RAB_DYNAMICBUFFER );

	assert( nullptr == m_IB );
	m_IB = new ( eg_mem_pool::RenderResource ) EGD11R_AllocBuffer( eg_crc("egv_index") , m_OwnerDevice , m_OwnerContext , nullptr , D3D11_BIND_INDEX_BUFFER , sizeof(egv_index) , MAX_INDEXES , 0 );
}

EGD11R_BufferMgr::~EGD11R_BufferMgr()
{
	EG_SafeDelete( m_IB );
	EG_SafeDelete( m_MeshVB );
	EG_SafeDelete( m_TerrainVB );
	EG_SafeDelete( m_SimpleVB );

	for( eg_size_t i=0; i<m_LayoutDatas.Len(); i++ )
	{
		m_LayoutDatas.GetByIndex( i ).Layout->Release();
	}
	m_LayoutDatas.Clear();

	m_OwnerContext->Release();
	m_OwnerDevice->Release();
}

void EGD11R_BufferMgr::Validate( eg_bool bInitialize )
{
	auto DoValidate = [this,bInitialize]( EGD11R_AllocBuffer* AllocBuffer ) -> void
	{
		if( AllocBuffer )
		{
			AllocBuffer->Validate( bInitialize );
		}
	};

	DoValidate( m_MeshVB );
	DoValidate( m_TerrainVB );
	DoValidate( m_SimpleVB );
	DoValidate( m_IB );
}

void EGD11R_BufferMgr::PreFrameUpdate()
{
	if( m_MeshVB )
	{
		m_MeshVB->PreFrameUpdate();
	}

	if( m_TerrainVB )
	{
		m_TerrainVB->PreFrameUpdate();
	}

	if( m_IB )
	{
		m_IB->PreFrameUpdate();
	}

	if( m_IB )
	{
		m_IB->SetAsCurrentIndexBuffer();
	}
}

ID3D11InputLayout* EGD11R_BufferMgr::GetInputLayoutForType( eg_string_crc VertexType ) const
{
	return m_LayoutDatas[VertexType].Layout;
}

const egD11R_VertexFormat& EGD11R_BufferMgr::GetVertexFormatForType( eg_string_crc VertexType ) const
{
	return *m_LayoutDatas[VertexType].Format;
}

egv_vbuffer EGD11R_BufferMgr::CreateVB( eg_string_crc VertexType , eg_uint NumVerts , const void* VertData )
{
	EGD11R_AllocBuffer* AllocBuffer = GetAllocForType( VertexType );
	egv_vbuffer Out( CT_Clear );
	if( AllocBuffer )
	{
		Out = AllocBuffer->CreateVB( NumVerts , VertData );
	}
	return Out;
}

void EGD11R_BufferMgr::DestroyVB( const egv_vbuffer& Buffer )
{
	if( Buffer.IsValid() )
	{
		eg_string_crc VertexType = eg_string_crc( Buffer.IntV3 );
		EGD11R_AllocBuffer* AllocBuffer = GetAllocForType( VertexType );
		assert( AllocBuffer ); // Bad type?
		if( AllocBuffer )
		{
			AllocBuffer->DestroyVB( Buffer );
		}
	}
}

egv_ibuffer EGD11R_BufferMgr::CreateIB( eg_uint NumIndexes , const egv_index* IndexData )
{
	egv_ibuffer Out = EGV_IBUFFER_NULL;
	if( m_IB )
	{
		Out = m_IB->CreateIB( NumIndexes , IndexData );
	}
	return Out;
}

void EGD11R_BufferMgr::DestroyIB( const egv_ibuffer& Buffer )
{
	if( Buffer != EGV_IBUFFER_NULL )
	{
		if( m_IB )
		{
			m_IB->DestroyIB( Buffer );
		}
	}
}

void EGD11R_BufferMgr::SetBufferData( const egv_vbuffer& Buffer , const void* SourceV , const eg_size_t SourceVSize )
{
	eg_string_crc VertexType = eg_string_crc( Buffer.IntV3 );
	EGD11R_AllocBuffer* AllocBuffer = GetAllocForType( VertexType );
	if( AllocBuffer )
	{
		AllocBuffer->SetBufferData( Buffer.IntV1 , SourceV , SourceVSize );
	}
}

void EGD11R_BufferMgr::SetBufferData( const egv_ibuffer& Buffer, const void* SourceV, const eg_size_t SourceVSize )
{
	if( m_IB )
	{
		m_IB->SetBufferData( Buffer , SourceV , SourceVSize );
	}
}

void EGD11R_BufferMgr::UpdateSimpleVB( const void* Data, eg_size_t DataSize )
{
	if( m_SimpleVB )
	{
		m_SimpleVB->UpdateDynamicResource( Data , DataSize );
	}
}

void EGD11R_BufferMgr::SetVertexLayout( eg_string_crc NewType )
{
	if( m_LastVertexLayoutType != NewType )
	{
		m_LastVertexLayoutType = NewType;

		switch_crc( NewType )
		{
		case_crc("egv_vert_mesh"):
			if( m_MeshVB )
			{
				m_MeshVB->SetAsCurrentVertexBuffer();
			}
			break;
		case_crc("egv_vert_simple"):
		{
			if( m_SimpleVB )
			{
				m_SimpleVB->SetAsCurrentVertexBuffer();
			}
		} break;
		case_crc("egv_vert_terrain"):
			if( m_TerrainVB )
			{
				m_TerrainVB->SetAsCurrentVertexBuffer();
			}
			break;
		default:
			assert( false );
			break;	
		}
	}
}

EGD11R_AllocBuffer* EGD11R_BufferMgr::GetAllocForType( eg_string_crc VertexType ) const
{
	switch_crc( VertexType )
	{
		case_crc("egv_vert_mesh"): return m_MeshVB;
		case_crc("egv_vert_terrain"): return m_TerrainVB;
	}
	assert( false );
	return nullptr;
}
