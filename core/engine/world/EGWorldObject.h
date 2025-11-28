// (c) 2018 Beem Media

#pragma once

#include "EGWorldObject.reflection.h"

class EGEntWorld;
class EGFileData;

egreflect class EGWorldObject : public EGObject
{
	EG_CLASS_BODY( EGWorldObject , EGObject )
	EG_FRIEND_RFL( EGWorldObject )

protected:

	egprop eg_string_crc m_Id = CT_Clear;
	egprop eg_transform  m_Pose = CT_Default;
	egprop eg_bool       m_bIsLockedInTool = false;

	EGWorldObject*          m_Parent = nullptr;
	EGArray<EGWorldObject*> m_Children;

protected:

	mutable egRflEditor m_Editor;

public:

	virtual void OnDestruct() override;

	virtual void ApplyToWorldPreload( EGEntWorld* World ) const { unused( World ); }
	virtual void ApplyToWorldPostLoad( EGEntWorld* World ) const { unused( World ); }
	virtual eg_bool IsPreLoadComplete( const EGEntWorld* World ) const { unused( World ); return true; }

	virtual void AddToWorldPreview( EGEntWorld* World ) const { unused( World ); }
	virtual void RemoveFromWorldPreview( EGEntWorld* World ) const { unused( World ); }
	virtual void RefreshInWorldPreview( EGEntWorld* World , const egRflEditor& ChangedProperty ) const;
	virtual void ResetPropertiesEditor() { }
	virtual egRflEditor* GetPropertiesEditor() { return nullptr; }
	const egRflEditor* GetPropertiesEditor() const { return const_cast<EGWorldObject*>(this)->GetPropertiesEditor(); }

	void SetId( eg_cpstr IdIn );
	eg_string_crc GetId() const { return m_Id; }
	void SetWorldPose( const eg_transform& NewWorldPose );
	eg_transform GetWorldPose() const;
	EGWorldObject* GetParent() { return m_Parent; }
	virtual void EditorInitNew( eg_cpstr InId );
	virtual void RefreshEditableProperties();
	void EditorDeleteThis();
	egRflEditor* GetEditor();
	const egRflEditor* GetEditor() const;
	void ClearEditor() { m_Editor = CT_Clear; }
	eg_bool IsLockedInTool() const { return m_bIsLockedInTool; }
	void SetLockedInTool( eg_bool bNewValue ) { m_bIsLockedInTool = bNewValue; }
	eg_int GetDepth() const;
	eg_d_string GetToolDesc() const;
	void GetObjAndChildren( EGArray<EGWorldObject*>& Out );
	void GetObjAndChildren( EGArray<const EGWorldObject*>& Out ) const;

	void Serialize( eg_rfl_serialize_fmt Format , eg_int Depth , EGFileData& MemFile ) const;

	static void BindParentAndChild( EGWorldObject* Parent , EGWorldObject* Child , EGWorldObject* BindBefore );
	static void UnbindParentAndChild( EGWorldObject* Parent , EGWorldObject* Child );
	static eg_d_string GetShortNameFromClass( EGClass* Class );
};
