// Base UI Object - All UI Objects Inherit from this
// (c) 2016 Beem Media

#pragma once

#include "EGUiTypes.h"
#include "EGList.h"
#include "EGUiWidgetInfo.h"

class EGMenu;
struct egUiWidgetInfo;

class EGUiWidget : public EGObject , public IListable
{
	EG_CLASS_BODY( EGUiWidget , EGObject )

public:

	enum class eg_t
	{
		UNK,
		CLEARZ,
		CAMERA,
		TEXT,
		ENTOBJ,
		ENTBUTTON,
		ENTGRID,
		SCROLLBAR,
		LIGHT,
		IMAGE,
	};

	enum class eg_offset_t
	{
		PRE,
		POST,
	};

	static EGUiWidget* CreateUiObject( EGMenu* InOwner , const egUiWidgetInfo* InInfo );
	static void DestroyUiObject( EGUiWidget* UiObj );
	static eg_t GetTypeFromLayoutObjInfo( const egUiWidgetInfo* Info );

protected:

	const egUiWidgetInfo* m_Info;
	EGMenu*               m_Owner;

	eg_transform          m_PreOffset;
	eg_transform          m_PostOffset;
	eg_bool               m_bIsVisible:1;
	eg_bool               m_bIsEnabled:1;
	egUiWidgetAdjacency   m_Adjacency;

public:

	EGUiWidget( const EGUiWidget& rhs ) = delete;
	EGUiWidget()
	: m_Owner( nullptr )
	, m_Info( nullptr )
	, m_bIsVisible( true )
	, m_bIsEnabled( true )
	, m_PreOffset( CT_Default )
	, m_PostOffset( CT_Default )
	{
		
	}

	virtual ~EGUiWidget(){ }

	virtual void Init( EGMenu* InOwner , const egUiWidgetInfo* InInfo );
	virtual void Draw( eg_real AspectRatio ){ unused( AspectRatio ); }
	virtual void Update( eg_real DeltaTime, eg_real AspectRatio  ){ unused( DeltaTime , AspectRatio ); }
	virtual void QueryEvents( EGArray<eg_d_string>& Out ){ unused( Out ); }
	virtual void QueryTextNodes( EGArray<eg_d_string>& Out ){ unused( Out ); }
	virtual void QueryBones( EGArray<eg_d_string>& Out ){ unused( Out ); }
	virtual void OnFocusGained( eg_bool FromMouse , const eg_vec2& WidgetHitPoint ){ unused( FromMouse , WidgetHitPoint ); }
	virtual void OnFocusLost() { }
	virtual void OnSetSelectedIndex( eg_uint Index ){ unused( Index ); }
	virtual eg_bool HandleInput( eg_menuinput_t Event , const eg_vec2& WidgetHitPoint , eg_bool bFromMouse ){ unused( Event , WidgetHitPoint , bFromMouse ); return false; }
	virtual void SetMuteAudio( eg_bool bMute ){ unused( bMute ); }
	virtual eg_aabb GetBoundsInMouseSpace( eg_real AspectRatio , const eg_mat& MatView , const eg_mat& MatProj )const { unused( AspectRatio , MatView , MatProj ); eg_aabb Out; Out.MakeZeroBox(); return Out; }
	virtual eg_mat GetProjMatrix()const{ return eg_mat::I; }
	virtual eg_mat GetViewMatrix()const{ return eg_mat::I; }
	virtual eg_bool IsHitPointValid( const eg_vec2& WidgetHitPoint ) const{ unused( WidgetHitPoint); return true; }
	virtual eg_bool OnMouseMovedOver( const eg_vec2& WidgetHitPoint , eg_bool bIsMouseCaptured ){ unused( WidgetHitPoint , bIsMouseCaptured ); return false; } // Called if the mouse moved over the widget ( xPos and yPos are relative the dimensions of the widget in [0,1]x[0,1] if the widget is captured the value may be out of that range.
	virtual eg_bool OnMouseMovedOn( const eg_vec2& WidgetHitPoint ){ unused( WidgetHitPoint ); return false; }
	virtual void OnMouseMovedOff(){ }
	virtual eg_bool OnMousePressed( const eg_vec2& WidgetHitPoint ){ unused( WidgetHitPoint ); return false; }
	virtual eg_bool OnMouseReleased( const eg_vec2& WidgetHitPoint , EGUiWidget* WidgetReleasedOn ){ unused( WidgetHitPoint , WidgetReleasedOn ); return false; }
	virtual eg_bool IsWidget() const { return false; }
	virtual eg_bool IsFocusable() const { return false; }
	virtual eg_bool FocusOnHover() const { return false; }
	virtual eg_bool IgnoreDuringSetAllVisibilty() const { return false; }

	eg_t GetType() const;
	const egUiWidgetInfo* GetInfo() const { return m_Info; }
	eg_transform GetFullPose( eg_real AspectRatio )const;
	eg_bool IsVisibleWidget( eg_bool bIncludeUnfocusable = true ) const;
	eg_aabb GetBoundsInSearchSpace();

	eg_bool IsVisible() const { return m_bIsVisible; }
	eg_bool IsEnabled() const { return m_bIsEnabled; }
	eg_bool IsToolPreview() const;

	EGMenu* GetOwnerMenu() const { return m_Owner; }
	eg_string_crc GetId() const;
	const egUiWidgetAdjacency& GetAdjacency() const { return m_Adjacency; }
	eg_string_crc GetAdjacentWidgetId( eg_widget_adjacency Dir ) const;
	void SetAdjacency( const egUiWidgetAdjacency& NewAdjacency ){ m_Adjacency = NewAdjacency; }

	//
	// ISdkMenuObj Interface:
	//
	virtual void SetVisible( eg_bool Visible ) { m_bIsVisible = Visible; }
	virtual void SetEnabled( eg_bool IsActive ) { m_bIsEnabled = IsActive; }
	virtual void SetOffset( eg_offset_t Type, const eg_transform& Offset ) { if( Type == eg_offset_t::PRE ){ m_PreOffset = Offset; } else { m_PostOffset = Offset; } }
	virtual void RunEvent( eg_string_crc EventCrc ) { unused( EventCrc ); }
	virtual void SetAnimationNormalTime( eg_string_crc NodeId , eg_string_crc SkeletonId , eg_string_crc AnimationId , eg_real t ) { unused( NodeId , SkeletonId , AnimationId , t ); }
	virtual void SetPalette( eg_uint PaletteIndex , const eg_vec4& Palette ) { unused( PaletteIndex , Palette ); }
	virtual void SetText( eg_string_crc TextNodeCrcId , const eg_loc_text& NewText ) { unused( TextNodeCrcId , NewText ); }
	void SetText( const eg_loc_text& NewText ) { SetText( CT_Clear , NewText ); }
	virtual void SetTexture( eg_string_crc NodeId , eg_string_crc GroupId , eg_cpstr TexturePath ) { unused( NodeId , GroupId , TexturePath ); }
	virtual void SetMaterial( eg_string_crc NodeId , eg_string_crc GroupId , egv_material Material ) { unused( NodeId , GroupId , Material ); }
	virtual void SetCBone( eg_string_crc NodeId , eg_string_crc BoneId , const eg_transform& Pose ) { unused( NodeId , BoneId , Pose ); }
	virtual eg_uint GetSelectedIndex() { return 0; }

protected:

	static EGClass* FindUiObjectClass( EGClass* DefaultClass, EGClass* SecondDefaultClass , eg_cpstr ClassName, eg_string_crc EntDefCrc, eg_bool bForceDefaultClass, eg_bool bSearchEntDict );;
};
