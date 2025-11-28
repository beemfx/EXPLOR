// (c) 2017 Beem Media

#include "ExMapping.h"
#include "ExGame.h"
#include "EGRenderer.h"
#include "EGFileData.h"

EG_CLASS_DECL( ExMapping )

void ExMapping::OnDestruct()
{
	SetMap( CT_Clear );
}

void ExMapping::SetOwner( ExGame* OwningGame )
{
	m_OwningGame = OwningGame;
}

void ExMapping::SetMap( const eg_string_crc& MapId )
{
	if( m_CurrentMapId == MapId )
	{
		return;
	}

	m_CachedSquaresRevealed = -1;
	m_CurrentMapReveal = nullptr;
	m_CurrentMapId = MapId;
	m_MapInfo = m_CurrentMapId.IsNotNull() ? ExMapInfos::Get().FindInfo( m_CurrentMapId ) : exMapInfoData();

	SetupCurrentRevealData();
	SetupMapGraphics();
	m_bNeedsDraw = true;
}

void ExMapping::SetGlobalSamplerToMap()
{
	MainDisplayList->SetGlobalSamplers( m_MapRender , egv_rtarget::Null , egv_rtarget::Null );
}

eg_ivec2 ExMapping::GetMapSize() const
{
	return eg_ivec2( static_cast<eg_int>(m_MapInfo.MapMetaInfo.MapSize.x) , static_cast<eg_int>(m_MapInfo.MapMetaInfo.MapSize.y) );
}

eg_ivec2 ExMapping::GetMapLowerLeftOrigin() const
{
	return eg_ivec2( static_cast<eg_int>(m_MapInfo.MapMetaInfo.LowerLeftOrigin.x) , static_cast<eg_int>(m_MapInfo.MapMetaInfo.LowerLeftOrigin.y) );
}

ex_map_reveal_result ExMapping::RevealPose( const eg_transform& WorldPose )
{
	ex_map_reveal_result Result = ex_map_reveal_result::None;
	exGridPose GridPos = WorldPoseToGridPos( WorldPose );
	eg_ivec2 RawOffset = GridPos.GridPos - GetMapLowerLeftOrigin();

	if( m_CurrentMapReveal && !m_CurrentMapReveal->IsRevealed( RawOffset ) )
	{
		m_CachedSquaresRevealed++;
		m_CurrentMapReveal->SetRevealed( RawOffset , true );
		m_bNeedsDraw = true;
		Result = ex_map_reveal_result::NewlyRevealed;

		if( m_OwningGame && m_OwningGame->IsServer() )
		{
			m_OwningGame->SDK_RunClientEvent( eg_crc("RevealMapSquare") , eg_event_parms( GridPos.GridPos ) );
		}
	}
	else
	{
		Result = ex_map_reveal_result::AlreadyRevealed;
	}

	return Result;
}

void ExMapping::RevealReplicatedSquare( const eg_event_parms& Parms )
{
	eg_ivec2 RawOffset = Parms.as_ivec2() - GetMapLowerLeftOrigin();

	if( m_CurrentMapReveal && !m_CurrentMapReveal->IsRevealed( RawOffset ) )
	{
		m_CurrentMapReveal->SetRevealed( RawOffset , true );
		m_bNeedsDraw = true;
	}
}

exGridPose ExMapping::WorldPoseToGridPos( const eg_transform& WorldPose ) const
{
	exGridPose Out( CT_Clear );

	Out.GridFace = ExGame::PoseToFace( WorldPose );
	Out.GridPos.x = EGMath_floor( WorldPose.GetPosition().x/m_MapInfo.MapMetaInfo.TileSize.x );
	Out.GridPos.y = EGMath_floor( WorldPose.GetPosition().z/m_MapInfo.MapMetaInfo.TileSize.y );

	return Out;
}

eg_bool ExMapping::IsPoseRevealed( const exGridPose& GridPose ) const
{
	eg_ivec2 RawOffset = GridPose.GridPos - GetMapLowerLeftOrigin();
	eg_bool bAllRevealed = m_MapInfo.MapType == ex_map_info_map_t::Town;
	return bAllRevealed || ( m_CurrentMapReveal && m_CurrentMapReveal->IsRevealed( RawOffset ) );
}

