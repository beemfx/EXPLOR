// (c) 2018 Beem Media

#pragma once

class EGFileData;

class EGMs3DFile
{
public:

#include <pshpack1.h>

	typedef unsigned char byte;
	typedef unsigned short word;

	static const byte SELECTED        = 1;
	static const byte HIDDEN          = 2;
	static const byte SELECTED2       = 4;
	static const byte DIRTY           = 8;

	typedef struct
	{
		char    id[10];                                     // always "MS3D000000"
		int     version;                                    // 4
	} ms3d_header_t;

	typedef struct
	{
		byte    flags;                                      // SELECTED | SELECTED2 | HIDDEN
		float   vertex[3];                                  //
		char    boneId;                                     // -1 = no bone
		byte    referenceCount;
	} ms3d_vertex_t;

	typedef struct
	{
		word    flags;                                      // SELECTED | SELECTED2 | HIDDEN
		word    vertexIndices[3];                           //
		float   vertexNormals[3][3];                        //
		float   s[3];                                       //
		float   t[3];                                       //
		byte    smoothingGroup;                             // 1 - 32
		byte    groupIndex;                                 //
	} ms3d_triangle_t;

	/*
	typedef struct
	{
		word edgeIndices[2];
	} ms3d_edge_t;
	*/

	typedef struct
	{
		byte            flags;                              // SELECTED | HIDDEN
		char            name[32];                           //
		word            numtriangles;                       //
		EGArray<word>   triangleIndices;					// the groups group the triangles
		char            materialIndex;                      // -1 = no material
	} ms3d_group_t;

	typedef struct
	{
		char            name[32];                           //
		float           ambient[4];                         //
		float           diffuse[4];                         //
		float           specular[4];                        //
		float           emissive[4];                        //
		float           shininess;                          // 0.0f - 128.0f
		float           transparency;                       // 0.0f - 1.0f
		char            mode;                               // 0, 1, 2 is unused now
		char            texture[128];                        // texture.bmp
		char            alphamap[128];                       // alpha.bmp
	} ms3d_material_t;

	typedef struct
	{
		float           time;                               // time in seconds
		float           rotation[3];                        // x, y, z angles
	} ms3d_keyframe_rot_t;

	typedef struct
	{
		float           time;                               // time in seconds
		float           position[3];                        // local position
	} ms3d_keyframe_pos_t;

	typedef struct
	{
		byte            flags;                              // SELECTED | DIRTY
		char            name[32];                           //
		char            parentName[32];                     //
		float           rotation[3];                        // local reference matrix
		float           position[3];

		word            numKeyFramesRot;                    //
		word            numKeyFramesTrans;                  //

		EGArray<ms3d_keyframe_rot_t> keyFramesRot;      // local animation matrices
		EGArray<ms3d_keyframe_pos_t> keyFramesTrans;  // local animation matrices
	} ms3d_joint_t;

	typedef struct
	{
		int index;											// index of group, material or joint
		int commentLength;									// length of comment (terminating '\0' is not saved), "MC" has comment length of 2 (not 3)
		EGArray<char> comment;						// comment
	} ms3d_comment_t;

	typedef struct
	{
		char boneIds[3];									// index of joint or -1, if -1, then that weight is ignored, since subVersion 1
		byte weights[3];									// vertex weight ranging from 0 - 100, last weight is computed by 1.0 - sum(all weights), since subVersion 1
																// weight[0] is the weight for boneId in ms3d_vertex_t
																// weight[1] is the weight for boneIds[0]
																// weight[2] is the weight for boneIds[1]
																// 1.0f - weight[0] - weight[1] - weight[2] is the weight for boneIds[2]
	} ms3d_vertex_ex_v1_t;

	typedef struct
	{
		char boneIds[3];									// index of joint or -1, if -1, then that weight is ignored, since subVersion 1
		byte weights[3];									// vertex weight ranging from 0 - 100, last weight is computed by 1.0 - sum(all weights), since subVersion 1
																// weight[0] is the weight for boneId in ms3d_vertex_t
																// weight[1] is the weight for boneIds[0]
																// weight[2] is the weight for boneIds[1]
																// 1.0f - weight[0] - weight[1] - weight[2] is the weight for boneIds[2]
		unsigned int extra;									// vertex extra, which can be used as color or anything else, since subVersion 2
	} ms3d_vertex_ex_v2_t;

	typedef struct
	{
		char boneIds[3];									// index of joint or -1, if -1, then that weight is ignored, since subVersion 1
		byte weights[3];									// vertex weight ranging from 0 - 100, last weight is computed by 1.0 - sum(all weights), since subVersion 1
																// weight[0] is the weight for boneId in ms3d_vertex_t
																// weight[1] is the weight for boneIds[0]
																// weight[2] is the weight for boneIds[1]
																// 1.0f - weight[0] - weight[1] - weight[2] is the weight for boneIds[2]
		unsigned int extra[2];									// vertex extra, which can be used as color or anything else, since subVersion 3
	} ms3d_vertex_ex_v3_t;

	typedef struct
	{
		float color[3];	// joint color, since subVersion == 1
	} ms3d_joint_ex_t;

	typedef struct
	{
		float jointSize;	// joint size, since subVersion == 1
		int transparencyMode; // 0 = simple, 1 = depth buffered with alpha ref, 2 = depth sorted triangles, since subVersion == 1
		float alphaRef; // alpha reference value for transparencyMode = 1, since subVersion == 1
	} ms3d_model_ex_t;

