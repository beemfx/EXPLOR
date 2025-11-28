// (c) 2018 Beem Media

#pragma once

#include "EGComponent.h"
#include "EGTimelineTypes.h"
#include "EGVisualComponent.reflection.h"

enum egv_material : eg_uint;

egreflect class EGVisualComponent : public egprop EGComponent
{
	EG_CLASS_BODY( EGVisualComponent , EGComponent )
	EG_FRIEND_RFL( EGVisualComponent )

protected:

	egprop eg_bool       m_bIsHidden     = false;
	egprop eg_string_crc m_RenderFilter  = CT_Clear;
	egprop eg_bool       m_bIsReflective = false;
	egprop eg_bool       m_bIsTransparent = false;

	EGTween<eg_vec4> m_Palette;

public:

protected:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual void RefreshFromDef() override;
	virtual void SetPalette( const eg_vec4& NewPalette ) override;
	virtual void Reset() override;
	virtual void RelevantUpdate( eg_real DeltaTime ) override;
	virtual void ScriptExec( const struct egTimelineAction& Action ) override;
	virtual void AddToSceneGraph( const eg_transform& ParentPose , EGWorldSceneGraph* SceneGraph ) const override;

public:

	void SetHidden( eg_bool bNewValue ) { m_bIsHidden = bNewValue; }
	virtual void SetMaterial( eg_string_crc GroupId , egv_material Material ) { unused( GroupId , Material ); }
};
