// (c) 2011 Beem Media

#include "EGMesh.h"
#include "EGSkel.h"
#include "EGMeshState.h"
#include "EGLoader.h"
#include "EGRenderer.h"

EGMesh::EGMesh( eg_cpstr Filename )
: EGMeshBase()
, m_LoadState( LOAD_NOT_LOADED )
, m_ib(EGV_IBUFFER_NULL)
, m_vb(CT_Clear)
, m_DefTexCreated(false)
{
	LoadFromBinaryUseThread(Filename);
}

EGMesh::~EGMesh()
{
	Unload();
}

void EGMesh::DeallocateMemory()
{
	//Only have to delete the lump of data,
	//then set everything else to null.
	if(m_pMem)
	{
		EGMem2_Free(m_pMem);
		m_pMem = nullptr;
	}
	m_nMemSize=0;

	#define MESH_DATA_SECTION( _type_ , _var_ ) m_p##_var_ = nullptr;
	#include "EGMeshDataSections.inc"
	#undef MESH_DATA_SECTION
}

void EGMesh::Draw( const class EGMeshState& MeshState ) const
{
	assert( MeshState.m_Materials.Len() == m_H.nSubMeshesCount );
	assert( MeshState.m_Bones.Len() == (m_H.nBonesCount+1) );

	for(eg_uint i=0; i<m_H.nSubMeshesCount; i++)
	{
		MainDisplayList->SetMaterial( MeshState.m_Materials[i]);

		//Set the bone matrices.
		MainDisplayList->SetBoneMats( MeshState.m_Bones.GetArray() , m_H.nBonesCount+1); //Set one extra since the first is the identity.		
		MainDisplayList->DrawIndexedTris(m_vb,m_ib,m_pSubMeshes[i].nFirstIndex,m_H.nVertexesCount,m_pSubMeshes[i].nNumTri);
	}

}

void EGMesh::DrawRaw( const class EGMeshState& MeshState ) const
{
	assert( MeshState.m_Bones.Len() == (m_H.nBonesCount+1) );

	//This render function does not set the material.
	for(eg_uint i=0; i<m_H.nSubMeshesCount; i++)
	{		
		//Set the bone matrices.
		MainDisplayList->SetBoneMats( MeshState.m_Bones.GetArray() , m_H.nBonesCount+1); //Set one extra since the first is the identity.
		MainDisplayList->DrawIndexedTris(m_vb,m_ib,m_pSubMeshes[i].nFirstIndex,m_H.nVertexesCount,m_pSubMeshes[i].nNumTri);
	}
}

void EGMesh::Unload()
{
	if( LOAD_LOADING == m_LoadState )
	{
		MainLoader->CancelLoad(this);
		DeallocateMemory();
	}
	//We won't bother to make sure a mesh is loaded,
	//we'll just delete everything anyway, and everything
	//that is empty should be set to null anyway.
	//Actually we will.
	if(!IsLoaded())
	{
		assert( nullptr == m_pMem ); //How'd this happen?
		return;
	}
	
	EGRenderer::Get().DestroyVB(m_vb);
	m_vb = CT_Clear;
	EGRenderer::Get().DestroyIB(m_ib);
	m_ib = EGV_IBUFFER_NULL;
	
	//To unload we'll just deallocate memory,
	//and set the ID and version to 0 so that
	//if we load again we won't accidentally keep
	//the information and think that the mesh was
	//read properly.
	DeallocateMemory();

	zero( &m_H );
}

void EGMesh::LoadFromBinaryUseThread(eg_cpstr strFile)
{
	assert( LOAD_LOADING != m_LoadState );
	Unload();
	m_LoadState = LOAD_LOADING;
	MainLoader->BeginLoad(strFile, this, EGLoader::LOAD_THREAD_MAIN);
}

void EGMesh::LoadFromBinary(eg_cpstr strFile)
{
	assert( LOAD_LOADING != m_LoadState );
	Unload();
	m_LoadState = LOAD_LOADING;
	MainLoader->LoadNow(strFile, this);
}

void EGMesh::DoLoad(eg_cpstr strFile , const eg_byte*const pMem , const eg_size_t Size )
{
	unused( strFile );

	assert( LOAD_LOADING == m_LoadState );
	assert(nullptr == m_pMem);

	if(!pMem || 0 == Size)
	{
		zero( &m_H );
		return;
	}

	m_nMemSize = Size;
	m_pMem = EGMem2_NewArray<eg_byte>(m_nMemSize, eg_mem_pool::Default);
	if( nullptr != m_pMem )
	{
		EGMem_Copy(m_pMem, pMem, m_nMemSize);
		m_pH = (egHeader*)&m_pMem[0];
		assert(MESH_ID == m_pH->nID && MESH_VERSION == m_pH->nVer);
		if(!(MESH_ID == m_pH->nID && MESH_VERSION == m_pH->nVer))
		{
			EGMem2_Free( m_pMem );
			m_pMem = nullptr;
			zero( &m_H );
		}
	}
	else
	{
		assert( false ); //Out of memory.
		zero( &m_H );
	}
}
void EGMesh::OnLoadComplete(eg_cpstr strFile)
{
	assert( LOAD_LOADING == m_LoadState );
	m_LoadState = LOAD_LOADED;
	assert( 0 == m_H.nID && 0 == m_H.nVer ); //m_H should only be set in this function and not on the loading thread, since IsLoaded is based on these values.

	if( nullptr == m_pMem )
	{
		zero( &m_H );
		assert( false ); //Mesh wasn't loaded.
		return;
	}

	SetPointers(*m_pH);

	InitPrivateData(strFile);

	//Create the vertex and index buffers.
	assert( IsLoaded() ); //We should never have gotten this far if the mesh isn't loaded, m_pMem should have been nulled in OnLoad if it failed.
	if( !IsLoaded() )return;

	m_vb = EGRenderer::Get().CreateVB(eg_crc("egv_vert_mesh"),m_pH->nVertexesCount, m_pVertexes);
	m_ib = EGRenderer::Get().CreateIB(m_pH->nIndexesCount, m_pIndexes);

	OnLoaded.Broadcast();
}
