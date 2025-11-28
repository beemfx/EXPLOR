// (c) 2018 Beem Media

#include "EGWorldSceneGraph.h"
#include "EGGameMap.h"
#include "EGRenderer.h"
#include "EGTerrainMesh.h"
#include "EGVisualComponent.h"
#include "EGEnt.h"
#include "EGMeshShadowComponent.h"
#include "EGCamera2.h"
#include "EGGlobalConfig.h"

EG_CLASS_DECL( EGWorldSceneGraph )

void EGWorldSceneGraph::ResetGraph( const EGCamera2& Camera )
{
	// We don't delete the pass filters.
	for( eg_size_t i=0; i<m_CompMap.Len(); i++ )
	{
		m_CompMap.GetByIndex( i ).Clear( false );
	}
	m_ReflectiveCompList.Clear( false );
	m_ShadowList.Clear( false );
	m_MapRegionsList.Clear( false );
	m_TerrainList.Clear( false );

	m_CameraPose = Camera.GetPose();

	m_bDebugWireFrame = DebugConfig_DrawWireframe.GetValue();
}

void EGWorldSceneGraph::AddItem( const egWorldSceneGraphComponent& NewComp )
{
	m_CompMap[NewComp.RenderFilter].Append( NewComp ); // Will create new filters as necessary.
	
	if( NewComp.bIsReflective )
	{
		m_ReflectiveCompList.Append( NewComp );
		m_ReflectiveCompList.Sort( [this]( const egWorldSceneGraphComponent& Left , const egWorldSceneGraphComponent& Right ) -> eg_bool
			{
				eg_real LeftDistSq = (Left.WorldPose.GetPosition() - m_CameraPose.GetPosition()).LenSqAsVec3();
				eg_real RightDistSq = (Right.WorldPose.GetPosition() - m_CameraPose.GetPosition()).LenSqAsVec3();
				return LeftDistSq < RightDistSq;
			} );
		if( m_ReflectiveCompList.Len() > m_MaxReflectiveEnts )
		{
			m_ReflectiveCompList.Resize( m_MaxReflectiveEnts );
		}
	}
}

void EGWorldSceneGraph::AddItem( const egWorldSceneGraphShadow& NewComp )
{
	m_ShadowList.Append( NewComp );
}

void EGWorldSceneGraph::AddItem( const egWorldSceneGraphMapRegion& NewMapRegion )
{
	m_MapRegionsList.Append( NewMapRegion );
}

void EGWorldSceneGraph::AddItem( const egWorldSceneGraphTerrain& NewTerrain )
{
	m_TerrainList.Append( NewTerrain );
}

void EGWorldSceneGraph::DrawMapRegions( EGPreDrawFn PreDrawCb ) const
{
	if( m_bDebugWireFrame )
	{
		MainDisplayList->PushRasterizerState( eg_rasterizer_s::WIREFRAME );
	}

	for( const egWorldSceneGraphMapRegion& MapRegion : m_MapRegionsList )
	{
		if( PreDrawCb )
		{
			PreDrawCb();
		}
		assert( MapRegion.GameMap && MapRegion.GameMap->IsLoaded() );
		MapRegion.GameMap->DrawRegion( MapRegion.RegionIndex );
	}

	if( m_bDebugWireFrame )
	{
		MainDisplayList->PopRasterizerState();
	}
}

void EGWorldSceneGraph::DrawTerrains( EGPreDrawFn PreDrawCb ) const
{
	if( m_bDebugWireFrame )
	{
		MainDisplayList->PushRasterizerState( eg_rasterizer_s::WIREFRAME );
	}

	// For now just draw all terrains.
	for( const egWorldSceneGraphTerrain& Terrain : m_TerrainList )
	{
		if( PreDrawCb )
		{
			PreDrawCb();
		}

		Terrain.TerrainMesh->Draw( Terrain.WorldPose );
	}

	if( m_bDebugWireFrame )
	{
		MainDisplayList->PopRasterizerState();
	}
}

