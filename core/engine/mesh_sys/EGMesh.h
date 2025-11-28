// (c) 2011 Beem Media

#pragma once

#include "EGMeshBase.h"
#include "EGDelegate.h"
#include "EGLoader_Loadable.h"

class EGMesh : public EGMeshBase , private ILoadable
{
public:

	EGSimpleMCDelegate OnLoaded;

public:

	EGMesh( eg_cpstr strFile );
	~EGMesh();

	void Unload();
	void DeallocateMemory();

	void Draw( const class EGMeshState& MeshState ) const;
	void DrawRaw( const class EGMeshState& MeshState ) const; //Won't set materials.

	const eg_aabb& GetBounds() const { return m_H.AABB; }

private:

	egv_vbuffer        m_vb;            //Vertex buffer.
	egv_ibuffer        m_ib;            //Index buffer.
	LOAD_S             m_LoadState:4;
	eg_bool            m_DefTexCreated:1;

private:

	void LoadFromBinary(eg_cpstr strFile);
	void LoadFromBinaryUseThread(eg_cpstr strFile);
	//ILOADABLE INTERFACE:
	virtual void DoLoad(eg_cpstr strFile, const eg_byte*const  pMem, const eg_size_t Size) override;
	virtual void OnLoadComplete(eg_cpstr strFile) override;
};