eg_int ExMapping::GetNumSquaresRevealed() const
{
	if( m_CachedSquaresRevealed < 0 )
	{
		m_CachedSquaresRevealed = m_CurrentMapReveal ? m_CurrentMapReveal->GetNumSquaresRevealed() : 0;
	}

	return m_CachedSquaresRevealed;
}

void ExMapping::ReplicateCurrentMap()
{
	m_CachedSquaresRevealed = -1;
	if( m_CurrentMapReveal && m_OwningGame && m_OwningGame->IsServer() )
	{
		for( eg_int x = 0; x < m_MapInfo.MapMetaInfo.MapSize.x; x++ )
		{
			for( eg_int y = 0; y < m_MapInfo.MapMetaInfo.MapSize.y; y++ )
			{
				eg_ivec2 RawPos( x , y );
				if( m_CurrentMapReveal->IsRevealed( RawPos ) )
				{
					eg_ivec2 GridPos = RawPos + GetMapLowerLeftOrigin();
					m_OwningGame->SDK_RunClientEvent( eg_crc("RevealMapSquare") , eg_event_parms( GridPos ) );
				}
			}
		}
	}
}

void ExMapping::ClientBeginFullReplication( eg_string_crc MapId )
{
	SetMap( CT_Clear );
	m_ClientMapReveal = exMappingRevealed();
	SetMap( MapId );
}

void ExMapping::Draw()
{
	if( !m_bNeedsDraw )
	{
		return;
	}

	m_bNeedsDraw = false;

	if( m_MapRender != egv_rtarget::Null && MainDisplayList )
	{
		eg_vec2 ScaleSize( 1.f/m_MapInfo.MapMetaInfo.MapSize.x , 1.f/m_MapInfo.MapMetaInfo.MapSize.y );
		eg_vec2 LowerLeft( (1.f-m_MapInfo.MapMetaInfo.MapSize.x)*ScaleSize.x , (1.f-m_MapInfo.MapMetaInfo.MapSize.y)*ScaleSize.y );
		eg_bool bAllRevealed = m_MapInfo.MapType == ex_map_info_map_t::Town;

		auto DrawBlock = [this,&ScaleSize,&LowerLeft]( eg_int x , eg_int y ) -> void
		{
			eg_mat Transform( CT_Default );
			Transform *= eg_mat::BuildScaling( eg_vec3( ScaleSize.x , ScaleSize.y , 1.f ) );
			Transform *= eg_mat::BuildTranslation( eg_vec3(LowerLeft.x + x*ScaleSize.x*2.f , LowerLeft.y + y*ScaleSize.y*2.f , 0.f ) );
			MainDisplayList->SetWorldTF( Transform );
			MainDisplayList->DrawBasicQuad();
		};
		
		MainDisplayList->ClearRT( eg_color(0.f,0.f,0.f,0.f) );
		MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF );
		MainDisplayList->PushDefaultShader( eg_defaultshader_t::TEXTURE );
		MainDisplayList->PushSamplerState( eg_sampler_s::TEXTURE_CLAMP_FILTER_POINT );
		MainDisplayList->SetMaterial( m_MapImage );
		MainDisplayList->SetWorldTF( eg_mat::I );
		MainDisplayList->SetProjTF( eg_mat::I );
		MainDisplayList->SetViewTF( eg_mat::I );
		MainDisplayList->DrawBasicQuad();

		MainDisplayList->SetMaterial( m_BlockSquare );
		MainDisplayList->SetVec4( eg_rv4_t::ENT_PALETTE_0 , eg_color(0.f,0.f,0.f,1.f).ToVec4() );

		if( m_CurrentMapReveal )
		{
			for( eg_int x = 0; x<m_MapInfo.MapMetaInfo.MapSize.x; x++ )
			{
				for( eg_int y=0; y<m_MapInfo.MapMetaInfo.MapSize.y; y++ )
				{
					if( !bAllRevealed && !m_CurrentMapReveal->IsRevealed( eg_ivec2( x , y ) ) )
					{
						DrawBlock( x , y );
					}
				}
			}
		}

		MainDisplayList->PopSamplerState();
		MainDisplayList->PopDefaultShader();
		MainDisplayList->PopDepthStencilState();
		MainDisplayList->ResolveToRenderTarget( m_MapRender );
		MainDisplayList->ClearRT( eg_color::Black ); // The screen so we don't see it draw in the viewport when we clamp it.
		MainDisplayList->SetCannotBeDropped();
	}
}

