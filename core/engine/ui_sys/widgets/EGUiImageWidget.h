// (c) 2017 Beem Media

#pragma once

#include "EGUiMeshWidget.h"
#include "EGEntObj.h"

class EGUiImageWidget : public EGUiMeshWidget
{
	EG_CLASS_BODY( EGUiImageWidget , EGUiMeshWidget )

protected:

	EGMaterialDef m_MaterialDef;
	egv_material  m_OverrideMaterial;
	eg_bool       m_bMaterialIsExternal = false;

public:

	virtual void Init( EGMenu* InOwner , const egUiWidgetInfo* InInfo ) override;
	~EGUiImageWidget();

	//
	// EGUiOBject Interface:
	//
	virtual void Draw( eg_real AspectRatio ) override;
	virtual void SetTexture( eg_string_crc NodeId , eg_string_crc GroupId , eg_cpstr TexturePath ) override;
	virtual void SetMaterial( eg_string_crc NodeId , eg_string_crc GroupId , egv_material Material ) override;

	void SetScaleVector( const eg_vec3& NewScale );

private:

	void DestroyMaterial();
};
