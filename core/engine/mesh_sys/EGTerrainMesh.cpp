// (c) 2013 Beem Media

#include "EGTerrainMesh.h"
#include "EGLoader.h"
#include "EGFileData.h"
#include "EGRenderer.h"
#include "EGVDynamicBuffer.h"
#include "EGDebugText.h"

EGTerrainMesh::EGTerrainMesh( EGGameTerrain2* Terrain ) 
: m_Terrain( Terrain )
, m_IsFinalized( false )
, m_mtrl( EGV_MATERIAL_NULL )
, m_SegDivs( 10 )
, m_LODThresholdPct( .1f )
, m_SegDimHi( 32 )
, m_SegDimLo( 6 )
{
	FinalizeLoad();
}

void EGTerrainMesh::FinalizeLoad()
{
	if( nullptr == m_Terrain )
	{
		assert( false );
		return;
	}

	m_Min = eg_vec2( m_Terrain->GetXMin() , m_Terrain->GetZMin() );
	m_Dims = eg_vec2( m_Terrain->GetXMax() , m_Terrain->GetZMax() ) - m_Min;

	const eg_uint TOTAL_SEGS = m_SegDivs * m_SegDivs;

	assert( TOTAL_SEGS >= 4 );

	const eg_uint NUM_HI_SEGS = 4;
	const eg_uint NUM_LO_SEGS = TOTAL_SEGS - NUM_HI_SEGS;

	const eg_uint NUM_VERTS = NUM_HI_SEGS*m_SegDimHi.GetNumVerts() + NUM_LO_SEGS*m_SegDimLo.GetNumVerts();
	const eg_uint NUM_MESH_INDS = NUM_HI_SEGS*3*m_SegDimHi.GetNumTris() + NUM_LO_SEGS*3*m_SegDimLo.GetNumTris();
	m_NumSeamInds = (m_SegDimHi.GetDim()*3 + m_SegDimLo.GetDim()*3)*8;
	const eg_uint NUM_INDS = NUM_MESH_INDS + m_NumSeamInds;

	assert( nullptr == m_MeshBuffer );
	m_MeshBuffer = EGRenderer::Get().CreateDynamicBuffer( eg_crc("egv_vert_terrain") , NUM_VERTS , NUM_INDS );

	// Create all the free segments
	if( m_MeshBuffer )
	{
		EGVDynamicBuffer::LockDynamicBuffers();

		eg_uint NextVertex = 0;
		eg_uint NextIndex = 0;
		for( eg_uint i=0; i<TOTAL_SEGS; i++ )
		{
			egTerrainMeshSegment NewSegment;
			NewSegment.Verts = &m_MeshBuffer->GetVertexBufferData<egv_vert_terrain>()[NextVertex];
			NewSegment.Indexes = &m_MeshBuffer->GetIndexBufferData()[NextIndex];
			NewSegment.FirstVertIdx = NextVertex;
			NewSegment.FirstIndexIdx = NextIndex;
			NewSegment.bIsHi = i < NUM_HI_SEGS;

			const eg_uint FIELD_SIZE = NewSegment.bIsHi ? m_SegDimHi.GetDim() : m_SegDimLo.GetDim();

			// We only need to set up the indexes once
			for(eg_uint y=0; y<FIELD_SIZE; y++)
			{
				for(eg_uint x=0; x<FIELD_SIZE; x++)
				{
					const eg_uint Quad = x + FIELD_SIZE * y;
					const eg_uint Triangle = 2 * Quad;
					const eg_uint FirstSegIndex = 3 * Triangle;

					const eg_uint A = x + (FIELD_SIZE+1)*y;
					const eg_uint B = A + 1;
					const eg_uint C = B + FIELD_SIZE+1;
					const eg_uint D = A + FIELD_SIZE+1;

					NewSegment.Indexes[FirstSegIndex+0] = EG_To<egv_index>(NewSegment.FirstVertIdx + A);
					NewSegment.Indexes[FirstSegIndex+1] = EG_To<egv_index>(NewSegment.FirstVertIdx + C);
					NewSegment.Indexes[FirstSegIndex+2] = EG_To<egv_index>(NewSegment.FirstVertIdx + B);

					NewSegment.Indexes[FirstSegIndex+3] = EG_To<egv_index>(NewSegment.FirstVertIdx + A);
					NewSegment.Indexes[FirstSegIndex+4] = EG_To<egv_index>(NewSegment.FirstVertIdx + D);
					NewSegment.Indexes[FirstSegIndex+5] = EG_To<egv_index>(NewSegment.FirstVertIdx + C);
				}
			}

			if( NewSegment.bIsHi )
			{
				NextVertex += m_SegDimHi.GetNumVerts();
				NextIndex += m_SegDimHi.GetNumTris()*3;
			}
			else
			{
				NextVertex += m_SegDimLo.GetNumVerts();
				NextIndex += m_SegDimLo.GetNumTris()*3;
			}

			assert( NextVertex <= m_MeshBuffer->GetVertexBufferDataCount() );
			assert( NextIndex <= m_MeshBuffer->GetIndexBufferDataCount() );

			m_FreeSegs.Append( NewSegment );
		}

		m_SeamInds = &m_MeshBuffer->GetIndexBufferData()[NextIndex];
		NextIndex += m_NumSeamInds;

		for( eg_uint i=0; i<m_NumSeamInds; i++ )
		{
			m_SeamInds[i] = 0;
		}

		assert( NextVertex == m_MeshBuffer->GetVertexBufferDataCount() );
		assert( NextIndex == m_MeshBuffer->GetIndexBufferDataCount() );

		EGVDynamicBuffer::UnlockDynamicBuffers();
	}

	UpdateLODSegments( true );
	GenerateMesh();

	eg_string MaterialSharedId = EGString_Format( "Terrain(%s,0)" , m_Filename.String() );
	m_mtrl = EGRenderer::Get().CreateMaterial( &m_Terrain->GetMaterialDef() , MaterialSharedId );

	m_IsFinalized = true;
}

