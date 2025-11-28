// (c) 2011 Beem Media

#pragma once

#include "EGRendererTypes.h"

class EGSkelBase;

class EGMeshBase
{
public:

	static const eg_uint MESH_ID      = 0x48534D4C; //(*(eg_uint*)"LMSH");
	static const eg_uint MESH_VERSION = 123;        //Increment this number if the format changes at all.

	struct egHeader
	{
		//Header Info:
		eg_uint nID = 0;
		eg_uint nVer = 0;
		eg_aabb AABB = eg_aabb( CT_Clear ); //The bounding box for the static mesh (not animated).

		//Data counts:
		#define MESH_DATA_SECTION( _type_ , _var_ ) eg_uint n##_var_##Count = 0;
		#include "EGMeshDataSections.inc"
		#undef MESH_DATA_SECTION

		//Data offsets (from start of m_pMem):
		#define MESH_DATA_SECTION( _type_ , _var_ ) eg_uint Ofs##_var_ = 0;
		#include "EGMeshDataSections.inc"
		#undef MESH_DATA_SECTION
	};

	struct egMtrl
	{
		EGMaterialDef Def;
	};

	//SSubMesh: A Sub Mesh is a smaller mesh within the entire mesh, submeshes
	//are used for different parts of the mesh that use different materials, 
	//typically you want as few as possible, because they are drawn separately.
	struct egSubMesh
	{
		eg_uint  nStrOfsName = 0;     //Name of the submesh.
		eg_uint  nFirstIndex = 0; //Index of the first vertex in the mesh.
		eg_uint  nNumTri = 0;     //Number of triangles in the mesh.
		eg_uint  nMtrIndex = 0;   //Index of the mesh's material.
	};

	struct egTextNode
	{
		eg_string_crc  Id = CT_Clear;
		eg_uint        Bone = 0; //0 for no bone.
		eg_real        Width = 0.f;
		eg_real        Height = 0.f;
		eg_real        LineHeight = 0.f;
		eg_transform   Pose = CT_Default;
		eg_text_align  Justify = eg_text_align::LEFT;
		eg_string_crc  Font = CT_Clear;
		eg_color32     Color = eg_color32::White;
		eg_string_crc  Text = CT_Clear;
	};

	struct egBone
	{
		eg_uint BoneNameOfs = 0;
		eg_uint SkelRef = 0; // Scratch, set in the mesh game class.
		eg_transform SkelBasePose = CT_Default;
	};

public:
	EGMeshBase();
	~EGMeshBase();

public:
	eg_bool IsLoaded()const;

	/////////////////////////
	// Animation Interface //
	/////////////////////////
	//SetupFram Functions: These functions setup the mesh to be rendered with a
	//particular frame of animation of a skeleton, or transition between two
	//animations, or the static pose of the mesh. (See EGEntTree).
	void SetupFrame( class EGMeshState& NodeState ) const;
	void SetupFrame( class EGMeshState& NodeState , eg_uint dwFrame1, eg_uint dwFrame2, eg_real t, EGSkelBase* pSkel) const;
	void SetupFrame( class EGMeshState& NodeState , eg_string_crc nAnim, eg_real fTime, EGSkelBase* pSkel) const;
	void SetupFrame( class EGMeshState& NodeState , eg_uint nFrame1_1, eg_uint nFrame2_1, eg_real fTime1, EGSkelBase* pSkel1, eg_uint nFrame_2, eg_real fTime2, EGSkelBase* pSkel2) const;
	void SetupFrame( class EGMeshState& NodeState , eg_string_crc nAnim1, eg_real fTime1, EGSkelBase* pSkel1, eg_string_crc nAnim2, eg_real fTime2, EGSkelBase* pSkel2) const;

	void SetupCustomBone( class EGMeshState& NodeState , eg_string_crc BoneId , const eg_transform& Transform ) const;

	eg_uint GetJointRef( eg_cpstr szJoint)const;
	eg_uint GetJointRef( eg_string_crc CrcId)const;
	const eg_transform* GetJointTransform( class EGMeshState& NodeState , eg_uint n)const;       //This is the matrix that any vertexes attached to the bone are multiplied by.
	const eg_transform* GetJointAttachTransform(  class EGMeshState& NodeState , eg_uint n)const; //This is the matrix another mesh should be multiplied by to attach (such as attach matrix from shoulders for a head, or from hand for a gun).

																																 //Bones and submeshes are 0 indexed.
	eg_uint  GetNumSubMeshes()const{ return m_H.nSubMeshesCount; }
	eg_cpstr GetSubMeshName(eg_uint n)const;
	const EGMaterialDef* GetSubMeshMaterialDef( eg_uint SubMeshIdx ) const;
	eg_uint  GetNumBones()const{ return m_H.nBonesCount; }
	eg_cpstr GetBoneName(eg_uint n)const;

	eg_uint GetNumTextNodes()const{ return m_H.nTextNodesCount; }
	const egTextNode* GetTextNode( eg_uint n ) const { assert( n < m_H.nTextNodesCount ); return &m_pTextNodes[n]; }

	static const eg_uint INVALID_JOINT = 0xF0000000;

	//A mesh should be made compatible with a skeleton, and then all skeletons
	//used to animate that mesh should share the same bone structure as this
	//skeleton. The mesh is made compatible with the first skeleton that animates
	//it. Because of the way the loading thread works this can occur at any time
	//so you better be sure to be using 100% compatible skeletons.
	void MakeCompatibleWith( const EGSkelBase* pSkel );

protected:

	//////////////////////////
	//BINARY FILE FORMAT START
	#define MESH_DATA_SECTION( _type_ , _var_ ) _type_* m_p##_var_ = nullptr;
	#include "EGMeshDataSections.inc"
	#undef MESH_DATA_SECTION
	//BINARY FILE FORMAT END
	////////////////////////

	//Additional information that is stored:
	eg_size_t     m_nMemSize = 0; //Size of the file in binary form (see chunk below).
	eg_byte*      m_pMem = nullptr; //The memory allocation chunk.
	egHeader      m_H = egHeader(); //Always have a header, for when the mesh isn't actually loaded.

protected:

	void SetPointers(const egHeader& Header);
	void InitPrivateData(eg_cpstr strFile);
};
