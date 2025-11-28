// (c) 2017 Beem Media

#include "EGMeshComponent.h"
#include "EGXMLBase.h"
#include "EGEngine.h"
#include "EGWorkerThreads.h"
#include "EGMesh.h"
#include "EGSkel.h"
#include "EGMeshMgr2.h"
#include "EGCrcDb.h"
#include "EGTimeline.h"
#include "EGEnt.h"
#include "EGAudioList.h"
#include "EGMeshState.h"
#include "EGRenderer.h"
#include "EGClient.h"
#include "EGFont.h"
#include "EGAudio.h"

EG_CLASS_DECL( EGMeshComponent )

void EGMeshComponent::SetTexture( eg_string_crc GroupId , eg_cpstr TexturePath )
{
	eg_string TxPathStr( TexturePath );
	TxPathStr.ConvertToLower();
	eg_string_crc TxPathHash = eg_string_crc( TxPathStr );
	eg_bool bChanged = false;

	egSubMeshMaterial* SubMeshMaterial = GetMaterial( GroupId );
	if( SubMeshMaterial )
	{
		if( TxPathHash != SubMeshMaterial->TxPathHash )
		{
			bChanged = true;
			SubMeshMaterial->TxPathHash = TxPathHash;
			if( SubMeshMaterial->Material != EGV_MATERIAL_NULL && !SubMeshMaterial->bExternallyOwned )
			{
				EGRenderer::Get().DestroyMaterial( SubMeshMaterial->Material );
				SubMeshMaterial->Material = EGV_MATERIAL_NULL;
			}

			EGString_Copy( SubMeshMaterial->Def.m_strTex[0] , TexturePath , countof(SubMeshMaterial->Def.m_strTex[0]) );
			SubMeshMaterial->Material = EGRenderer::Get().CreateMaterial( &SubMeshMaterial->Def , "" );
			SubMeshMaterial->bExternallyOwned = false;
		}
	}
	else
	{
		egSubMeshMaterial NewMaterial;
		NewMaterial.GroupId = GroupId;
		EGString_Copy( NewMaterial.Def.m_strTex[0] , TexturePath , countof(NewMaterial.Def.m_strTex[0] ) );
		NewMaterial.Material = EGRenderer::Get().CreateMaterial( &NewMaterial.Def , "" );
		NewMaterial.bExternallyOwned = false;
		NewMaterial.TxPathHash = TxPathHash;
		m_CreatedMaterials.Append( NewMaterial );
	}

	if( bChanged )
	{
		ResolveMaterialOverrides();
	}

	m_bHasCustomMaterials = true;
}

void EGMeshComponent::SetMaterial( eg_string_crc GroupId , egv_material Material )
{
	eg_string TxPathStr( EGString_Format( "ExtOwned%u" , static_cast<eg_uint>(Material) ) );
	TxPathStr.ConvertToLower();
	eg_string_crc TxPathHash = eg_string_crc( TxPathStr );
	eg_bool bChanged = false;

	egSubMeshMaterial* SubMeshMaterial = GetMaterial( GroupId );
	if( SubMeshMaterial )
	{
		if( TxPathHash != SubMeshMaterial->TxPathHash )
		{
			bChanged = true;
			SubMeshMaterial->TxPathHash = TxPathHash;
			if( SubMeshMaterial->Material != EGV_MATERIAL_NULL && !SubMeshMaterial->bExternallyOwned )
			{
				EGRenderer::Get().DestroyMaterial( SubMeshMaterial->Material );
				SubMeshMaterial->Material = EGV_MATERIAL_NULL;
			}

			EGString_Copy( SubMeshMaterial->Def.m_strTex[0] , TxPathStr , countof(SubMeshMaterial->Def.m_strTex[0]) );
			SubMeshMaterial->Material = Material;
			SubMeshMaterial->bExternallyOwned = true;
		}
	}
	else
	{
		egSubMeshMaterial NewMaterial;
		NewMaterial.GroupId = GroupId;
		EGString_Copy( NewMaterial.Def.m_strTex[0] , TxPathStr , countof(NewMaterial.Def.m_strTex[0] ) );
		NewMaterial.Material = Material;
		NewMaterial.bExternallyOwned = true;
		NewMaterial.TxPathHash = TxPathHash;
		m_CreatedMaterials.Append( NewMaterial );
	}

	if( bChanged )
	{
		ResolveMaterialOverrides();
	}

	m_bHasCustomMaterials = true;
}

