// (c) 2016 Beem Media

#include "EGUiLightWidget.h"
#include "EGUiLayout.h"
#include "EGMenu.h"
#include "EGCamera2.h"
#include "EGDebugShapes.h"
#include "EGGlobalConfig.h"

EG_CLASS_DECL( EGUiLightWidget )

void EGUiLightWidget::Init( EGMenu* InOwner, const egUiWidgetInfo* InInfo )
{
	Super::Init( InOwner , InInfo );

	m_LightColor = eg_color(InInfo->LightInfo.LightColor);
	m_AmbientColor = eg_color(InInfo->LightInfo.AmbientColor);
	m_Range = InInfo->LightInfo.Range;
	m_Falloff = InInfo->LightInfo.Falloff;
	m_DbLightObj = EGNewObject<EGDebugSphere>( eg_mem_pool::Default );
}

EGUiLightWidget::~EGUiLightWidget()
{
	EG_SafeRelease( m_DbLightObj );
}

void EGUiLightWidget::Draw( eg_real AspectRatio )
{
	eg_bool bDrawLighObj = GlobalConfig_IsUiLayoutTool.GetValue() || DebugConfig_DrawCloseLights.GetValue();

	if( bDrawLighObj && m_DbLightObj )
	{		
		eg_real ObjRadius = GetObjRadius();

		eg_transform Pose = EGUiWidget::GetFullPose( AspectRatio );
		eg_vec4 ScaleVec = eg_vec4( ObjRadius , ObjRadius , ObjRadius , 1.f );
		m_DbLightObj->DrawDebugShape( ObjRadius , Pose , m_LightColor );

		if( !m_Info->bIsLocked )
		{
			eg_color Palette = m_LightColor;
			Palette.a = .5f;
			MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF );
			m_DbLightObj->DrawDebugShape( m_Range , Pose , Palette );
			MainDisplayList->PopDepthStencilState();
		}
	}

	EGLight Lt( CT_Clear );
	Lt.Color = m_LightColor;
	Lt.Pos = GetFullPose( AspectRatio ).GetPosition();
	Lt.Dir = eg_vec4(0.f,0.f,1.f,0.f);
	Lt.SetRangeSq( m_Range*m_Range , m_Falloff*m_Falloff );
	Lt.SetIntensity( 1.f );
	m_Owner->AddLight( Lt, m_AmbientColor );
}

void EGUiLightWidget::Update( eg_real DeltaTime, eg_real AspectRatio )
{
	Super::Update( DeltaTime , AspectRatio );

	if( m_DbLightObj )
	{
		m_DbLightObj->Update( DeltaTime );
	}
}

eg_aabb EGUiLightWidget::GetBoundsInMouseSpace( eg_real AspectRatio , const eg_mat& MatView , const eg_mat& MatProj ) const
{
	const eg_transform Pose = GetFullPose(AspectRatio);
	//3) Create a bound box with the specified bounds.

	eg_real ObjRadius = GetObjRadius();

	eg_aabb ButtonDims;
	ButtonDims.CreateFromVec4s( &eg_vec4(-ObjRadius,-ObjRadius,-ObjRadius,1.f) , 1 );
	ButtonDims.AddPoint( eg_vec4(ObjRadius,ObjRadius,ObjRadius,1.f) );

	ButtonDims.Min.w = 1.f;
	ButtonDims.Max.w = 1.f;


	eg_vec4 Corners[8];
	ButtonDims.Get8Corners( Corners , countof(Corners) );

	for( eg_vec4& Corner : Corners )
	{
		Corner *= Pose;

		eg_vec2 CornerMouseSpace = EGCamera2::WorldSpaceToMouseSpace( Corner , AspectRatio , MatView , MatProj );
		Corner.x = CornerMouseSpace.x;
		Corner.y = CornerMouseSpace.y;
		Corner.z = 0.f;
		Corner.w = 1.f;
	}

	ButtonDims.CreateFromVec4s( Corners , countof(Corners) );


	return ButtonDims;
}

void EGUiLightWidget::SetPalette( eg_uint PaletteIndex, const eg_vec4& Palette )
{
	if( 0 == PaletteIndex )
	{
		m_LightColor = eg_color(Palette); // * m_Info->LightInfo.DiffColor;
	}
	else if( 1 == PaletteIndex )
	{
		m_AmbientColor = eg_color(Palette);
	}
}
