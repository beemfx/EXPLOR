/*******************************************************************************
bone.fx is the built in functions for transforming vertices that are skinned by
bones. There are three versions of this function, one to transform just a vertex
and one to transform a vertex and normal, and finally one to transform a vertex,
normal, and tangent.

Note that bone transforms should be an orthogonal 3x3 with translation in the
4th colum (4th row in D3D terms).
*******************************************************************************/
#ifndef __BONE_FX__
#define __BONE_FX__

//We are assuming that g_mBoneM[0] == identity.

#define EG_BONE_VECTOR_TRANSFORM( _Vector_ , _Vertex_ ) mul(_Vector_ , g_mBoneM[_Vertex_.Bone0])*_Vertex_.Weight0 + mul(_Vector_ , g_mBoneM[_Vertex_.Bone1])*_Vertex_.Weight1;


egv_vert_mesh EG_BoneTransform(in egv_vert_mesh IN)
{
	egv_vert_mesh OUT = IN;
	OUT.Pos  = EG_BONE_VECTOR_TRANSFORM( IN.Pos , IN );
	OUT.Norm = EG_BONE_VECTOR_TRANSFORM( IN.Norm , IN );
	OUT.Tan  = EG_BONE_VECTOR_TRANSFORM( IN.Tan , IN );
	OUT.Pos.xyz *= EG_GetScaling().xyz;
	return OUT;
}

void EG_BoneTransformThis( in out egv_vert_mesh V )
{
	V.Pos  = EG_BONE_VECTOR_TRANSFORM( V.Pos , V );
	V.Norm = EG_BONE_VECTOR_TRANSFORM( V.Norm , V );
	V.Tan  = EG_BONE_VECTOR_TRANSFORM( V.Tan , V );
	V.Pos.xyz *= EG_GetScaling().xyz;
}

#undef EG_BONE_VECTOR_TRANSFORM

#endif //__BONE_FX__