void EGWorldSceneGraph::DrawComponents( const eg_string_crc& RenderFilter , EGPreDrawFn PreDrawCb ) const
{
	auto DrawCompList = [this,&PreDrawCb]( const EGCompList& ListToDraw ) -> void
	{
		for( const egWorldSceneGraphComponent& Comp : ListToDraw )
		{
			SetLightingForEnt( MainDisplayList , Comp.EntOwner );
			if( PreDrawCb )
			{
				PreDrawCb();
			}

			if( m_bDebugWireFrame )
			{
				MainDisplayList->PushRasterizerState( eg_rasterizer_s::WIREFRAME );
			}
			Comp.Component->Draw( Comp.WorldPose );
			if( m_bDebugWireFrame )
			{
				MainDisplayList->PopRasterizerState();
			}
		}
	};

	if( RenderFilter.IsNull() )
	{
		for( eg_size_t i=0; i<m_CompMap.Len(); i++ )
		{
			const EGCompList& CompList = m_CompMap.GetByIndex( i );
		 	DrawCompList( CompList );
		}
	}
	else
	{
		if( m_CompMap.Contains( RenderFilter ) )
		{
			const EGCompList& CompList = m_CompMap[RenderFilter];
			DrawCompList( CompList );
		}
	}
}

void EGWorldSceneGraph::DrawShadows( eg_real FarPln , EGPreDrawFn PreDrawCb /*= nullptr */ ) const
{
	unused( PreDrawCb );

	eg_uint NumShadowsDrawn = 0;
	for( const egWorldSceneGraphShadow& Shadow : m_ShadowList )
	{
		DrawVolumeShadow( MainDisplayList , Shadow.EntOwner , Shadow.Component , FarPln , Shadow.WorldPose );
		NumShadowsDrawn++;
		if( NumShadowsDrawn > m_MaxShadows )
		{
			break;
		}
	}
}

const EGEnt* EGWorldSceneGraph::GetEntForReflectiveDraw( eg_uint Index ) const
{
	const EGEnt* Out = nullptr;

	if( m_ReflectiveCompList.IsValidIndex( Index ) )
	{
		if( m_ReflectiveCompList[Index].EntOwner )
		{
			Out = m_ReflectiveCompList[Index].EntOwner;
		}
	}

	return Out;
}