void ExMapping::SaveTo( EGFileData& FileOut )
{
	const eg_string_crc HeaderId = eg_crc("ExMapping");
	const eg_uint32 NumMaps = m_MapRevealDatas.LenAs<eg_uint32>();
	FileOut.Write( &HeaderId , sizeof(HeaderId) );
	FileOut.Write( &NumMaps , sizeof(NumMaps) );
	for( eg_uint32 i=0; i<NumMaps; i++ )
	{
		m_MapRevealDatas[i].SaveTo( FileOut );
	}
}

void ExMapping::LoadFrom( const EGFileData& FileIn )
{
	SetMap( CT_Clear );
	m_MapRevealDatas.Clear();

	eg_string_crc HeaderId = CT_Clear;
	FileIn.Read( &HeaderId , sizeof(HeaderId) );

	if( HeaderId != eg_crc("ExMapping") )
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Header was incorrect, bad data." );
		assert( false );
		return;
	}

	eg_uint32 NumMaps = 0;
	FileIn.Read( &NumMaps , sizeof(NumMaps) );
	m_MapRevealDatas.Resize( NumMaps );
	for( eg_uint32 i=0; i<NumMaps; i++ )
	{
		m_MapRevealDatas[i].LoadFrom( FileIn );
	}
}

void ExMapping::SetupMapGraphics()
{
	if( m_MapImage != EGV_MATERIAL_NULL )
	{
		EGRenderer::Get().DestroyMaterial( m_MapImage );
		m_MapImage = EGV_MATERIAL_NULL;
	}

	if( m_BlockSquare != EGV_MATERIAL_NULL )
	{
		EGRenderer::Get().DestroyMaterial( m_BlockSquare );
		m_BlockSquare = EGV_MATERIAL_NULL;
	}

	if( m_MapRenderMaterial != EGV_MATERIAL_NULL )
	{
		EGRenderer::Get().DestroyMaterial( m_MapRenderMaterial );
		m_MapRenderMaterial = EGV_MATERIAL_NULL;
		MapMaterialChangedDelegate.Broadcast( m_MapRenderMaterial );
	}

	if( m_MapRender != egv_rtarget::Null )
	{
		EGRenderer::Get().DestroyRenderTarget( m_MapRender );
		m_MapRender = egv_rtarget::Null;
	}

	if( m_MapInfo.MapMetaInfo.MiniMap.FullPath.Len() > 0 && m_OwningGame && m_OwningGame->IsClient() )
	{
		{
			EGMaterialDef MatDef;
			eg_string s(*m_MapInfo.MapMetaInfo.MiniMap.FullPath);
			s.CopyTo(MatDef.m_strTex[0], EG_MAX_PATH);
			eg_string SharedId = EGString_Format( "MappingTx(%s)" , s.String() );
			m_MapImage = EGRenderer::Get().CreateMaterial( &MatDef , SharedId );
		}

		{
			EGMaterialDef MatDef;
			eg_string s("/game/ui/shared/BlockSquare");
			s.CopyTo(MatDef.m_strTex[0], EG_MAX_PATH);
			eg_string SharedId = EGString_Format( "MappingBlockTx(%s)" , s.String() );
			m_BlockSquare = EGRenderer::Get().CreateMaterial( &MatDef , SharedId );
		}

		{
			EGMaterialDef MatDef;
			eg_string s(*m_MapInfo.MapMetaInfo.MiniMap.FullPath);
			s.CopyTo(MatDef.m_strTex[0], EG_MAX_PATH);
			eg_string SharedId = EGString_Format( "MapRenderTx(%s)" , s.String() );
			MatDef.SetShader( "/game/shaders/ex_uimap" );
			m_MapRenderMaterial = EGRenderer::Get().CreateMaterial( &MatDef , SharedId );
			MapMaterialChangedDelegate.Broadcast( m_MapRenderMaterial );

			const eg_uint ImageWidth = AUTOMAP_TILE_WIDTH * static_cast<eg_uint>(m_MapInfo.MapMetaInfo.MapSize.x);
			const eg_uint ImageHeight = AUTOMAP_TILE_WIDTH * static_cast<eg_uint>(m_MapInfo.MapMetaInfo.MapSize.y);
			m_MapRender = EGRenderer::Get().CreateRenderTarget( ImageWidth , ImageHeight );
		}
	}
}

