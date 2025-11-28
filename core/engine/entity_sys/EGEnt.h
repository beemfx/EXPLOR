// (c) 2011 Beem Software

#pragma once

#include "EGEntTypes.h"
#include "EGEntDef.h"
#include "EGList.h"
#include "EGPhysBody2.h"
#include "EGEntComponentTree.h"
#include "EGWeakPtr.h"
#include "EGNetSmoothTransform.h"
#include "EGEnt.reflection.h"

class EGGame;
struct egEntCreateParms;
class EGEntComponent;
class EGEntWorld;
class EGServer;
class EGClient;
class EGFileData;
class EGWorldSceneGraph;

egreflect class EGEnt : public EGObject , public IEGPhysBodyOwner
{
	EG_CLASS_BODY( EGEnt , EGObject )
	EG_FRIEND_RFL( EGEnt )

friend class EGEntWorld;

public:

	EGEnt();
	~EGEnt(){}
	
private:

	const EGEntDef*      m_EntDef;  //Client Drawing and Audio Assets.
	eg_ent_id            m_Id;    //Entity's unique ID.
	eg_ent_role          m_Role = eg_ent_role::Unknown;
	eg_ent_world_role    m_WorldRole = eg_ent_world_role::Unknown;
	eg_string_crc        m_DefCrcId; // Template on which the entity is based.
	egNetSmoothTransform m_Pose;
	eg_vec4              m_LinVel;
	eg_vec4              m_AngVel;
	eg_aabb              m_aabb;
	EGEntListHook        m_UpdateListHook = this;
	EGEntListHook        m_RegionListHook = this;
	EGEntListHook        m_NetSmoothingHook = this;
	eg_region_id         m_RegionId = eg_region_id::NoRegion;
	eg_uint              m_nCreateTime; //Should be set to the game time when the entity is created.
	EGEntCloseLights     m_CloseLights;
	eg_color32           m_AmbientLight = eg_color32(0,0,0);
	egprop eg_bool       m_bIsSerialized = true;
	eg_bool              m_bPoseHasJumped:1;
	eg_bool              m_bPhysBodyStateWasSaved:1;
	
private:

	EGPhysBody2*          m_PhysBody;
	EGWeakPtr<EGObject>   m_Owner;
	EGWeakPtr<EGGame>     m_OwnerGame;
	EGWeakPtr<EGEntWorld> m_OwnerWorld;
	egPhysBodyState       m_SavedPhysBodyState;
	eg_bool               m_bPendingDestruction = false;
	eg_bool               m_bIsInServerUpdateList = false;
	eg_bool               m_bCanReplicateData = false;
	mutable egRflEditor   m_Editor;

protected:

	EGEntComponentTree m_ComponentTree;
	
public:

	void Init( const eg_ent_id& EntId , EGEntWorld* OwnerWorld , const egEntCreateParms& CreateParms );
	void Deinit();
	void RefreshGame();

	// Implementable interface (called only on server) (may want to call OnUpdate on client in the future)
	virtual void OnPreInitComponents(){ }
	virtual void OnPostInitComponents(){ }
	virtual void OnCreate( const egEntCreateParms& Parms );
	virtual void OnEnterWorld();
	virtual void OnLeaveWorld();
	virtual void OnDestroy(){ }
	virtual void OnUpdate( eg_real DeltaTime ){ unused( DeltaTime ); }
	virtual void OnFullReplication() const { }
	virtual void PreSave() { m_SavedPhysBodyState = GetPhysBodyState();m_bPhysBodyStateWasSaved = true; }
	virtual void PostSave( EGFileData& GameDataOut ){ unused( GameDataOut ); }
	virtual void PostLoad( const EGFileData& GameDataIn ){ unused( GameDataIn ); }
	virtual void OnDataReplicated( const void* Offset , eg_size_t Size ) { unused( Offset , Size ); }
	virtual void PreInitWorldProperties() { }
	virtual void PostInitWorldProperties() { }

	// BEGIN IEGPhysBodyOwner
	virtual void PhysBodySetPhysBody( class EGPhysBody2* PhysBody ) override;
	virtual void PhysBodyOnUpdateState( const egPhysBodyState& NewState ) override;
	virtual eg_ent_id PhysBodyGetEntId() const override;
	// END IEGPhysBodyOwner

	eg_bool IsAuthority() const { return m_Role == eg_ent_role::Authority; }
	eg_bool IsServer() const;
	eg_bool IsInServerUpdateList() const { assert( IsServer() ); return m_bIsInServerUpdateList; }
	void SetIsInServerUpdateList( eg_bool bNewValue ) { assert( IsServer() ); m_bIsInServerUpdateList = bNewValue; }
	const eg_aabb& GetBounds() const { return m_aabb; }
	const eg_color32& GetAmbientLight() const { return m_AmbientLight; }
	eg_cpstr GetDefName() const;
	eg_ent_role GetRole() const { return m_Role; }
	eg_ent_world_role GetWorldRole() const { return m_WorldRole; }
	eg_bool IsClient() const;
	eg_bool IsEditorPreview() const;
	eg_bool IsPendingDestruction() const { return m_bPendingDestruction; }
	eg_uint GetUpdateType() const { return m_UpdateListHook.GetListId(); }
	const eg_region_id& GetWorldRegion() const { return m_RegionId; }
	EGClient* GetOwnerClient();
	EGServer* GetOwnerServer();
	EGEntWorld* GetWorld();
	const EGEntWorld* GetWorld() const;
	void SetPose( const eg_transform& NewPose );
	void MoveToPose( const eg_transform& NewPose );
	void RunEvent( eg_string_crc Event );
	void ReplicateDataToClient( const void* p , eg_size_t Size ) const;
	void ReplicateDataToServer( const void* p , eg_size_t Size ) const;
	void SetActive( eg_bool bActive );
	void SetKinematicEnabled( eg_bool bEnabled );
	void SetCollisionEnabled( eg_bool bEnabled );
	void SetRaycastHitEnabled( eg_bool bEnabled );
	eg_bool IsKinematicEnabled() const;
	eg_bool IsCollisionEnabled() const;
	eg_bool IsRaycastHitEnabled() const;
	void SetIsSerialized( eg_bool bNewValue ) { m_bIsSerialized = bNewValue; }
	eg_bool IsSerialized() const { return m_bIsSerialized; }
	void DeleteFromWorld();
	void SetLinearVelocity( const eg_vec4& Vel ) { if( m_PhysBody ){ m_PhysBody->SetLinearVelocity( Vel ); } }
	void SetAngularVelocity( const eg_vec4& Vel ) { if( m_PhysBody ){ m_PhysBody->SetAngularVelocity( Vel ); } }
	void SetRelativeLinearVelocity( const eg_vec4& Vel ) { if( m_PhysBody ){ m_PhysBody->SetRelativeLinearVelocity( Vel ); } }
	void SetRelativeAngularVelocity( const eg_vec4& Vel ) { if( m_PhysBody ){ m_PhysBody->SetRelativeAngularVelocity( Vel ); }  }
	void AddForce( const eg_vec4& Force ) { if( m_PhysBody ){ m_PhysBody->AddForce( Force ); } }
	void AddLocalForce( const eg_vec4& Force ) { if( m_PhysBody ){ m_PhysBody->AddLocalForce( Force ); } }
	void AddImpulse( const eg_vec4& Impulse ) { if( m_PhysBody ){ m_PhysBody->AddImpulse( Impulse ); } }
	void AddLocalImpulse( const eg_vec4& Impulse ) { if( m_PhysBody ){ m_PhysBody->AddLocalImpulse( Impulse ); } }
	eg_vec4 GetLinearVelocity() const { return m_PhysBody ? m_PhysBody->GetLinearVelocity() : eg_vec4( CT_Clear ); }
	eg_vec4 GetAngularVelocity() const { return m_PhysBody ? m_PhysBody->GetAngularVelocity() : eg_vec4( CT_Clear ); }
	eg_transform GetPose()const{ return m_Pose; }

	EGEntCloseLights& GetCloseLights() { return m_CloseLights; }
	const EGEntCloseLights& GetCloseLights() const { return m_CloseLights; }

	eg_ent_id     GetID() const;
	eg_d_string   GetDefId() const;
	EGGame*       SDK_GetGame();
	const EGGame* SDK_GetGame() const;
	void          RefreshFromSimulation();

	eg_bool       IsLit() const;
	eg_bool IsAlwaysVisible() const;
	egPhysBodyState GetPhysBodyState() const;
	void SetPhysBodyState( const egPhysBodyState& State , eg_bool bIgnorePose );

	void ReevaluateAABB();

	template<class T>
	T* GetComponentById( eg_string_crc Id ) const
	{
		return m_ComponentTree.GetComponentById<T>( Id );
	}

	template<class T>
	T* FindComponentByClass() const
	{
		return m_ComponentTree.FindComponentByClass<T>();
	}

	template<class T>
	EGArray<T*> FindAllComponentsByClass() const
	{
		return std::move(m_ComponentTree.FindAllComponentsByClass<T>());
	}

	void ActiveUpdate( eg_real DeltaTime );
	void RelevantUpdate( eg_real DeltaTime );
	void ClientDraw( const eg_transform& WorldPose ) const;
	void AddToSceneGraph( EGWorldSceneGraph* SceneGraph ) const;

	eg_bool CanReplicateData() const { return m_bCanReplicateData; }

	void InitComponents();
	void DeinitComponents();

	egRflEditor* GetEditor();
	const egRflEditor* GetEditor() const;
	void ClearEditor() { m_Editor = CT_Clear; }

private:

	void MarkDirtyOnServer();
}; 