EGMeshComponent::egSubMeshMaterial* EGMeshComponent::GetMaterial( eg_string_crc GroupId )
{
	for( egSubMeshMaterial& Material : m_CreatedMaterials )
	{
		if( Material.GroupId == GroupId )
		{
			return &Material;
		}
	}
	return nullptr;
}

void EGMeshComponent::ResolveMaterialOverrides()
{
	if( m_CreatedMesh && m_CreatedMesh->IsLoaded() )
	{
		const eg_uint NumSubMeshes = m_CreatedMesh->GetNumSubMeshes();
		m_MeshState.m_Materials.Resize( NumSubMeshes );
		for( eg_uint i=0; i<NumSubMeshes; i++ )
		{
			eg_string_crc GroupId = eg_string_crc(m_CreatedMesh->GetSubMeshName(i));
			egSubMeshMaterial* Material = GetMaterial( GroupId );
			if( Material )
			{
				m_MeshState.m_Materials[i] = Material->Material;
			}
			else
			{
				m_MeshState.m_Materials[i] = EGV_MATERIAL_NULL;
			}
		}
	}
}

void EGMeshComponent::ResetBones()
{
	if( m_CreatedMesh && m_CreatedMesh->GetNumBones() > 0 )
	{
		m_MeshState.m_Bones.Resize( m_CreatedMesh->GetNumBones() + 1 );
		m_MeshState.m_AttachBones.Resize( m_CreatedMesh->GetNumBones() + 1 );
	}
	else
	{
		m_MeshState.m_Bones.Resize( 1 );
		m_MeshState.m_AttachBones.Resize( 1 );
	}

	for( eg_transform& Tr : m_MeshState.m_Bones )
	{
		Tr = CT_Default;
	}

	for( eg_transform& Tr : m_MeshState.m_AttachBones )
	{
		Tr = CT_Default;
	}
}

void EGMeshComponent::ResetMaterials()
{
	if( !m_bHasCustomMaterials )
	{
		return;
	}

	m_bHasCustomMaterials = false;

	for( egSubMeshMaterial& Material : m_CreatedMaterials )
	{
		if( Material.Material != EGV_MATERIAL_NULL && !Material.bExternallyOwned )
		{
			EGRenderer::Get().DestroyMaterial( Material.Material );
			Material.Material = EGV_MATERIAL_NULL;
		}
	}
	m_CreatedMaterials.Clear();

	InitSkinMaterials();
	InitDefaultMaterials();
	ResolveMaterialOverrides();
}

void EGMeshComponent::InitSkinMaterials()
{
	assert( m_Materials.IsEmpty() );

	eg_uint MtrlIndex = 0;
	for( const egMeshMaterialGroup& MaterialDef : m_MeshDef->m_Materials )
	{
		egSubMeshMaterial NewMaterial;
		NewMaterial.GroupId = MaterialDef.MeshGroupId.Crc;
		NewMaterial.Def = MaterialDef.Material;

		// There is actually a pretty problematic bug here... Basically if we create a material with the same name as
		// previously existing material before it gets purged we won't get the new material. This really only happens
		// with the editor when we change a texture. Since the names come from the definition. So in-game this wouldn't
		// ever happen. So when we are in the tool we'll add some additional information to make the name different so
		// the old material can get purged properly. Really this should happen in the renderer, but for now this seems
		// to work.
		const eg_int UniqueNumber = Engine_IsTool() ? static_cast<eg_int>(reinterpret_cast<eg_uintptr_t>(this)) : 0;

		eg_string MaterialUniqeId = EGString_Format("CompSkin(%s,%s,%i,%d)" , *m_InitData.Def->GetName() , *m_InitData.Def->GetOwnerFilename() , MtrlIndex , UniqueNumber );
		NewMaterial.Material = EGRenderer::Get().CreateMaterial( &NewMaterial.Def , MaterialUniqeId );
		m_CreatedMaterials.Append( NewMaterial );
		MtrlIndex++;
	}
}