void EGTerrainMesh::Draw( const eg_transform& WorldPose ) const
{
	if( m_IsFinalized && m_MeshBuffer )
	{
		MainDisplayList->PushDefaultShader( eg_defaultshader_t::TERRAIN );
		MainDisplayList->SetWorldTF( eg_mat(WorldPose) );
		MainDisplayList->SetMaterial(m_mtrl);
		MainDisplayList->DrawIndexedTris( m_MeshBuffer->GetVertexBuffer() , m_MeshBuffer->GetIndexBuffer() , 0 , EG_To<eg_uint>(m_MeshBuffer->GetVertexBufferDataCount()) , EG_To<eg_uint>(m_MeshBuffer->GetIndexBufferDataCount())/3 );
		MainDisplayList->PopDefaultShader();
	}
}

void EGTerrainMesh::UpdateLOD( eg_real DeltaTime , const eg_vec4& CameraPos , const eg_transform& WorldPose )
{
	unused( DeltaTime );

	const eg_real DistThreshold = (m_Dims.x/m_SegDivs) * m_LODThresholdPct;
	eg_vec2 NewLODPos = (CameraPos * WorldPose.GetInverse()).XZ_ToVec2();
	eg_bool bMovedEnoughToForceUpdate = (NewLODPos - m_LODPos).LenSq() > (DistThreshold*DistThreshold);
	m_LODPos = NewLODPos;

	eg_bool bUpdatedLODSeg = UpdateLODSegments( bMovedEnoughToForceUpdate );

	if( bUpdatedLODSeg )
	{
		GenerateMesh();
	}
}

void EGTerrainMesh::Unload()
{
	m_Terrain = nullptr;
	m_IsFinalized = false;

	if( m_MeshBuffer )
	{
		EGRenderer::Get().DestroyDynamicBuffer( m_MeshBuffer );
		m_MeshBuffer = nullptr;
	}

	if( m_mtrl )
	{
		EGRenderer::Get().DestroyMaterial(m_mtrl);
	}

	m_mtrl = EGV_MATERIAL_NULL;
}

