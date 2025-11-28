// (c) 2017 Beem Media

#pragma once

#include "EGList.h"
#include "EGTextNode.h"
#include "EGUiTypes.h"
#include "EGUiLayout.h"
#include "EGTextFormat.h"

class EGMenuStack;
class EGOverlayMgr;
class EGUiWidget;
class EGClient;
class EGUiDragAndDropWidget;

class EGMenu : public EGObject , public IEGCustomFormatHandler
{
	EG_CLASS_BODY( EGMenu , EGObject )

private:

	static const eg_uint OBJ_LIST_ID=2;
	static const eg_bool DEFAULT_ORTHO = true; //Always start with an ORTHO camera

public:

	enum class eg_menuevent_t
	{
		NONE,
		INIT,
		DEINIT,
		ACTIVATE,
		DEACTIVATE,
		UPDATE,
		PRE_DRAW,
		DRAW,
		POST_DRAW,
	};

private:

	EGClient*              m_OwnerClient;
	const EGUiLayout*      m_Layout;
	class EGFontBase*      m_Font;
	EGMenuStack*           m_OwnerMenuStack;
	EGOverlayMgr*          m_OwnerOverlayMgr;
	eg_real                m_AspectRatio; 
	EGList<EGUiWidget>     m_Objects;
	EGUiWidget*            m_CaptureWidget;
	EGUiWidget*            m_FocusedWidget;
	EGUiWidget*            m_LastMouseOverWidget;
	EGUiDragAndDropWidget* m_DragAndDropWidget;
	eg_vec2                m_LastMousePos;
	EGLight                m_Lights[4];
	eg_color               m_AmbientLight;
	eg_vec4                m_CameraPos;
	eg_uint                m_LightCount;
	eg_bool                m_bWasLastInputByMouse:1;
	eg_bool                m_bVisible:1;
	eg_bool                m_bInputDisabled:1;
	eg_bool                m_bIsTool:1;
	eg_bool                m_bIgnoreAltNav:1;

public:

	EGMenu();
	~EGMenu();

	virtual void Init( eg_string_crc MenuCrc, EGMenuStack* MenuStackOwner, EGOverlayMgr* OverlayOwner, EGClient* OwnerClient );
	void InitForTool( EGUiLayout* MenuLayout );

	void ProcessEvent( eg_menuevent_t Event , eg_parm RealParm1 = CT_Clear , eg_parm Parm2 = CT_Clear );
	virtual eg_bool HandleInput( eg_menuinput_t InputType );
	virtual eg_bool HandleMouseEvent( eg_menumouse_e MouseEvent , eg_real xPos , eg_real yPos );
	void ProcessInputCmds( const struct egLockstepCmds& Cmds );
	eg_real GetAspect()const;
	EGClient* GetClientOwner(){ return GetOwnerClient(); }
	EGUiWidget* FindObjectFromLayout( const egUiWidgetInfo* Info );
	EGUiWidget* FindObjectAt( eg_real x , eg_real y , eg_vec2* WidgetHitPointOut , const EGUiWidget* ForceFindObj , eg_bool bWidgetsOnly );
	void SetCameraPos( const eg_vec4& Pos ){ m_CameraPos = Pos; }
	void AddLight( const EGLight& Lt, const eg_color& AmbientLight );
	void ClearLights();
	eg_uint GetLights( EGLight* OutArray , eg_uint MaxOut, eg_vec4* CameraPoseOut, eg_color* AmbientColorOut );
	EGMenu* GetParentMenu() const;
	const EGUiLayout* GetLayout() const { return m_Layout; }
	void SetInputEnabled( eg_bool bEnabled ) { m_bInputDisabled = !bEnabled; }
	eg_bool IsInputEnabled() const { return !m_bInputDisabled; }
	eg_bool IsAltNavIgnored() const { return m_bIgnoreAltNav; }

protected:

	EGMenuStack* GetOwnerMenuStack() { return m_OwnerMenuStack; }
	EGOverlayMgr* GetOwnerOverlayMgr() { return m_OwnerOverlayMgr; }

	EGList<EGUiWidget>& GetWidgets() { return m_Objects; }

	void SetIgnoreAltNav( eg_bool bNewValue ) { m_bIgnoreAltNav = bNewValue; }

private:

	void Draw();
	void Draw_ObjBoundsDebug( EGUiWidget* Object, const eg_mat& MatView, const eg_mat& MatProj );
	void Update( eg_real DeltaTime , eg_real AspectRatio );
	EGUiWidget* FindWidgetAt( eg_real x , eg_real y , eg_vec2* HitPointOut, const EGUiWidget* ForceFindObj );
	eg_vec2 GetWidgetHitPoint( const EGUiWidget* Obj, const eg_vec2& HitPoint , const eg_mat& ViewMat , const eg_mat& ProjMat )const; // Can bet out of range.