void EGWorldSceneGraph::DrawVolumeShadow( EGDisplayList* DisplayList , const EGEnt* Ent , const EGMeshShadowComponent* ShadowComp , eg_real FarPlaneDistSq , const eg_transform& FullPose ) const
{
	const eg_real SHADOW_VISIBILITY_FROM_FAR_PLANE_PCT = .2f;
	const eg_bool bDebugDrawShadowVolume = DebugConfig_DrawVolShadows.GetValue();

	if( nullptr == Ent )
	{
		return;
	}

	/************************************************************************
	Shadows are one of those things that are easier on paper than they are
	in application. Casting a volume shadow is not a difficult task, the
	difficult part is deciding how to go about casting the shadow, what
	direction should it be cast, and how intense should it be cast.

	The model that I decided upon for shadows is thus. We already have a
	four closest lights quadratic falloff model for the lights, so that is
	used for the basis of the shadow model. The idea goes like this, the
	four closest lights are averaged together to form one light position.
	This position will be used to cast the shadow in an appropriate
	direction. Note that this is not just a basic average, it is a weighted
	averaged based on the intensity of each light. (The lights and their
	intensities are computed in the RenderEnt method, that data is stored
	in the pEnt->m_TreeInfo structure, and is used here).

	This model is all well and good except when an object steps out of a
	light source. In this condition and object wouldn't really cast a shadow
	since it isn't actually standing in any light where a shadow could be
	cast. It seems like we could just turn of the shadow in that case, but
	this will cause the shadow to suddenly shift positions, or disappear
	when the entity moves around. Instead what I do is changed the
	intensity of the shadow. The total intensity of the shadow is based upon
	the total intensity of light shining on the entity. If the intensity is
	low, the shadow is not as dark. If it is high, the shadow is very dark.
	Technically this is not how shadows actually work, but the model looks
	alright because if an object is close to a light source, it blocks a lot
	of light and creates a dark shadow. When the object is further away, more
	light, in general, would be in the area, and that light would be
	reflected across more surfaces. And so the shadow would appear lighter.
	************************************************************************/

	//::EG_OLprintf(("Dropping shadow for %i."), nID);

	const EGEntCloseLights& CloseLights = Ent->GetCloseLights();
	if( CloseLights.Len() == 0 )
	{
		return;
	}

	//The first thing we do is compute the total light intensity, this is the
	//total intensity of all the light shining on the entity. It should
	//be not greater than MAX_LIGHTS since each intensity should
	//have been computed in the range of [0, 1].
	eg_real fTtlWt = 0.0f;
	for(eg_uint i=0; i<CloseLights.Len(); i++)
	{
		fTtlWt += CloseLights[i].Weight;
	}

	//At this point if the total intensity is very small, we can forget the
	//shadow.
	if(fTtlWt<0.001f)
	{
		return;
	}

	eg_real Fade = 1.f;

	eg_real FullOutPct = FarPlaneDistSq*SHADOW_VISIBILITY_FROM_FAR_PLANE_PCT;
	eg_real BeginFadePct = FarPlaneDistSq*SHADOW_VISIBILITY_FROM_FAR_PLANE_PCT*.5f;
	eg_real RenderDistSq = (FullPose.GetPosition() - m_CameraPose.GetPosition()).LenSqAsVec3();

	if( RenderDistSq >= BeginFadePct )
	{
		Fade = EGMath_GetMappedRangeValue( RenderDistSq , eg_vec2(BeginFadePct,FullOutPct) , eg_vec2(1.f,0.f) );
		Fade = EG_Clamp( Fade , 0.f , 1.f );
	}

	//If the total weight is very small we drop a shadow only from the first
	//light (probably shouldn't drop at all).
	eg_vec4 vLtPos(0,0,0,1);

	for(eg_uint i=0; i<CloseLights.LenAs<eg_uint>(); i++)
	{
		const eg_real fCntrb = CloseLights[i].Weight/fTtlWt;//(fTtlWt>0.01f)?(afWts[i]/fTtlWt):(1.0f/nLts);

		vLtPos.x += fCntrb*CloseLights[i].Pos.x;
		vLtPos.y += fCntrb*CloseLights[i].Pos.y;
		vLtPos.z += fCntrb*CloseLights[i].Pos.z;
	}

	//::EG_OLprintf(("The total light for %i is (%f, %f, %f) (%f)."), nID, v3LtPos.x, v3LtPos.y, v3LtPos.z, fTtlWt);
	//Color for the light doesn't really matter since it isn't used
	//by the shader, only the direction matters.

	//Since shadows flare out we set the direction as the position,
	//the shader knows this so it will work appropriately.
	EGLight lt;
	lt.Pos = vLtPos;
	lt.Dir = Ent->GetBounds().GetCenter();
	lt.Dir -= lt.Pos;
	lt.Dir.NormalizeThisAsVec3();

	DisplayList->SetLight(0, lt); //The 0 light is used by the vertex shader.


												 //Note that this is not the most efficient way to cast shadows since we
												 //are clearing the stencil buffer by itself, and we are rendering a quad
												 //for each shadow, but this seems to be the only way to render each shadow
												 //at it's own intensity.

	if( !bDebugDrawShadowVolume )
	{
		DisplayList->PushBlendState( eg_blend_s::BLEND_NONE_COLOR_NONE );
	}
	DisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF_STEST_SWRITE_MESH_SHADOW );
	DisplayList->PushRasterizerState( eg_rasterizer_s::CULL_NONE );
	DisplayList->PushDefaultShader( eg_defaultshader_t::SHADOW );
	DisplayList->SetMaterial(EGV_MATERIAL_NULL);
	//These are the registry values for shadows:
	//g_vs_reg0.x: Shadow stretch.
	//g_vs_reg0.y: 0 for directional shadows, nonzero for positional shadows.
	//Directional shadows just use the light direction and cast shadows in that
	//direction. Positional ones use the lights position to cast the shadow
	//for each vertex (these can flare out, but if the light is inside the
	//shadow they may appear incorrect).
	DisplayList->SetVec4(eg_rv4_t::FREG_0, eg_vec4(5.0f, 0, 0, 0));

	ShadowComp->ForShadowDrawDraw( FullPose );

	if( !bDebugDrawShadowVolume )
	{
		DisplayList->PopBlendState();
	}
	DisplayList->PopDepthStencilState();
	DisplayList->PopRasterizerState();
	DisplayList->PopDefaultShader();

	//The final color of the shadow is based on the overall intensity.
	//(The intensity should be in a range of 0 to 1)

	//Now the final intensity needs to be scaled by the number of lights.
	//This insures that it is in a range from [0, 1].
	fTtlWt/=CloseLights.LenAs<eg_uint>();
	//We now set the shadows color, we only modify the alpha channel since
	//this the render states determine that the alpha channel alone effects
	//the intensity of the shadow. (We could actually ad color to the shadow
	//if we wanted.)
	fTtlWt *= Fade;
	DisplayList->DrawShadowVolume(eg_color(0,0,0,fTtlWt)); //Drawing the shadow volume clears the shadow stencil, so it won't get drawn the next time around.
}

