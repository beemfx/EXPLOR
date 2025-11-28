// UI Entity Object
// (c) 2016 Beem Media
#pragma once
#include "EGUiWidget.h"
#include "EGEntObj.h"

class EGUiMeshWidget: public EGUiWidget
{
	EG_CLASS_BODY( EGUiMeshWidget , EGUiWidget )

protected:

	EGEntObj m_EntObj;
	eg_vec4  m_ScaleVector;
	eg_sampler_s m_OverrideSamplerState = eg_sampler_s::COUNT;

public:

	virtual void Init( EGMenu* InOwner , const egUiWidgetInfo* InInfo ) override;
	~EGUiMeshWidget();

	const EGEntObj& GetEntObj() const{ return m_EntObj; }
	EGEntObj& GetEntObj(){ return m_EntObj; }


	//
	// EGUiOBject Interface:
	//
	virtual void Draw( eg_real AspectRatio ) override;
	virtual void Update( eg_real DeltaTime, eg_real AspectRatio ) override;
	virtual void SetMuteAudio( eg_bool bMute ) override;
	virtual eg_aabb GetBoundsInMouseSpace( eg_real AspectRatio , const eg_mat& MatView , const eg_mat& MatProj )const override;
	virtual void QueryEvents( EGArray<eg_d_string>& Out ) override;
	virtual void QueryTextNodes( EGArray<eg_d_string>& Out ) override;
	virtual void QueryBones( EGArray<eg_d_string>& Out ) override;



	//
	// ISdkMenuObj Interface:
	//
	virtual void RunEvent( eg_string_crc Event ) override;
	virtual void SetAnimationNormalTime( eg_string_crc NodeId , eg_string_crc SkeletonId , eg_string_crc AnimationId , eg_real t ) override;
	virtual void SetCBone( eg_string_crc NodeId , eg_string_crc BoneId , const eg_transform& Pose ) override;
	virtual void SetTexture( eg_string_crc NodeId , eg_string_crc GroupId , eg_cpstr TexturePath );
	virtual void SetMaterial( eg_string_crc NodeId , eg_string_crc GroupId , egv_material Material );
	virtual void SetPalette( eg_uint PaletteIndex , const eg_vec4& Palette ) override;
	virtual void SetText( eg_string_crc TextNodeCrcId , const eg_loc_text& NewText ) override;

	void SetOverrideSamplerState( eg_sampler_s NewSamplerState ) { m_OverrideSamplerState = NewSamplerState; }
	void ClearOverrideSamplerState() { m_OverrideSamplerState = eg_sampler_s::COUNT; }

private:

	void UpdateLocTextForTool();
};