void EGTerrainMesh::GenerateMesh()
{
	if( nullptr == m_MeshBuffer || nullptr == m_Terrain )
	{
		return;
	}

	EGVDynamicBuffer::LockDynamicBuffers();

	m_SegsToChange.Clear( false );

	// Hi-res segments:
	const eg_ivec2 MinSegVec = eg_ivec2(EG_Min(m_LODSegIn.x,m_LODSegDiag.x),EG_Min(m_LODSegIn.y,m_LODSegDiag.y));
	const eg_ivec2 MaxSegVec = eg_ivec2(EG_Max(m_LODSegIn.x,m_LODSegDiag.x),EG_Max(m_LODSegIn.y,m_LODSegDiag.y));
	const egTerrainMeshSegId SegIdLL( MinSegVec );
	const egTerrainMeshSegId SegIdUR( MaxSegVec );
	const egTerrainMeshSegId SegIdUL( eg_ivec2( MinSegVec.x , MaxSegVec.y ) );
	const egTerrainMeshSegId SegIdLR( eg_ivec2( MaxSegVec.x , MinSegVec.y ) );

	auto IsHiRes = [&SegIdLL,&SegIdUR,&SegIdUL,&SegIdLR]( const egTerrainMeshSegId& CompareId ) -> eg_bool
	{
		return CompareId == SegIdLL || CompareId == SegIdUR || CompareId == SegIdUL || CompareId == SegIdLR;
	};

	// First generate a list of all the segments that have changed and free
	// their buffers.
	for( eg_uint x=0; x<m_SegDivs; x++ )
	{
		for( eg_uint y=0; y<m_SegDivs; y++ )
		{
			egTerrainMeshSegId SegId( eg_ivec2(x,y) );
			if( IsHiRes( SegId ) )
			{
				if( !m_HiSegs.Contains( SegId ) )
				{
					m_SegsToChange.Append( SegId );
					// If this was currently a lo-seg free it.
					if( m_LoSegs.Contains( SegId ) )
					{
						m_FreeSegs.Append( m_LoSegs[SegId] );
						m_LoSegs.Delete( SegId );
					}
				}
			}
			else
			{
				if( !m_LoSegs.Contains( SegId ) )
				{
					m_SegsToChange.Append( SegId );
					// If this was currently a hi-seg free it.
					if( m_HiSegs.Contains( SegId ) )
					{
						m_FreeSegs.Append( m_HiSegs[SegId] );
						m_HiSegs.Delete( SegId );
					}
				}
			}
		}
	}

	const eg_vec2 SegDim = m_Dims/EG_To<eg_real>(m_SegDivs);

	auto BuildSeg = [this,&IsHiRes,&SegDim]( const egTerrainMeshSegId& SegToBuild ) -> void
	{
		assert( !m_HiSegs.Contains( SegToBuild ) && !m_LoSegs.Contains( SegToBuild ) );

		const eg_bool bIsHi = IsHiRes( SegToBuild );
		egTerrainMeshSegment FoundSeg;
		for( eg_size_t i=0; i<m_FreeSegs.Len(); i++ )
		{
			if( m_FreeSegs[i].bIsHi == bIsHi )
			{
				FoundSeg = m_FreeSegs[i];
				m_FreeSegs.DeleteByIndex( i );
				break;
			}
		}

		assert( FoundSeg.IsValid() && FoundSeg.bIsHi == bIsHi ); // Something in algorithm is wrong.
		if( FoundSeg.IsValid() )
		{
			const eg_vec2 SegMin = eg_vec2( m_Min.x + SegToBuild.Loc.x*SegDim.x , m_Min.y + SegToBuild.Loc.y*SegDim.y );
			const eg_vec2 SegMax = SegMin + SegDim;
			eg_uint FIELD_SIZE = 0;
			if( bIsHi )
			{
				FIELD_SIZE = m_SegDimHi.GetDim();
				m_HiSegs.Insert( SegToBuild , FoundSeg );
			}
			else
			{
				FIELD_SIZE = m_SegDimLo.GetDim();
				m_LoSegs.Insert( SegToBuild , FoundSeg );
			}

			const eg_vec2 FieldRange = eg_vec2( 0.f , EG_To<eg_real>(FIELD_SIZE) );

			for( eg_uint x=0; x <= FIELD_SIZE; x++ )
			{
				for( eg_uint y=0; y <= FIELD_SIZE; y++ )
				{
					eg_real xPos = EGMath_GetMappedRangeValue( static_cast<eg_real>(x) , FieldRange , eg_vec2(SegMin.x,SegMax.x));
					eg_real zPos = EGMath_GetMappedRangeValue( static_cast<eg_real>(y) , FieldRange , eg_vec2(SegMin.y,SegMax.y));

					eg_size_t VertexOffset = x + (FIELD_SIZE+1)*y;
					assert( (bIsHi && VertexOffset < m_SegDimHi.GetNumVerts()) || (!bIsHi && VertexOffset < m_SegDimLo.GetNumVerts() ) );
					egv_vert_terrain& v = FoundSeg.Verts[VertexOffset];
					v = m_Terrain->GetVertexAt( xPos , zPos );
					if( false )
					{
						if( FoundSeg.bIsHi )
						{
							v.Pos.y -= 2.f;
							// v.Color0 = eg_color::Green;
						}
						else
						{
							// v.Color0 = eg_color::Red;
						}
					}
				}
			}
		}
	};

	for( const egTerrainMeshSegId& SegToChange : m_SegsToChange )
	{
		BuildSeg( SegToChange );
	}

	assert( m_FreeSegs.Len() == 0 );

	GenerateMesh_GenerateSeams();

	EGVDynamicBuffer::UnlockDynamicBuffers();

	EGRenderer::Get().UpdateDynamicBuffer( m_MeshBuffer );
}