void EGWorldSceneGraph::SetLightingForEnt( EGDisplayList* DisplayList , const EGEnt* Ent )
{
	if( nullptr == Ent || !Ent->IsLit() )
	{
		DisplayList->DisableAllLights();
		DisplayList->SetVec4( eg_rv4_t::AMBIENT_LIGHT , eg_vec4(1.f,1.f,1.f,1.f) );
		return;
	}
	//When rendering the entity, we'll find the specified number of closest
	//lights, and use them for rendering.

	//We need the position of the entity.
	const eg_vec4 EntPos = Ent->GetBounds().GetCenter();
	assert(1 == EntPos.w);

	const EGEntCloseLights& CloseLights = Ent->GetCloseLights();

	DisplayList->DisableAllLights();

	EGLight RenderLight;

	for(eg_uint i=0; i<CloseLights.LenAs<eg_uint>(); i++)
	{
		const egEntLight& Light = CloseLights[i];
		RenderLight.Pos = eg_vec4( Light.Pos , 1.f );
		RenderLight.Dir = EntPos - RenderLight.Pos;
		RenderLight.Dir.NormalizeThisAsVec3();//Normalize the light direction.

													 //LIGHT BRIGHTNESS FALLOFF MODEL
													 //This is a quadratic falloff model for the light brightness.
													 //It is designed so that if the entity is within range, the brightness
													 //decrease by a square root, that way the brightness will stay fairly 
													 //constant close to the light, and then suddenly drop off at the end of
													 //the range.
													 //The formula is given by 
													 //    (1/R)sqrt(R(R-D)) : (0 <= D < R)
													 //    0                 : (R <= D)
													 //Where R is the range of the light, and D is the distance to the light.
													 //Note that to save on computation the values of R and D are actually
													 //the squared value of these lengths. Thus only one square root is
													 //called per light. Here is the code:
		const eg_real& fDistSq  = Light.DistSq;
		const eg_real& fRangeSq = Light.RangeSq;

		if( true )//fDistSq <= fRangeSq )
		{
			const eg_real  fScale   = (fDistSq < fRangeSq)?(1.0f/fRangeSq)*EGMath_sqrt((fRangeSq*(fRangeSq-fDistSq))):0.0f;

			RenderLight.Color = eg_color(Light.Color);
			RenderLight.Color.a = 1.f;

			RenderLight.SetRangeSq( fRangeSq , fRangeSq*.5f*.5f );
			RenderLight.SetIntensity( fScale );

			//Enable the closest lights.
			DisplayList->SetLight(i, RenderLight);
			DisplayList->EnableLight(i, true);

			Light.Weight = fScale;
		}
		else
		{
			Light.Weight = 0.f;
		}
	}

	DisplayList->SetVec4( eg_rv4_t::AMBIENT_LIGHT , eg_color(Ent->GetAmbientLight()).ToVec4() );
}