	typedef struct  
	{
		word indexes[3];
	} ms3d_full_triangle_t;

	typedef struct 
	{
		ms3d_vertex_t vertex;
		ms3d_vertex_ex_v3_t ex;
		float normal[3];
		float s;
		float t;
	} ms3d_full_vertex_t;

	typedef struct  
	{
		float rotation[3];
		float position[3];
	} ms3d_full_keyframe_t;

#include <poppack.h>

private:

	ms3d_header_t                m_Header;
	word                         m_nNumVertices = 0;
	EGArray<ms3d_vertex_t>       m_arrVertices;
	word                         m_nNumTriangles = 0;
	EGArray<ms3d_triangle_t>     m_arrTriangles;
	// EGArray<ms3d_edge_t>         m_arrEdges;
	word                         m_nNumGroups = 0;
	EGArray<ms3d_group_t>        m_arrGroups;
	word                         m_nNumMaterials = 0;
	EGArray<ms3d_material_t>     m_arrMaterials;
	float                        m_fAnimationFPS = 0.f;
	float                        m_fCurrentTime = 0.f;
	int                          m_iTotalFrames = 0;
	word                         m_nNumJoints = 0;
	EGArray<ms3d_joint_t>        m_arrJoints;
	int                          m_subVersionA = 0;
	int                          m_nNumGroupComments = 0;
	EGArray<ms3d_comment_t>      m_GroupComments;
	int                          m_nNumMaterialComments = 0;
	EGArray<ms3d_comment_t>      m_MaterialComments;
	int                          m_nNumJointComments = 0;
	EGArray<ms3d_comment_t>      m_JointComments;
	int                          m_nNumModelComments = 0;
	EGArray<ms3d_comment_t>      m_ModelComments;
	int                          m_subVersionB = 0;
	EGArray<ms3d_vertex_ex_v3_t> m_VertexExV3;
	int                          m_SubVersionC = 0;
	EGArray<ms3d_joint_ex_t>        m_JointEx;
	int                          m_SubVersionD = 0;
	ms3d_model_ex_t              m_ModelEx;


public:

	EGMs3DFile();
	~EGMs3DFile();

	EGMs3DFile(const EGMs3DFile& rhs) = delete;
	EGMs3DFile& operator=(const EGMs3DFile& rhs) = delete;

public:

	bool LoadFromFile( const EGFileData& MemFile );
	void Clear();

	int GetNumVertices();
	void GetVertexAt(int nIndex, ms3d_vertex_t **ppVertex);
	const EGArray<ms3d_vertex_t>& GetVertexes() const { return m_arrVertices; }
	int GetNumTriangles();
	void GetTriangleAt(int nIndex, ms3d_triangle_t **ppTriangle);
	const EGArray<ms3d_triangle_t>& GetTriangles() const { return m_arrTriangles; }
	// int GetNumEdges();
	// void GetEdgeAt(int nIndex, ms3d_edge_t **ppEdge);
	int GetNumGroups();
	void GetGroupAt(int nIndex, ms3d_group_t **ppGroup);
	const EGArray<ms3d_group_t>& GetGroups() const { return m_arrGroups; }
	int GetNumMaterials();
	void GetMaterialAt(int nIndex, ms3d_material_t **ppMaterial);
	const EGArray<ms3d_material_t>& GetMaterials() const { return m_arrMaterials; }
	int GetNumJoints();
	void GetJointAt(int nIndex, ms3d_joint_t **ppJoint);
	const EGArray<ms3d_joint_t>& GetJoints() const { return m_arrJoints; }
	int FindJointByName(const char* lpszName);

	const EGArray<ms3d_comment_t>& GetGroupComments() const { return m_GroupComments; }
	const EGArray<ms3d_comment_t>& GetMaterialComments() const { return m_MaterialComments; }
	const EGArray<ms3d_comment_t>& GetJointComments() const { return m_JointComments; }
	const EGArray<ms3d_comment_t>& GetModelComments() const { return m_ModelComments; }

	eg_cpstr GetCommentForGroup( eg_size_t GroupIndex ) const;
	eg_cpstr GetCommentForMaterial( eg_size_t MaterialIndex ) const;
	eg_cpstr GetCommentForJoint( eg_size_t JointIndex ) const;
	eg_cpstr GetCommentForModel() const;

	void GetGroupTriangles( eg_size_t GroupIndex , EGArray<ms3d_full_triangle_t>& TrianglesOut , EGArray<ms3d_full_vertex_t>& VertexesOut ) const;
	void GetKeyFrameTimes( EGArray<eg_real>& FrameTimesOut ) const;
	void GetKeyFrameAtTime( eg_real FrameTime , EGArray<ms3d_full_keyframe_t>& KeyFrameOut ) const;

	float GetAnimationFPS();
	float GetCurrentTime();
	int GetTotalFrames();

private:

	static void GetPositionByFrame( const ms3d_joint_t& Joint , eg_real FrameTime , ms3d_full_keyframe_t& TargetKeyFrame );
	static void GetRotationByFrame( const ms3d_joint_t& Joint , eg_real FrameTime , ms3d_full_keyframe_t& TargetKeyFrame );
	static void GetTransformByFrame( const ms3d_joint_t& Joint , eg_real FrameTime , ms3d_full_keyframe_t& TargetKeyFrame );


	static word MAKEDWORD( word a , word b) { return ((unsigned int)(((word)(a)) | ((word)((word)(b))) << 16)); }
};
