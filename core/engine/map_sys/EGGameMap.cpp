/***************************************
	EGGameMap - 3D Renderable Map Stuff
***************************************/
#include "EGGameMap.h"
#include "EGRenderer.h"
#include "EGWorkerThreads.h"
#include "EGLoader.h"
#include "EGDebugShapes.h"

EGGameMap::EGGameMap( eg_bool Create3DAssets )
: EGWorldMapBase()
, m_VB(CT_Clear)
, m_Create3DAssets( Create3DAssets )
, m_LoadState( LOAD_NOT_LOADED )
{

}

EGGameMap::~EGGameMap()
{
	Unload();
}

void EGGameMap::LoadOnThisThread(eg_cpstr strFile)
{
	Unload();
	m_LoadState = LOAD_LOADING;
	MainLoader->LoadNow( BuildFinalFilename(strFile), this);
}

void EGGameMap::LoadOnLoadingThread( eg_cpstr strFile , eg_bool bServer )
{
	assert( bServer || EGWorkerThreads_IsThisMainThread() ); //Not supported to do this load from any other thread yet (need param to do that).
	Unload();
	m_LoadState = LOAD_LOADING;
	MainLoader->BeginLoad( BuildFinalFilename(strFile), this , bServer ? EGLoader::LOAD_THREAD_SERVER : EGLoader::LOAD_THREAD_MAIN);
}

void EGGameMap::DoLoad(eg_cpstr strFile, const eg_byte*const  pMem, const eg_size_t Size)
{
	assert( LOAD_LOADING == m_LoadState );
	
	assert(nullptr == m_pMem);

	if(!pMem || 0 == Size)
	{
		zero( &m_H );
		return;
	}

	m_nMemSize = Size;
	m_pMem = EGMem2_NewArray<eg_byte>(m_nMemSize, eg_mem_pool::Default);
	if( nullptr == m_pMem )
	{
		zero( &m_H );
		return;
	}

	EGMem_Copy(m_pMem, pMem, m_nMemSize);
	
	m_pH = (egHeader*)&m_pMem[0];
	assert(MAP_ID == m_pH->nID && MAP_VERSION == m_pH->nVersion);
	if(!(MAP_ID == m_pH->nID && MAP_VERSION == m_pH->nVersion))
	{
		assert( false );
		EGMem2_Free( m_pMem );
		m_pMem = nullptr;
		zero( &m_H );
		return;
	}

	//Load the physics data:
	eg_string PhysFile = strFile;
	PhysFile.ClampEnd( countof( ".emap" ) - 1 );
	m_PhysData.LoadNow( PhysFile );
}

void EGGameMap::OnLoadComplete( eg_cpstr strFile )
{
	m_LoadState = LOAD_LOADED;

	if(m_pMem && m_pH->nID == MAP_ID && m_pH->nVersion == MAP_VERSION )
	{
		SetPointers(*m_pH);
		for(eg_uint i=0; i<this->m_pH->nMtrlsCount; i++)
		{
			this->m_pMtrls[i].Def.MakePathsRelativeTo(strFile);
		}

		assert( m_PhysData.IsLoaded() );
	}
	else
	{
		assert( false );
		if( m_pMem )
		{
			EGMem2_Free( m_pMem );
			m_pMem = nullptr;
			zero( &m_H );
		}
	}

	if( IsLoaded() && m_Create3DAssets )
	{
		//Load the materials and light maps.
		for(eg_uint i=0; i<m_H.nMtrlsCount; i++)
		{
			eg_string_big MaterialSharedId = EGString_Format( "Map(%s,%i)" , strFile , i );
			m_pMtrls[i].Mtrl=EGRenderer::Get().CreateMaterial( &m_pMtrls[i].Def , MaterialSharedId );
		}

		//With the materials loaded, we set the material for each segment.
		for(eg_uint i=0; i<m_H.nSegmentsCount; i++)
		{
			egRasterSegment* pSeg = &m_pSegments[i];
			pSeg->mtrl = pSeg->nMtrlRef!=0 ? m_pMtrls[pSeg->nMtrlRef-1].Mtrl:EGV_MATERIAL_NULL;
		}
	
		m_VB=EGRenderer::Get().CreateVB(eg_crc("egv_vert_mesh"),m_H.nVertexesCount, m_pVertexes);
	}

	MapLoadedDelegate.Broadcast( this );
}

void EGGameMap::Unload()
{
	if(!IsLoaded())return;

	if( LOAD_LOADING == m_LoadState )
	{
		MainLoader->CancelLoad(this);
	}

	m_PhysData.Clear();

	if( m_Create3DAssets )
	{
		//Delete associated materials
		for(eg_uint i=0; i<m_H.nMtrlsCount; i++)
		{
			EGRenderer::Get().DestroyMaterial(this->m_pMtrls[i].Mtrl);
			m_pMtrls[i].Mtrl = EGV_MATERIAL_NULL;
		}
		EGRenderer::Get().DestroyVB(m_VB);
		m_VB = CT_Clear;
	}

	DeallocateMemory();
	zero(&m_H);
	m_LoadState = LOAD_NOT_LOADED;
}