	void __Construct_InitObjs();
	EGUiWidget* __Construct_InsertObj( const egUiWidgetInfo* Info , eg_bool bInsertFirst );

	void MoveSelection( eg_widget_adjacency Direction );
	void ChangeFocusInternal( EGUiWidget* NewSelection , eg_bool FromMouse , const eg_vec2& WidgetHitPoint );

public:

	// Overrides:
	virtual void OnInit(){ }
	virtual void OnDeinit(){ }
	virtual void OnActivate(){ }
	virtual void OnDeactivate(){ }
	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ){ unused( DeltaTime , AspectRatio ); }
	virtual void OnPreDraw( eg_real AspectRatio ){ unused( AspectRatio ); }
	virtual void OnDraw( eg_real AspectRatio ){ unused( AspectRatio ); }
	virtual void OnPostDraw( eg_real AspectRatio ){ unused( AspectRatio ); }
	virtual void OnFocusedWidgetChanged( EGUiWidget* NewFocusedWidget , EGUiWidget* OldFocusedWidget ) { unused( NewFocusedWidget , OldFocusedWidget ); }

	virtual eg_bool OnInput( eg_menuinput_t InputType ){ unused( InputType ); return false; }
	virtual void OnInputCmds( const struct egLockstepCmds& Cmds ){ unused( Cmds ); }
	virtual void OnDragAndDropStarted( EGUiWidget* SourceWidget , EGUiDragAndDropWidget* DragAndDropWidget ){ unused( SourceWidget , DragAndDropWidget ); }
	virtual void OnDragAndDropEnded( EGUiWidget* DroppedOntoWidget , const eg_vec2& WidgetHitPoint ){ unused( DroppedOntoWidget , WidgetHitPoint ); }
	virtual void OnDragAndDropHovered( EGUiWidget* HoveredWidget ){ unused( HoveredWidget ); }

	// BEGIN IEGCustomFormatHandler
	virtual void FormatText( eg_cpstr Flags , EGTextParmFormatter* Formatter ) const override;
	// END IEGCustomFormatHandler

	// ISdkMenu:
	eg_bool IsActive() const;
	EGMenu* MenuStack_SwitchTo( eg_string_crc MenuId );
	EGMenu* MenuStack_PushTo( eg_string_crc MenuId );
	void MenuStack_Pop();
	void MenuStack_PopTo( EGMenu* Menu );
	void MenuStack_Clear();
	void SetVisible( eg_bool bVisible );
	EGUiWidget* GetWidget( eg_string_crc CrcId );
	template <typename T> T* GetWidget( eg_string_crc CrcId ){ return EGCast<T>(GetWidget(CrcId)); }
	void SetFocusedWidget( EGUiWidget* NewFocusWidget , eg_uint NewSelectedIndex , eg_bool bAllowAudio );
	void SetFocusedWidget( eg_string_crc CrcId , eg_uint Index , eg_bool bAllowAudio );
	void BeginMouseCapture( EGUiWidget* Widget );
	EGUiWidget* GetMouseCapture() { return m_CaptureWidget; }
	void EndMouseCapture();
	EGUiWidget* GetFocusedWidget();
	template <typename T> T* GetFocusedWidget(){ return EGCast<T>(GetFocusedWidget()); }
	EGUiWidget* DuplicateWidget( const EGUiWidget* ObjToDuplicate );
	void MoveWidgetAfter( EGUiWidget* WidgetToMove , EGUiWidget* WidgetToMoveAfter );
	EGClient* GetOwnerClient()const;
	EGClient* GetPrimaryClient()const;
	void SetAllObjectsVisibility( eg_bool bVisible ); // Useful for hiding everything when you only want to show a few things for a reveal
	eg_vec2 GetMousePos() const { return m_LastMousePos; }
	eg_bool GetWasLastInputByMouse() const { return m_bWasLastInputByMouse; }
	void SetDragAndDropWidget( EGUiDragAndDropWidget* NewDragAndDropWidget );
	EGUiDragAndDropWidget* BeginDragAndDrop( EGUiWidget* SourceWidget );
	void EndDragAndDrop( EGUiWidget* DroppedOntoWidget , const eg_vec2& WidgetHitPoint );

	static eg_bool IsIdValidMenu( eg_string_crc MenuId );
};
