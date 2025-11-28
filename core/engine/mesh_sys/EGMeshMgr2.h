// (c) 2017 Beem Media

#pragma once

#include "EGItemMap.h"
#include "EGDelegate.h"

class EGMesh;
class EGSkel;
class EGMeshMgr2;

class EGMeshMgrObj : public EGObject
{
	EG_CLASS_BODY( EGMeshMgrObj , EGObject )

public:

	EGDelegate<void,EGMeshMgrObj*> OnLoaded;

private:

	eg_string_crc m_Id;
	EGMeshMgr2*   m_Owner;

public:

	// BEGIN	EGObject
	virtual void OnDestruct() override;
	// END EGObject

	void InitMeshMgrObj( eg_string_crc Id , EGMeshMgr2* Owner )
	{
		m_Id = Id;
		m_Owner = Owner;
	}

	eg_string_crc GetId() const { return m_Id; }

	void OnInnerObjectLoaded();
};

class EGMeshObj : public EGMeshMgrObj
{
	EG_CLASS_BODY( EGMeshObj , EGMeshMgrObj )

public:
	
	EGMesh* Obj;

};

class EGSkelObj : public EGMeshMgrObj
{
	EG_CLASS_BODY( EGSkelObj , EGMeshMgrObj )

public:

	EGSkel* Obj;
};

class EGMeshMgr2
{
private:

	struct egBaseObjRef
	{
		EGMesh*   Mesh = nullptr;
		EGSkel*   Skel = nullptr;
		eg_size_t RefCount = 0;

		egBaseObjRef() = default;
		egBaseObjRef( EGMesh* In ): Mesh( In ) , RefCount( 1 ){ }
		egBaseObjRef( EGSkel* In ): Skel( In ) , RefCount( 1 ){ }

		operator EGMesh*() const { return Mesh; }
		operator EGSkel*() const { return Skel; }
	};

private:
	
	static EGMeshMgr2 StaticInst;

	EGItemMap<EGStringCrcMapKey,egBaseObjRef> m_ItemMap;

public:

	static EGMeshMgr2& Get(){ return StaticInst; }

	EGMeshMgr2(): m_ItemMap( egBaseObjRef() ){ }
	~EGMeshMgr2(){ assert( m_ItemMap.Len() == 0 ); }

	EGMeshObj* CreateMesh( eg_cpstr Filename );
	EGSkelObj* CreateSkel( eg_cpstr Filename );

	void OnObjCreated( EGMeshMgrObj* Obj );
	void OnObjLoaded( EGMeshMgrObj* Obj );
	void OnObjReleased( EGMeshMgrObj* Obj );

private:

	template<typename TypeOut,typename TypeClass>
	TypeOut* CreateObj( eg_cpstr Filename , eg_cpstr Ext )
	{
		eg_string_crc Id = FilenameToId( Filename , Ext );
		TypeOut* MeshObjOut = EGNewObject<TypeOut>( eg_mem_pool::Default );
		if( MeshObjOut )
		{
			MeshObjOut->InitMeshMgrObj( Id , this );

			if( m_ItemMap.Contains( Id ) )
			{
				egBaseObjRef& MeshRef = m_ItemMap[Id];
				assert( !(MeshRef.Mesh && MeshRef.Skel) );
				MeshObjOut->Obj = MeshRef;
				MeshRef.RefCount++;
			}
			else
			{
				eg_string FullFilename( Filename );
				FullFilename.Append( Ext );
				TypeClass* NewMesh = new TypeClass( FullFilename );
				MeshObjOut->Obj = egBaseObjRef(NewMesh);
				m_ItemMap.Insert( Id , NewMesh );
			}

			OnObjCreated( MeshObjOut );

		}
		return MeshObjOut;
	}

private:

	eg_string_crc FilenameToId( eg_cpstr Filename , eg_cpstr Ext );
};