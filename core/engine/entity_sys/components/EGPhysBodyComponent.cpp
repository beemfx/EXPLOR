// (c) 2017 Beem Media

#include "EGPhysBodyComponent.h"
#include "EGDisplayList.h"
#include "EGRenderer.h"
#include "EGDebugShapes.h"

EG_CLASS_DECL( EGPhysBodyComponent )

void EGPhysBodyComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );

	const EGPhysBodyComponent* PhysDef = EGCast<EGPhysBodyComponent>( InitData.Def );
	if( PhysDef )
	{
		m_BodyDef.ClassName = PhysDef->m_BodyClass.GetClassName();
		m_BodyDef.Flags = PhysDef->m_BodyFlags;
		for( const eg_phys_shape& Shape : PhysDef->m_Shapes )
		{
			m_BodyDef.Shapes.Append( Shape );
		}
	}

	m_BodyDef.Finalize();
}

void EGPhysBodyComponent::DrawForTool( const eg_transform& ParentPose ) const
{
	EGDebugSphere* Sphere = EGDebugShapes::Get().GetSphere();
	EGDebugBox* Box = EGDebugShapes::Get().GetBox();
	EGDebugCylinder* Cylinder = EGDebugShapes::Get().GetCylinder();
	EGDebugCapsule* Capsule = EGDebugShapes::Get().GetCapsule();

	static eg_color ShapeColors[] =
	{
		eg_color(eg_color32(0,255,0)),
		eg_color(eg_color32(0,0,255)),
		eg_color(eg_color32(255,255,0)),
		eg_color(eg_color32(255,0,0)),
		eg_color(eg_color32(0,255,255)),
		eg_color(eg_color32(255,0,255)),
	};

	eg_size_t NextColorIdx = 0;
	auto GetNextColor = [&NextColorIdx]() -> const eg_color&
	{
		const eg_color& Out = ShapeColors[NextColorIdx];
		NextColorIdx = (NextColorIdx+1)%countof(ShapeColors);
		return Out;
	};

	MainDisplayList->SetWorldTF( eg_mat::BuildTransformNoScale( ParentPose ) );

	MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF );
	MainDisplayList->PushRasterizerState( eg_rasterizer_s::WIREFRAME );

	const EGPhysBodyComponent* PhysDef = EGCast<EGPhysBodyComponent>( m_InitData.Def );
	if( PhysDef )
	{
		for( const eg_phys_shape& Shape : PhysDef->m_Shapes )
		{
			const eg_color& ShapeColor = GetNextColor();

			egPhysShapeDef PhysShape = Shape;
			MainDisplayList->SetWorldTF( eg_mat::BuildTransformNoScale( ParentPose ) );
			// MainDisplayList->DrawAABB( PhysShape.GetAABB() , ShapeColor );

			switch( PhysShape.Shape.Type )
			{
			case eg_shape_t::Sphere:
			{
				Sphere->SetupSphere( PhysShape.Shape.Sphere.Radius );
				Sphere->SetPose( ParentPose * PhysShape.Shape.Transform );
				Sphere->SetColor( ShapeColor );
				Sphere->Draw();
			} break;
			case eg_shape_t::Cylinder:
				Cylinder->SetupCylinder( PhysShape.Shape.Cylinder.Height , PhysShape.Shape.Cylinder.Radius );
				Cylinder->SetPose( ParentPose * PhysShape.Shape.Transform );
				Cylinder->SetColor( ShapeColor );
				Cylinder->Draw();
				break;
			case eg_shape_t::Box:
				Box->SetupBox( PhysShape.Shape.Box.XDim , PhysShape.Shape.Box.YDim , PhysShape.Shape.Box.ZDim );
				Box->SetPose( ParentPose * PhysShape.Shape.Transform );
				Box->SetColor( ShapeColor );
				Box->Draw();
				break;
			case eg_shape_t::Capsule:
				Capsule->SetupCapsule( PhysShape.Shape.Capsule.Height , PhysShape.Shape.Capsule.Radius );
				Capsule->SetPose( ParentPose * PhysShape.Shape.Transform );
				Capsule->SetColor( ShapeColor );
				Capsule->Draw();

				// Testing
				#if 0
				Cylinder->SetupCylinder( PhysShape.Shape.Capsule.Height , PhysShape.Shape.Capsule.Radius );
				Cylinder->SetPose( ParentPose * PhysShape.Shape.Transform );
				Cylinder->SetColor( eg_color(eg_color32(255,0,255)) );
				Cylinder->Draw();
				#endif
				break;
			}
		}
	}

	MainDisplayList->PopRasterizerState();
	MainDisplayList->PopDepthStencilState();
}

void EGPhysBodyComponent::OnPropChanged( const egRflEditor& ChangedProperty , eg_bool& bNeedsRebuildOut )
{
	Super::OnPropChanged( ChangedProperty , bNeedsRebuildOut );

	// Basically if anything changes on this one we need a refresh:

	for( eg_phys_shape& Shape : m_Shapes )
	{
		Shape.Validate();
	}
	RefreshEditableProperties();
	bNeedsRebuildOut = true;
}

void EGPhysBodyComponent::RefreshEditableProperties()
{
	Super::RefreshEditableProperties();

	SetPropEditable( "m_Name" , false );
	SetPropEditable( "m_ParentJointId" , false );
	SetPropEditable( "m_BasePose" , false );
	SetPropEditable( "m_ScaleVector" , false );

	egRflEditor* ShapesEd = m_Editor.GetChildPtr( "m_Shapes" );
	if( ShapesEd && ShapesEd->GetData() == &m_Shapes && ShapesEd->GetNumChildren() == m_Shapes.Len() )
	{
		for( eg_size_t i=0; i<m_Shapes.Len(); i++ )
		{
			egRflEditor* ShapeEd = ShapesEd->GetArrayChildPtr( i );
			if( ShapeEd )
			{
				m_Shapes[i].RefreshEditableProperties( *ShapeEd );
			}
		}
	}
	else
	{
		assert( false ); // No editor?
	}
}