void EGTerrainMesh::GenerateMesh_GenerateSeams()
{
	const eg_ivec2 MinSegVec = eg_ivec2(EG_Min(m_LODSegIn.x,m_LODSegDiag.x),EG_Min(m_LODSegIn.y,m_LODSegDiag.y));
	const eg_ivec2 MaxSegVec = eg_ivec2(EG_Max(m_LODSegIn.x,m_LODSegDiag.x),EG_Max(m_LODSegIn.y,m_LODSegDiag.y));
	const egTerrainMeshSegId SegIdLL( MinSegVec );
	const egTerrainMeshSegId SegIdUR( MaxSegVec );
	const egTerrainMeshSegId SegIdUL( eg_ivec2( MinSegVec.x , MaxSegVec.y ) );
	const egTerrainMeshSegId SegIdLR( eg_ivec2( MaxSegVec.x , MinSegVec.y ) );

	const eg_uint NUM_INDS_PER_SEAM = m_SegDimHi.GetDim()*3 + m_SegDimLo.GetDim()*3;
	const eg_uint BigVertSpacing = m_SegDimHi.GetDim() / m_SegDimLo.GetDim();
	eg_uint NextIndex = 0;

	auto BuildSeam = [this,&NextIndex,&BigVertSpacing]( const egv_index FirstLoVert , const egv_index FirstHiVert , const egv_index RotA , const egv_index RotB , const egv_index LoSpacing , const egv_index HiSpacing ) -> void
	{
		egv_index CurLoVert = FirstLoVert;
		
		// Form the hi to low triangles
		for( egv_index i=0; i<m_SegDimHi.GetDim(); i++ )
		{
			m_SeamInds[NextIndex+0] = FirstHiVert + HiSpacing*(i + RotA);
			m_SeamInds[NextIndex+1] = FirstHiVert + HiSpacing*(i + RotB);
			m_SeamInds[NextIndex+2] = CurLoVert;
			NextIndex+=3;

			if( 0 == ((i%BigVertSpacing) - BigVertSpacing/2) )
			{
				m_SeamInds[NextIndex+0] = CurLoVert + LoSpacing*RotB;
				m_SeamInds[NextIndex+1] = CurLoVert + LoSpacing*RotA;
				m_SeamInds[NextIndex+2] = FirstHiVert + HiSpacing*(i + 1);
				CurLoVert+=LoSpacing;
				NextIndex+=3;
			}
		}

		assert( NextIndex <= m_NumSeamInds );
	};

	auto BuildEmptySeam = [this,&NextIndex,&NUM_INDS_PER_SEAM]() -> void
	{
		// No adjacent seam, just put everything to a single vertex.
		for( eg_uint i=0; i<NUM_INDS_PER_SEAM; i++ )
		{
			m_SeamInds[NextIndex + i] = 0;
		}
		NextIndex += NUM_INDS_PER_SEAM;

		assert( NextIndex <= m_NumSeamInds );
	};

	auto BuildSeamBeneath  = [this,&BuildSeam,&BuildEmptySeam]( const egTerrainMeshSegId& SegId ) -> void
	{
		const egTerrainMeshSegId AdjSegId( eg_ivec2(SegId.Loc.x,SegId.Loc.y-1) );

		if( m_HiSegs.Contains(SegId) && m_LoSegs.Contains(AdjSegId) )
		{
			const egTerrainMeshSegment& Seg = m_HiSegs[SegId];
			const egTerrainMeshSegment& AdjSeg = m_LoSegs[AdjSegId];

			const egv_index FirstHiVert = EG_To<egv_index>(Seg.FirstVertIdx); // Very first vertex
			const egv_index LastLoRowOffset = EG_To<egv_index>((m_SegDimLo.GetDim()+1) * m_SegDimLo.GetDim());
			const egv_index FirstLoVert = EG_To<egv_index>(AdjSeg.FirstVertIdx + LastLoRowOffset ); // First vertex of last row

			BuildSeam( FirstLoVert , FirstHiVert , 0 , 1 , 1 , 1 );
		}
		else
		{
			BuildEmptySeam();
		}
	};

	auto BuildSeamAbove  = [this,&BuildSeam,&BuildEmptySeam]( const egTerrainMeshSegId& SegId ) -> void
	{
		const egTerrainMeshSegId AdjSegId( eg_ivec2(SegId.Loc.x,SegId.Loc.y+1) );

		if( m_HiSegs.Contains(SegId) && m_LoSegs.Contains(AdjSegId) )
		{
			const egTerrainMeshSegment& Seg = m_HiSegs[SegId];
			const egTerrainMeshSegment& AdjSeg = m_LoSegs[AdjSegId];

			const egv_index LastHiRowOffset = EG_To<egv_index>((m_SegDimHi.GetDim()+1) * m_SegDimHi.GetDim());
			const egv_index FirstHiVert = EG_To<egv_index>(Seg.FirstVertIdx + LastHiRowOffset); // Very first vertex
			const egv_index FirstLoVert = EG_To<egv_index>(AdjSeg.FirstVertIdx); // First vertex of last row

			BuildSeam( FirstLoVert , FirstHiVert , 1 , 0 , 1 , 1 );
		}
		else
		{
			BuildEmptySeam();
		}
	};

	auto BuildSeamRight  = [this,&BuildSeam,&BuildEmptySeam]( const egTerrainMeshSegId& SegId ) -> void
	{
		const egTerrainMeshSegId AdjSegId( eg_ivec2(SegId.Loc.x+1,SegId.Loc.y) );

		if( m_HiSegs.Contains(SegId) && m_LoSegs.Contains(AdjSegId) )
		{
			const egTerrainMeshSegment& Seg = m_HiSegs[SegId];
			const egTerrainMeshSegment& AdjSeg = m_LoSegs[AdjSegId];

			const egv_index FirstHiVert = EG_To<egv_index>(Seg.FirstVertIdx+m_SegDimHi.GetDim());
			const egv_index FirstLoVert = EG_To<egv_index>(AdjSeg.FirstVertIdx);

			BuildSeam( FirstLoVert , FirstHiVert , 0 , 1 , EG_To<egv_index>(m_SegDimLo.GetDim()+1) , EG_To<egv_index>(m_SegDimHi.GetDim()+1) );
		}
		else
		{
			BuildEmptySeam();
		}
	};

	auto BuildSeamLeft  = [this,&BuildSeam,&BuildEmptySeam]( const egTerrainMeshSegId& SegId ) -> void
	{
		const egTerrainMeshSegId AdjSegId( eg_ivec2(SegId.Loc.x-1,SegId.Loc.y) );

		if( m_HiSegs.Contains(SegId) && m_LoSegs.Contains(AdjSegId) )
		{
			const egTerrainMeshSegment& Seg = m_HiSegs[SegId];
			const egTerrainMeshSegment& AdjSeg = m_LoSegs[AdjSegId];

			const egv_index FirstHiVert = EG_To<egv_index>(Seg.FirstVertIdx);
			const egv_index FirstLoVert = EG_To<egv_index>(AdjSeg.FirstVertIdx+m_SegDimLo.GetDim());

			BuildSeam( FirstLoVert , FirstHiVert , 1 , 0 , EG_To<egv_index>(m_SegDimLo.GetDim()+1) , EG_To<egv_index>(m_SegDimHi.GetDim()+1) );
		}
		else
		{
			BuildEmptySeam();
		}
	};

	BuildSeamBeneath( SegIdLL );
	BuildSeamBeneath( SegIdLR );
	BuildSeamAbove( SegIdUL );
	BuildSeamAbove( SegIdUR );
	BuildSeamRight( SegIdLR );
	BuildSeamRight( SegIdUR );
	BuildSeamLeft( SegIdLL );
	BuildSeamLeft( SegIdUL );

	assert( NextIndex == m_NumSeamInds );
}

