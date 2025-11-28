// (c) 2018 Beem Media

#pragma once

class EGWorldSceneGraph;
class EGCamera2;

struct egWorldRenderPipeDrawSpecs
{
	eg_bool bIsSplitscreen = false;
};

class EGWorldRenderPipe : public EGObject
{
	EG_CLASS_BODY( EGWorldRenderPipe , EGObject )

protected:

	enum class eg_draw_scene_reason
	{
		DRAW_Z_BUFFER,
		DRAW_MAIN_SCENE,
		DRAW_REFLECTED_SCENE,
	};

	enum class eg_draw_thing
	{
		PRE_TERRAIN,
		PRE_MAP,
		PRE_SHADOW,
		PRE_ENT,
	};

	struct egDrawThingInfo
	{
		eg_draw_thing DrawThing;

		union
		{
			struct egMapRegion
			{
				eg_uint RegionId;
				eg_bool bCameraIsInRegion : 1;
			} MapRegion;
		};

		egDrawThingInfo() { zero( this ); }
		egDrawThingInfo( eg_draw_thing Thing ): egDrawThingInfo(){ DrawThing = Thing; }
	};

protected:

	const EGWorldSceneGraph* m_WorldSceneGraph = nullptr;
	eg_real                  m_MaxShadowDistance = 0.f;

public:

	virtual void Update( eg_real DeltaTime ) { unused( DeltaTime ); }
	void DrawSceneGraph( const EGWorldSceneGraph* SceneGraph , const EGCamera2& Camera , const egWorldRenderPipeDrawSpecs& DrawSpecs );
	virtual void DrawDebugItems( class EGDisplayList* DisplayList ) { unused(DisplayList); }

protected:

	virtual void OnDrawEvent( const struct egDrawThingInfo& DrawThingInfo, class EGDisplayList* DisplayList );
	virtual void DrawScene( eg_draw_scene_reason Reason );
	virtual void DrawPrimaryScene( const EGCamera2& Camera );
	virtual void DrawWorld( EGDisplayList* DisplayList , const EGCamera2& Camera );
	void DrawMap();
	void DrawTerrain();
	void DrawEnts( eg_string_crc Filter );
	void DrawShadows( const EGCamera2& Camera );
	void Draw_Scene_Reflection( const EGCamera2& Camera );
	void Draw_DebugDrawMapOnly();
};
