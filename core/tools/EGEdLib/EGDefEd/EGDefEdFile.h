// (c) 2017 Beem Media

#pragma once

#include "EGEntDef.h"
#include "EGSerializeTypes.h"

class EGEntDef;
class EGEntObj;
class EGComponent;

class EGDefEdFile
{
public:

	enum class eg_setbound_t
	{
		ToMeshBounds,
		ToPhysicsBounds,
		ToMeshAndPhysicsBounds,
	};
	
private:

	EGEntDef*                   m_EntDef = nullptr;
	EGArray<EGComponent*>       m_Components;
	EGEntObj*                   m_PreviewEnt = nullptr;
	eg_vec2                     m_LastMousePos = eg_vec2(0.f,0.f);
	eg_vec2                     m_NextMousePos = eg_vec2(0.f,0.f);
	eg_bool                     m_bShowTextOutlines = false;
	eg_byte                     m_EntDefMem[sizeof(EGEntDef)];

	static EGDefEdFile GlobalLayoutFile;

public:
	static EGDefEdFile& Get(){ return GlobalLayoutFile; }

	void Init();
	void Deinit();
	void SetMousePos( eg_real x , eg_real y ){ m_NextMousePos = eg_vec2(x,y); }
	EGComponent* GetComponentByMousePos( eg_real x, eg_real y );
	void Update( eg_real DeltaTime );
	void Draw();
	void Open( eg_cpstr16 Filename );
	eg_bool Save( eg_cpstr16 Filename );
	void InitPreviewObject();
	void DeinitPreviewObject();
	void ApplyComponentChanges( egRflEditor* RootComponent , const egRflEditor* ChangedProperty );
	void ApplySettingChanges( const egRflEditor* ChangedProperty , egEntDefEditNeeds& NeedsOut );
	void PreviewTimeline( eg_string_crc CrcId );
	void ResetTimeline();
	void StopTimeline();
	eg_aabb GetBounds() const;
	
	EGComponent* InsertNewComponent( EGClass* ComponentClass );
	void MoveObject( EGComponent* ObjToMove , EGComponent* ObjToMoveBefore , EGComponent* ObjParent );
	void DeleteObject( EGComponent* ObjToMove );

	void DeleteTimeline( eg_size_t TimelineIndex );
	void CreateNewTimeline( eg_size_t InsertAtIndex );
	eg_int MoveTimelineEvent( eg_size_t TimelineIndex , eg_int DeltaMove );
	eg_bool IsTimelineEventNameTaken( eg_string_crc Name ) const;

	void SetBoundsTo( eg_setbound_t BoundsToType );
	void CreateTextComponents();

	eg_size_t GetNumObjects();
	EGComponent* GetComponentInfoByIndex( eg_size_t Index );
	void FullRebuild();
	void ConvertToBWNoShadow();
	void ToggleShowTextOutlines(){ m_bShowTextOutlines = !m_bShowTextOutlines; }

	egRflEditor* GetEditor() { return m_EntDef ? m_EntDef->GetEditor() : nullptr; }
	egEntDefEdConfig* GetConfig() { return m_EntDef ? &m_EntDef->GetConfig() : nullptr; }

	void QueryTimelines( EGArray<eg_d_string>& Out );
	EGTimeline* GetTimelineById( eg_string_crc TimelineId );

	void MoveComponentToPose( EGComponent* Component , const eg_transform& NewPose );

private:

	void RefreshComponentList();

	static void StaticQueryComponents( const EGComponent* ComponentDef , EGArray<eg_string_crc>& Out );
	static void StaticQuerySubMeshes( const EGComponent* ComponentDef , EGArray<eg_d_string>& Out );
};