void EGMeshComponent::InitDefaultMaterials()
{
	// Create the default materials for any materials that were not overridden
	if( m_CreatedMesh && m_CreatedMesh->IsLoaded() )
	{
		const eg_uint NumSubmeshes = m_CreatedMesh->GetNumSubMeshes();
		for( eg_uint i=0; i<NumSubmeshes; i++ )
		{
			eg_string_crc GroupId = eg_string_crc( m_CreatedMesh->GetSubMeshName( i ) );
			egSubMeshMaterial* CurrentMaterial = GetMaterial( GroupId );
			if( CurrentMaterial == nullptr )
			{
				const EGMaterialDef* MaterialDef = m_CreatedMesh->GetSubMeshMaterialDef( i );

				egSubMeshMaterial NewMaterial;
				NewMaterial.GroupId = GroupId;
				if( MaterialDef )
				{
					NewMaterial.Def = *MaterialDef;
					eg_string MaterialId = EGString_Format( "CompDef(%s,%s,%i)" , *m_InitData.Def->GetName() , *m_InitData.Def->GetOwnerFilename() , i );
					NewMaterial.Material = EGRenderer::Get().CreateMaterial( &NewMaterial.Def , MaterialId );
				}
				else
				{
					NewMaterial.Material = EGV_MATERIAL_NULL;
				}
				m_CreatedMaterials.Append( NewMaterial );
			}
		}
	}
}

void EGMeshComponent::CreateAssets()
{
	if( m_InitData.bIsClient )
	{
		if( m_MeshDef )
		{
			if( m_MeshDef->m_Mesh.Filename.FullPath.Len() > 0 )
			{
				m_MeshObj = EGMeshMgr2::Get().CreateMesh( *m_MeshDef->m_Mesh.Filename.FullPath );
				m_MeshObj->OnLoaded.Bind( this , &ThisClass::OnAssetLoaded );
				m_CreatedMesh = m_MeshObj->Obj;
			}

			m_Materials.Clear();
			InitSkinMaterials();
		}

		UpdateRenderAssetLoad();
	}
}

void EGMeshComponent::DestroyAssets()
{
	for( egSubMeshMaterial& Material : m_CreatedMaterials )
	{
		if( Material.Material != EGV_MATERIAL_NULL && !Material.bExternallyOwned )
		{
			EGRenderer::Get().DestroyMaterial( Material.Material );
			Material.Material = EGV_MATERIAL_NULL;
		}
	}
	m_CreatedMaterials.Clear();

	EG_SafeRelease( m_MeshObj );
	m_CreatedMesh = nullptr;
	m_bReady = false;
	m_bHasCustomMaterials = false;
}

void EGMeshComponent::Reset()
{
	Super::Reset();

	m_MeshState.Reset();
	ResetBones();
	ResetMaterials();
}

void EGMeshComponent::QueryBones( EGArray<eg_d_string>& Out ) const
{
	if( m_CreatedMesh )
	{
		eg_uint NumBones= m_CreatedMesh->GetNumBones();
		for( eg_uint n=0; n<NumBones; n++ )
		{
			eg_cpstr BoneName = m_CreatedMesh->GetBoneName( n );
			EGCrcDb::AddAndSaveIfInTool( BoneName );
			Out.AppendUnique( BoneName );
		}
	}
}

void EGMeshComponent::QuerySubMeshes( EGArray<eg_d_string>& Out )
{
	if( m_CreatedMesh && m_CreatedMesh->IsLoaded() )
	{
		for( eg_uint i=0; i<m_CreatedMesh->GetNumSubMeshes(); i++ )
		{
			Out.Append( m_CreatedMesh->GetSubMeshName(i) );
		}
	}
}

eg_aabb EGMeshComponent::GetMeshBounds() const
{
	eg_aabb Out( CT_Default );

	if( m_CreatedMesh && m_CreatedMesh->IsLoaded() )
	{
		Out = m_CreatedMesh->GetBounds();

		eg_vec4 Corners[8];
		Out.Get8Corners( Corners , countof(Corners) );
		eg_transform MyPose = m_Pose.GetCurrentValue() * ComputeLocalPose();
		for( eg_vec4& Corner : Corners )
		{
			Corner = Corner * m_CachedScale.GetCurrentValue();
			Corner.w = 1.f;
			Corner *= MyPose;
		}

		Out.Min = Corners[0];
		Out.Max = Corners[0];
		for( const eg_vec4& Corner : Corners )
		{
			Out.AddPoint( Corner );
		}
	}
	else
	{
		EGLogf( eg_log_t::Warning , "Tried to query a mesh for it's bounds while it wasn't loaded." );
	}

	return Out;
}

void EGMeshComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );

	m_EntOwner = EGCast<EGEnt>( InitData.Owner );
	m_MeshDef = EGCast<EGMeshComponent>( InitData.Def );

	CreateAssets();
}

void EGMeshComponent::OnDestruct()
{
	DestroyAssets();

	Super::OnDestruct();
}

void EGMeshComponent::RelevantUpdate( eg_real DeltaTime )
{
	Super::RelevantUpdate( DeltaTime );

	m_Life += DeltaTime;
}