void ExMapping::SetupCurrentRevealData()
{
	m_CurrentMapReveal = nullptr;
	m_CachedSquaresRevealed = -1;

	if( m_CurrentMapId.IsNull() )
	{
		return;
	}

	if( m_OwningGame && m_OwningGame->IsClient() )
	{
		m_ClientMapReveal.InitForMapping( GetMapId() , GetMapSize() );
		m_CurrentMapReveal = &m_ClientMapReveal;
		return;
	}

	for( exMappingRevealed& RevealData : m_MapRevealDatas )
	{
		if( RevealData.GetMapId() == m_CurrentMapId )
		{
			m_CurrentMapReveal = &RevealData;
			break;
		}
	}

	if( nullptr == m_CurrentMapReveal )
	{
		exMappingRevealed NewData;
		m_MapRevealDatas.Resize( m_MapRevealDatas.Len() + 1 );
		m_CurrentMapReveal = &m_MapRevealDatas[m_MapRevealDatas.Len()-1];
	}

	m_CurrentMapReveal->InitForMapping( GetMapId() , GetMapSize() );
}

void exMappingRevealed::InitForMapping( const eg_string_crc& InMapId, const eg_ivec2& InMapSize )
{
	MapId = InMapId;
	if( MapSize.x != InMapSize.x || MapSize.y != InMapSize.y )
	{
		if( MapSize.x != 0 || MapSize.y != 0 )
		{
			EGLogf( eg_log_t::Warning , "A map was resized. All automap data has been reset." );
		}

		MapSize = InMapSize;
		eg_int TotalSize = MapSize.x * MapSize.y;
		if( TotalSize > 0 )
		{
			RevealedArray.Resize( TotalSize );
			for( eg_bool& bRevealed : RevealedArray )
			{
				bRevealed = false;
			}
		}
	}
}

eg_bool exMappingRevealed::IsRevealed( const eg_ivec2& Pos ) const
{
	eg_int Index = PosToIndex( Pos );
	return RevealedArray.IsValidIndex( Index ) ? RevealedArray[Index] : true;
}

void exMappingRevealed::SetRevealed( const eg_ivec2& Pos, eg_bool bRevealed )
{
	eg_int Index = PosToIndex( Pos );
	if( RevealedArray.IsValidIndex( Index ) )
	{
		RevealedArray[Index] = bRevealed;
	}
	else
	{
		assert( false );
	}
}

eg_int exMappingRevealed::GetNumSquaresRevealed() const
{
	eg_int Count = 0;

	for( eg_bool bValue : RevealedArray )
	{
		if( bValue )
		{
			Count++;
		}
	}

	return Count;
}

void exMappingRevealed::SaveTo( EGFileData& FileOut )
{
	FileOut.Write( &MapId , sizeof(MapId) );
	FileOut.Write( &MapSize , sizeof(MapSize) );
	FileOut.Write( RevealedArray.GetArray() , RevealedArray.Len() );
}

void exMappingRevealed::LoadFrom( const EGFileData& FileIn )
{
	InitForMapping( CT_Clear , eg_ivec2(0,0) );

	eg_string_crc ReadMapId = CT_Clear;
	eg_ivec2 ReadMapSize(0,0);
	FileIn.Read( &ReadMapId , sizeof(ReadMapId ) );
	FileIn.Read( &ReadMapSize , sizeof(ReadMapSize) );
	InitForMapping( ReadMapId , ReadMapSize );
	assert( RevealedArray.Len() == MapSize.x * MapSize.y );
	if( RevealedArray.Len() > 0 )
	{
		FileIn.Read( RevealedArray.GetArray() , RevealedArray.Len() );
	}
}

eg_int exMappingRevealed::PosToIndex( const eg_ivec2& Pos ) const
{
	return Pos.y * MapSize.x + Pos.x;
}
