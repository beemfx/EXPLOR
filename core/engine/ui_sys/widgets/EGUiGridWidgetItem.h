#pragma once

#include "EGUiMeshWidget.h"

class EGUiGridWidget;
class EGUiGridWidget2;

class EGUiGridWidgetItem : public EGUiMeshWidget
{
	EG_CLASS_BODY( EGUiGridWidgetItem , EGUiMeshWidget )

public:

	EGUiGridWidget* Owner;
	EGUiGridWidget2* Owner2;

	void InitGridInfo( EGUiGridWidget* InOwner, eg_string_crc EntDefCrc , eg_string_crc InitialEvent )
	{
		Owner = InOwner;
		Owner2 = nullptr;
		m_EntObj.Init( EntDefCrc );
		if( InitialEvent )
		{
			RunEvent( InitialEvent );
		}
	}

	void InitGridInfo( EGUiGridWidget2* InOwner, eg_string_crc EntDefCrc , eg_string_crc InitialEvent )
	{
		Owner = nullptr;
		Owner2 = InOwner;
		m_EntObj.Init( EntDefCrc );
		if( InitialEvent )
		{
			RunEvent( InitialEvent );
		}
	}

	void DrawAsGridItem( const eg_transform& Pose , const eg_vec4& ScaleVec , eg_bool bIsLit );
	const eg_aabb& GetObjBaseBounds()const{ return m_EntObj.GetBaseBounds(); }

	virtual void Update(  eg_real DeltaTime, eg_real AspectRatio ) override;
	virtual void SetVisible( eg_bool Visible ) override;
	virtual void SetEnabled( eg_bool InFocus ) override;
	virtual eg_uint GetSelectedIndex() override final;
};