void EGGameMap::DrawGraphs( class EGDebugSphere* VertexShape )const
{
	if(!m_Create3DAssets || !IsLoaded() )
		return;

	static const eg_color GraphColorList[] =
	{
		eg_color( 0 , 1 , 0 , 1 ),
		eg_color( 1 , 0 , 0 , 1 ),
		eg_color( 0 , 0 , 1 , 1 ),
		eg_color( 1 , 1 , 0 , 1 ),
		eg_color( 0 , 1 , 1 , 1 ),
		eg_color( 1 , 0 , 1 , 1 ),
	};

	for(eg_uint gi = 0; gi < m_H.nGraphsCount; gi++)
	{
		const egGraph& G = m_pGraphs[gi];

		const eg_color& GraphColor = GraphColorList[ gi%countof(GraphColorList) ];

		for(eg_uint i=0; i<G.nNumVerts; i++)
		{
			egGraphVertex& V = m_pGraphVerts[G.nFirstVert+i];

			eg_transform Pose = eg_transform::BuildTranslation( eg_vec3(V.Pos.x , V.Pos.y , V.Pos.z) );
			VertexShape->DrawDebugShape( .1f , Pose , GraphColor );

			MainDisplayList->SetWorldTF( eg_mat::I );
			for(eg_uint j=0; j<V.nNumEdges; j++)
			{
				eg_uint nTo = V.anEdge[j]-1;
				MainDisplayList->PushDefaultShader( eg_defaultshader_t::COLOR );
				MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF );
				MainDisplayList->DrawLine(V.Pos, m_pGraphVerts[nTo].Pos, eg_color(eg_color32(255,204,255,51)) );
				MainDisplayList->PopDepthStencilState();
				MainDisplayList->PopDefaultShader();
			}
		}
	}
}

void EGGameMap::DrawAABBs(eg_flags nFlags)const
{
	if(!m_Create3DAssets || !IsLoaded() )
		return;
		
	/*
	static const eg_uint  MAPDEBUG_HULLAABB=0x00000001;
	static const eg_uint  MAPDEBUG_HULLTRI=0x00000002;
	static const eg_uint  MAPDEBUG_WORLDAABB=0x00000004;
	*/
	if(nFlags.IsSet(MAPDEBUG_HULLAABB))
	{
		for(eg_uint i=0; i<m_H.nGeoBlocksCount; i++)
		{
			MainDisplayList->DrawAABB(m_pGeoBlocks[i].aabb, eg_color(eg_color32(255,255,0,0)) );
		}
	}
	
	for(eg_uint i=0; i<m_H.nRegionsCount; i++)
	{
		MainDisplayList->DrawAABB(m_pRegions[i].aabb, eg_color(eg_color32(255,0,255,0)) );
	}
	
	if(nFlags.IsSet(MAPDEBUG_WORLDAABB))
		MainDisplayList->DrawAABB(m_H.aabbMapBounds, eg_color(eg_color32(255,0,255,255)) );
		
	//Draw raw triangles:
	//EGD3D9Renderer::Get()->DrawRawTris(m_pGeoTris, m_nGeoTriCount);
	
	/*
	if(EG_CheckFlag(nFlags, MAPDEBUG_HULLTRI))
	{
		for(eg_uint i=0; i<m_nGeoBlockCount; i++)
		{
			LD_DrawTris(
				m_pDevice, 
				(ml_vec3*)&m_pGeoBlocks[i].pTris[0], 
				m_pGeoBlocks[i].nTriCount);	
		}
	}
	*/
}

void EGGameMap::DrawRegion(eg_uint nRegion)const
{
	if(!m_Create3DAssets || !IsLoaded() || !::EG_IsBetween<eg_uint>(nRegion, 1, m_H.nRegionsCount))
	{
		return;
	}

	MainDisplayList->SetWorldTF(eg_mat::I);
	MainDisplayList->PushDefaultShader( eg_defaultshader_t::MAP );
	MainDisplayList->PushSamplerState( eg_sampler_s::TEXTURE_WRAP_FILTER_DEFAULT );
	
	for(eg_uint j=0; j<m_pRegions[nRegion-1].nRasterSegCount; j++)
	{
		eg_uint nFace=m_pRegions[nRegion-1].nFirstRasterSeg+j;
		MainDisplayList->SetMaterial(m_pSegments[nFace].mtrl);
		MainDisplayList->DrawTris(m_VB, m_pSegments[nFace].nFirst, m_pSegments[nFace].nTriCount);
	}

	MainDisplayList->PopSamplerState();
	MainDisplayList->PopDefaultShader();
}

void EGGameMap::Draw()const
{
	if( !m_Create3DAssets || !IsLoaded() )
		return;
	//Obviously we shouldn't render ever region, only the visible
	//one's, but that has not been implemented yet, as portals are
	//not in the map format yet.
	
	for(eg_uint nRegion=1; nRegion<=m_H.nRegionsCount; nRegion++)
	{
		DrawRegion(nRegion);
	}
}

void EGGameMap::DeallocateMemory()
{
	//Only have to delete the lump of data,
	//then set everything else to null.
	if(m_pMem)
	{
		EGMem2_Free(m_pMem);
		m_pMem = nullptr;
	}
	m_nMemSize=0;

	#define MAP_DATA_SECTION( _type_ , _var_ ) m_p##_var_ = nullptr;
	#include "EGMapDataSections.inc"
	#undef MAP_DATA_SECTION
}

eg_string EGGameMap::BuildFinalFilename( eg_cpstr Filename )
{
	return EGString_Format( "%s.emap", Filename );
}
