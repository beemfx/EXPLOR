// (c) 2017 Beem Media

#include "EGMeshMgr2.h"
#include "EGMesh.h"
#include "EGSkel.h"

EG_CLASS_DECL( EGMeshMgrObj )
EG_CLASS_DECL( EGMeshObj )
EG_CLASS_DECL( EGSkelObj )

void EGMeshMgrObj::OnDestruct()
{
	if( m_Owner )
	{
		m_Owner->OnObjReleased( this );
	}
}

void EGMeshMgrObj::OnInnerObjectLoaded()
{
	if( m_Owner )
	{
		m_Owner->OnObjLoaded( this );
	}
}

///////////////////////////////////////////////////////////////////////////////

EGMeshMgr2 EGMeshMgr2::StaticInst;


EGMeshObj* EGMeshMgr2::CreateMesh( eg_cpstr Filename )
{
	return CreateObj<EGMeshObj, EGMesh>( Filename, ".emesh" );
}

EGSkelObj* EGMeshMgr2::CreateSkel( eg_cpstr Filename )
{
	return CreateObj<EGSkelObj, EGSkel>( Filename, ".eskel" );
}

void EGMeshMgr2::OnObjCreated( EGMeshMgrObj* Obj )
{
	auto ListenForLoad = [&Obj]( auto* Asset ) -> void
	{
		if( Asset )
		{
			// eg_size_t NumBefore = Asset->OnLoaded.GetNumListeners();
			Asset->OnLoaded.AddUnique( Obj , &EGMeshMgrObj::OnInnerObjectLoaded );
			// eg_size_t NumAfter = Asset->OnLoaded.GetNumListeners();
			// assert( NumAfter == (NumBefore+1) );

			if( Asset->IsLoaded() )
			{
				Obj->OnInnerObjectLoaded();
				// eg_size_t NumNow = Asset->OnLoaded.GetNumListeners();
				// assert( NumNow == NumBefore );
			}
		}
	};
	
	if( Obj )
	{
		if( m_ItemMap.Contains( Obj->GetId() ) )
		{
			egBaseObjRef& MeshRef = m_ItemMap[Obj->GetId()];
			ListenForLoad( MeshRef.Mesh );
			ListenForLoad( MeshRef.Skel );
		}
		else
		{
			assert( false ); // What?
		}
	}
}

void EGMeshMgr2::OnObjLoaded( EGMeshMgrObj* Obj )
{
	auto StopListeningForLoad = [&Obj]( auto* Asset ) -> void
	{
		if( Asset )
		{
			//eg_size_t NumBefore = Asset->OnLoaded.GetNumListeners();
			Asset->OnLoaded.RemoveAll( Obj );
			//eg_size_t NumAfter = Asset->OnLoaded.GetNumListeners();
			//assert( NumAfter == (NumBefore-1) );
		}
	};

	if( Obj )
	{
		if( m_ItemMap.Contains( Obj->GetId() ) )
		{
			egBaseObjRef& MeshRef = m_ItemMap[Obj->GetId()];
			StopListeningForLoad( MeshRef.Mesh );
			StopListeningForLoad( MeshRef.Skel );
		}

		Obj->OnLoaded.ExecuteIfBound( Obj );
	}
}

void EGMeshMgr2::OnObjReleased( EGMeshMgrObj* Obj )
{
	auto StopListeningForLoad = [&Obj]( auto* Asset ) -> void
	{
		if( Asset )
		{
			// eg_size_t NumBefore = Asset->OnLoaded.GetNumListeners();
			Asset->OnLoaded.RemoveAll( Obj );
			// eg_size_t NumAfter = Asset->OnLoaded.GetNumListeners();
			// assert( NumAfter == (NumBefore-1) );
		}
	};

	if( Obj )
	{
		if( m_ItemMap.Contains( Obj->GetId() ) )
		{
			egBaseObjRef& MeshRef = m_ItemMap[Obj->GetId()];
			StopListeningForLoad( MeshRef.Mesh );
			StopListeningForLoad( MeshRef.Skel );

			MeshRef.RefCount--;
			if( MeshRef.RefCount == 0 )
			{
				#if defined( __DEBUG__ )
				if( MeshRef.Mesh )
				{
					assert( MeshRef.Mesh->OnLoaded.GetNumListeners() == 0 );
				}
				if( MeshRef.Skel )
				{
					assert( MeshRef.Skel->OnLoaded.GetNumListeners() == 0 );
				}
				#endif
				EG_SafeDelete( MeshRef.Mesh );
				EG_SafeDelete( MeshRef.Skel );
				m_ItemMap.Delete( Obj->GetId() );
			}
		}
	}
}

eg_string_crc EGMeshMgr2::FilenameToId( eg_cpstr Filename , eg_cpstr Ext )
{
	eg_string StrFilename( Filename );
	StrFilename.ConvertToLower();
	eg_string_crc Out = eg_string_crc(StrFilename);
	Out += Ext;
	return Out;
}