void EGMeshComponent::Draw( const eg_transform& ParentPose ) const
{	
	eg_transform FullPose = m_Pose.GetCurrentValue();
	FullPose.ScaleTranslationOfThisNonUniformly( m_GlobalScale.ToVec3() );
	FullPose *= ParentPose;
	
	if( m_bReady && m_CreatedMesh && !m_bIsHidden )
	{
		MainDisplayList->SetWorldTF( eg_mat( FullPose ) );
		MainDisplayList->SetFloat( F_TIME, m_Life );
		MainDisplayList->SetVec4( eg_rv4_t::SCALE, m_CachedScale.GetCurrentValue() * m_GlobalScale );
		MainDisplayList->SetVec4( eg_rv4_t::ENT_PALETTE_0, m_Palette.GetCurrentValue() );
		
		m_CreatedMesh->Draw( m_MeshState );
	}
}

void EGMeshComponent::ScriptExec( const struct egTimelineAction& Action )
{
	const eg_string_crc ActionAsCrc = eg_string_crc(Action.FnCall.FunctionName);

	switch_crc( ActionAsCrc )
	{
		case_crc("SetTexture"):
		{
			if( Action.FnCall.NumParms >= 2 )
			{
				SetTexture( Action.StrCrcParm(0) , Action.StrParm(1) );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for SetTexture." );
			}
		} break;

		default:
		{
			Super::ScriptExec( Action );
		} break;
	}
}

void EGMeshComponent::RefreshFromDef()
{
	Super::RefreshFromDef();

	DestroyAssets();
	CreateAssets();

	Reset();
}

void EGMeshComponent::OnPropChanged( const egRflEditor& ChangedProperty , eg_bool& bNeedsRebuildOut )
{
	Super::OnPropChanged( ChangedProperty , bNeedsRebuildOut );

	eg_bool bNeedsPropsRefreshed = false;

	if( ChangedProperty.GetData() == &m_Materials )
	{
		bNeedsPropsRefreshed = true;
	}

	if( ChangedProperty.GetData() == &m_Mesh )
	{
		bNeedsPropsRefreshed = true;
	}

	if( bNeedsPropsRefreshed )
	{
		bNeedsRebuildOut = true;
		RefreshEditableProperties();
	}
}

void EGMeshComponent::RefreshEditableProperties()
{
	Super::RefreshEditableProperties();

	EGWeakPtr<EGMeshComponent> WeakThis = this;;
	egComboBoxPopulateFn PopulateMeshId = [WeakThis]( EGArray<eg_d_string>& Out , eg_bool& bManualEditOut ) -> void
	{
		bManualEditOut = true;
		Out.Append( "" );
		if( StaticQueryForSubMeshes && WeakThis.IsValid() )
		{
			StaticQueryForSubMeshes( WeakThis.GetObject() , Out );
		}
	};

	for( egMeshMaterialGroup& Material : m_Materials )
	{
		Material.MeshGroupId.PopulateCb = PopulateMeshId;
	}
}

eg_bool EGMeshComponent::UpdateRenderAssetLoad()
{
	assert( EGWorkerThreads_IsThisMainThread() || Engine_IsTool() );
	if( m_bReady )
	{
		return false;
	}

	eg_bool bReady = true;

	bReady = bReady && ( nullptr == m_CreatedMesh || m_CreatedMesh->IsLoaded() );

	if( !bReady )return false;

	InitDefaultMaterials();
	ResetBones();
	ResolveMaterialOverrides();

	m_bReady = true;
	return true;
}

void EGMeshComponent::OnAssetLoaded( EGMeshMgrObj* Obj )
{
	unused( Obj );

	UpdateRenderAssetLoad();
}

void EGMeshComponent::GetTextNodes( EGArray<const EGMeshBase::egTextNode*>& NodesOut ) const
{
	if( m_CreatedMesh && m_CreatedMesh->IsLoaded() )
	{
		const eg_uint NumTextNodes = m_CreatedMesh->GetNumTextNodes();
		for( eg_uint i=0; i<NumTextNodes; i++ )
		{
			const EGMesh::egTextNode* TextNode = m_CreatedMesh->GetTextNode( i );
			NodesOut.Append( TextNode );
		}
	}
}

eg_cpstr EGMeshComponent::GetTextNodeBoneId( eg_uint BoneId ) const
{
	eg_cpstr Out = "";

	if( m_CreatedMesh && m_CreatedMesh->IsLoaded() && BoneId > 0 )
	{
		Out = m_CreatedMesh->GetBoneName( BoneId - 1 );
	}

	return Out;
}