eg_bool EGTerrainMesh::UpdateLODSegments( eg_bool bIgnoreThreshold )
{
	// Note that the LOD segment is multiplied by 2, this is because we actually
	// want segments divided in such a way to know which quarter of the segment
	// we are in.
	eg_ivec2 NewLODSeg2X = CT_Clear;
	NewLODSeg2X.x = EG_To<eg_int>(EGMath_GetMappedRangeValue( m_LODPos.x , eg_vec2(m_Min.x,m_Min.x+m_Dims.x)  , eg_vec2(0,EG_To<eg_real>(m_SegDivs*2)) ));
	NewLODSeg2X.y = EG_To<eg_int>(EGMath_GetMappedRangeValue( m_LODPos.y , eg_vec2(m_Min.y,m_Min.y+m_Dims.y) , eg_vec2(0,EG_To<eg_real>(m_SegDivs*2)) ));
	NewLODSeg2X.x = EG_Clamp<eg_int>( NewLODSeg2X.x , 0 , m_SegDivs*2-1 );
	NewLODSeg2X.y = EG_Clamp<eg_int>( NewLODSeg2X.y , 0 , m_SegDivs*2-1 );
	
	// We use a threshold so that if the camera is resting right on the segment
	// line, but moving around a bit it doesn't generate new LODs too often.
	eg_bool bFarEnoughInSegmentToUpdateX = true;
	eg_bool bFarEnoughInSegmentToUpdateY = true;
	if( !bIgnoreThreshold && m_LODSeg2X != NewLODSeg2X )
	{
		const eg_int ThreshDivs = m_SegDivs*2;
		eg_vec2 SegInPct = CT_Clear;
		SegInPct.x = EGMath_GetMappedRangeValue( m_LODPos.x , eg_vec2(m_Min.x + NewLODSeg2X.x*m_Dims.x/ThreshDivs,m_Min.x + (NewLODSeg2X.x+1)*m_Dims.x/ThreshDivs) , eg_vec2(0.f,1.f) );
		SegInPct.y = EGMath_GetMappedRangeValue( m_LODPos.y , eg_vec2(m_Min.y + NewLODSeg2X.y*m_Dims.y/ThreshDivs,m_Min.y + (NewLODSeg2X.y+1)*m_Dims.y/ThreshDivs) , eg_vec2(0.f,1.f) );
		SegInPct.x = EG_Clamp( SegInPct.x , 0.f , 1.f );
		SegInPct.y = EG_Clamp( SegInPct.y , 0.f , 1.f );
		bFarEnoughInSegmentToUpdateX = EG_IsBetween( SegInPct.x , m_LODThresholdPct , 1.f-m_LODThresholdPct );
		bFarEnoughInSegmentToUpdateY = EG_IsBetween( SegInPct.y , m_LODThresholdPct , 1.f-m_LODThresholdPct );
		// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , EGString_Format( "LOD Seg Pct: %g %g" , SegInPct.x , SegInPct.y ) );
	}

	eg_bool bXSegChanged = bFarEnoughInSegmentToUpdateX && m_LODSeg2X.x != NewLODSeg2X.x;
	eg_bool bYSegChanged = bFarEnoughInSegmentToUpdateY && m_LODSeg2X.y != NewLODSeg2X.y;

	eg_bool bLODSegmentChanged = bXSegChanged || bYSegChanged;
	if( bLODSegmentChanged )
	{
		m_LODSeg2X = NewLODSeg2X;

		auto GetAdjSegment = [this]( const eg_int& SegIn , const eg_int& Seg ) -> eg_int
		{
			eg_int Out = 0;

			if( SegIn == 0 )
			{
				Out = 1;
			}
			else if( SegIn == m_SegDivs-1 )
			{
				Out = m_SegDivs-2;
			}
			else
			{
				Out = SegIn + (((Seg%2) == 0) ? -1 : 1);
			}

			return Out;
		};

		m_LODSegIn = m_LODSeg2X/2;
		m_LODSegDiag = eg_ivec2( GetAdjSegment( m_LODSegIn.x , m_LODSeg2X.x ) , GetAdjSegment( m_LODSegIn.y , m_LODSeg2X.y ) );

		// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , EGString_Format( "LOD Seg: %d %d Diag Seg: %d %d" , m_LODSegIn.x , m_LODSegIn.y , m_LODSegDiag.x , m_LODSegDiag.y ) );
	}

	return bLODSegmentChanged;
}
