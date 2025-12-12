// (c) 2017 Beem Media

#pragma once

#include "EGUiMeshWidget.h"
#include "ExCombatTeamWidgetBase.h"
#include "ExCombat.h"

class ExCombatMenuOpponentsWidget : public EGUiMeshWidget , public ExCombatTeamWidgetBase
{
	EG_CLASS_BODY( ExCombatMenuOpponentsWidget , EGUiMeshWidget )

private:

	static const eg_real CAMERA_MOVE_TIME;
	static const eg_real CAMERA_RESTORE_TIME;
	static const eg_real TEMP_ANIM_TIME;
	static const eg_real MONSTER_SPACING;

	enum class ex_monster_t
	{
		Empty, // Slot is available
		Visible, // Monster is a visible opponent
		TempAnimation, // Monster is normally off screen but is playing some animation
		Dead, // Monster is dead, should be purged
	};

	enum class ex_monster_anim
	{
		Idle,
		Dodge,
		Attack,
		Die,
	};

	struct exMonsterObj
	{
		ex_monster_t       Type = ex_monster_t::Empty;
		EGEntObj*          Obj = nullptr;
		const ExCombatant* Combatant = nullptr;
		exMovingPose       Pose;
		eg_real            TempAnimLife = 0.f;
		eg_bool            bIsSelected = false;
		exResourceBarTween HealthTween;

		exMonsterObj() = default;
		exMonsterObj( exMonsterObj&& rhs );
		~exMonsterObj();

		const exMonsterObj& operator = ( const exMonsterObj& rhs );
		const exMonsterObj& operator = ( exMonsterObj&& rhs );
		void Update( eg_real DeltaTime );
		void SetPose( const eg_transform& NewPose ){ Pose.SetPose( NewPose ); }
		void MoveTo( const eg_transform& NewPose ){ Pose.MoveTo( NewPose , false ); }
		const eg_transform& GetPose() const { return Pose.GetPose(); }
		const eg_transform& GetTargetPose() const { return Pose.GetTargetPose(); }
		void PlayAnimation( ex_monster_anim InAnim );
		void SetIsSelected( eg_bool bNewValue );
	};

	typedef EGItemMap<EGStringCrcMapKey,EGEntObj*> ExEntObjItemMap;
	typedef EGArray<exMonsterObj> ExMonsterArray;

private:

	ExEntObjItemMap m_MonsterMap;
	ExMonsterArray  m_FrontRowMonsters;
	ExMonsterArray  m_TempMonsters;
	exMovingPose    m_CameraPose;
	exMovingPose    m_CenterPose;
	eg_int          m_TempAnimPoseIndex = 0;
	eg_int          m_TargetOffset = 0;
	eg_real         m_LastAspectRatio = 0.f;
	eg_int          m_MouseCaptureTarget = 0;
	const ExCombatant* m_LastAttacked = nullptr;

public:

	EGDelegate<void,const ExCombatant*> TargetChosenDelegate;

	ExCombatMenuOpponentsWidget(): m_MonsterMap(nullptr){ }

	// BEGIN ExCombatTeamWidgetBase
	virtual void InitCombat( class ExCombat* Combat ) override final;
	virtual void SetCombatantList( const ExCombatTeam& Team , const ExCombatantList& List ) override final;
	virtual void PlayAnimation( const exCombatAnimInfo& AnimInfo ) override final;
	// END ExCombatTeamWidgetBase

	virtual void Init( EGMenu* InOwner , const egUiWidgetInfo* InInfo ) override final;
	virtual void Update( eg_real DeltaTime, eg_real AspectRatio ) override final;

	virtual void Draw( eg_real AspectRatio ) override final;

	virtual void OnConstruct() override final;
	virtual void OnDestruct() override final;

	virtual void SetEnabled( eg_bool IsActive ) override;
	virtual eg_bool HandleInput( eg_menuinput_t Event , const eg_vec2& WidgetHitPoint , eg_bool bFromMouse ) override;

	virtual eg_bool OnMouseMovedOver( const eg_vec2& WidgetHitPoint , eg_bool bIsMouseCaptured ) override; // Called if the mouse moved over the widget ( xPos and yPos are relative the dimensions of the widget in [0,1]x[0,1] if the widget is captured the value may be out of that range.
	virtual eg_bool OnMouseMovedOn( const eg_vec2& WidgetHitPoint ) override;
	virtual void OnMouseMovedOff() override;
	void HandleMouseHoveredOn( const eg_vec2& WidgetHitPoint );
	eg_int GetTargetOffsetAtHitPoint( const eg_vec2& WidgetHitPoint ) const;
	virtual eg_aabb GetBoundsInMouseSpace( eg_real AspectRatio , const eg_mat& MatView , const eg_mat& MatProj ) const override;
	virtual eg_bool IsWidget() const override { return true; }
	virtual eg_bool IsFocusable() const override { return IsEnabled(); }
	virtual eg_bool OnMousePressed( const eg_vec2& WidgetHitPoint );
	virtual eg_bool OnMouseReleased( const eg_vec2& WidgetHitPoint , EGUiWidget* WidgetReleasedOn );

	void HandleBackedOut() { RestoreCamera(); }

private:

	void SelectInitialTarget();
	static eg_size_t TargetOffsetToMonsterIndex( eg_int TargetOffset );
	void MoveTarget( eg_int MoveDelta , eg_bool bDontMoveCamera = false );

	void RefreshTargetted( exMonsterObj* NewTarget );

	void MoveCameraToMonster( exMonsterObj* MonsterObj , eg_bool bRestore );
	void RestoreCamera();

	void CreateMonsterMap();
	void DestroyMonsterMap();
	exMonsterObj* GetMonsterAtTargetOffset( eg_int Offset );
	exMonsterObj* GetMonsterObj( const ExCombatant* Combatant );
	exMonsterObj* SpawnMonsterObj( const ExCombatant* Combatant , eg_bool bSpawnAsTemp );

	eg_transform GetSlotPose( eg_int SlotIndex , eg_bool bTempSlot , eg_transform* StartingOffsetOut );
};