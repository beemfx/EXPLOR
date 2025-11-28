// (c) 2019 Beem Media

#include "EGMs3DCompiler.h"
#include "EGMs3DFile.h"
#include "EGParse.h"
#include "EGFileData.h"
#include "EGRendererTypes.h"
#include "EGBase64.h"
#include "EGMeshBase.h"
#include "EGSkelBase.h"

EGMs3DCompiler::EGMs3DCompiler( const EGFileData& FileData )
{
	LoadFile( FileData );
}

EGMs3DCompiler::~EGMs3DCompiler()
{
	CloseFile();
}

void EGMs3DCompiler::LoadFile( const EGFileData& FileData )
{
	m_File = new EGMs3DFile();
	if( m_File )
	{
		m_File->LoadFromFile( FileData );
		ProcessScripts();
	}
}

void EGMs3DCompiler::CloseFile()
{
	EG_SafeDelete( m_File );
}

void EGMs3DCompiler::SaveEMesh( EGFileData& FileDataOut ) const
{
	if( nullptr == m_File )
	{
		return;
	}

	struct
	{
		eg_aabb aabb = eg_aabb(CT_Default);
	}
	DefData;

	auto Write = [&FileDataOut]( auto... Args ) -> void
	{
		eg_char8 Buffer[1024];
		EGString_FormatToBuffer( Buffer , countof( Buffer ) , Args... );
		FileDataOut.WriteStr8( Buffer );
	};


	{
		////////////////////////////
		/// Header and emesh tag ///
		////////////////////////////
		Write( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" );
		Write( "<emesh version=\"%u\" format=\"%s\" >\n" , EGMeshBase::MESH_VERSION , m_FloatOutput == eg_float_format_t::Text ? "text" : "base64" );


		// Write the text nodes.
		GetTextNodes( FileDataOut );
		Write( "\n" );

		///////////////////////
		/// Write materials ///
		///////////////////////
		const eg_uint NumMaterials = m_File->GetMaterials().LenAs<eg_uint>();
		{
			for( eg_uint i = 0; i < NumMaterials; i++ )
			{
				const EGMs3DFile::ms3d_material_t& Material = m_File->GetMaterials()[i];

				//We need to cut the extension off the texture name:
				char sTex0F[128] = { '\0' };
				char sTex1F[128] = { '\0' };
				char sTex2F[128] = { '\0' };
				char sTex3F[128] = { '\0' };

				eg_cpstr StrMtrlComment = m_File->GetCommentForMaterial( i );

				static eg_char ShaderPath[1024] = { '\0' };
				ShaderPath[0] = '\0';
				eg_string_big OverrideDiffuse( CT_Clear );
				eg_string_big OverrideTex0( CT_Clear );
				eg_string_big OverrideTex1( CT_Clear );
				eg_string_big OverrideTex2( CT_Clear );
				eg_string_big OverrideTex3( CT_Clear );

				auto HandleMtrlComment = [&OverrideDiffuse , &OverrideTex0 , &OverrideTex1 , &OverrideTex2 , &OverrideTex3]( const egParseFuncInfoAsEgStrings& Info ) -> void
				{
					if( Info.FunctionName.EqualsI( "SetShader" ) && Info.NumParms >= 1 )
					{
						EGString_Copy( ShaderPath , Info.Parms[0] , countof( ShaderPath ) );
					}
					else if( (Info.FunctionName.EqualsI( "SetTexture" ) || Info.FunctionName.EqualsI( "SetOverrideTexture" )) && Info.NumParms >= 1 )
					{
						if( Info.NumParms == 1 )
						{
							OverrideDiffuse = Info.Parms[0];
						}
						else if( Info.NumParms >= 2 )
						{
							eg_uint Index = Info.Parms[0].ToUInt();
							switch( Index )
							{
							case 0: OverrideTex0 = Info.Parms[1]; break;
							case 1: OverrideTex1 = Info.Parms[1]; break;
							case 2: OverrideTex2 = Info.Parms[1]; break;
							case 3: OverrideTex3 = Info.Parms[1]; break;
							}
						}
					}
				};

				EGParse_ProcessFnCallScript( StrMtrlComment , EGString_StrLen(StrMtrlComment) , HandleMtrlComment );

				EGLogf( eg_log_t::General , "Found a shader %s" , ShaderPath );
				//MessageBoxW( nullptr , EGString_ToWide( EGString_Format( "Found shader %s" , Shader.String() ) ) , nullptr , MB_OK );

				EGString_Copy( sTex0F , Material.texture , countof(sTex0F) );
				EGString_Copy( sTex1F , Material.alphamap , countof(sTex1F) );

				for( eg_uint sel = 0; sel < 2; sel++ )
				{
					char* s = (0 == sel) ? sTex0F : sTex1F;
					const eg_size_t nLen = EGString_StrLen( s );
					for( eg_size_t i = nLen; i > 0; i-- )
					{
						if( s[i] == '.' )
						{
							s[i] = 0;
							break;
						}
					}

					//Change backslashes to forward slashes, because that is the emergence
					//way.
					for( eg_uint i = 0; i < nLen; i++ )
					{
						if( '\\' == s[i] )
							s[i] = '/';
					}
				}

				if( OverrideDiffuse.Len() > 0 )
				{
					OverrideDiffuse.CopyTo( sTex0F , countof( sTex0F ) );
				}

				if( OverrideTex0.Len() > 0 )
				{
					OverrideTex0.CopyTo( sTex0F , countof( sTex0F ) );
				}

				if( OverrideTex1.Len() > 0 )
				{
					OverrideTex1.CopyTo( sTex1F , countof( sTex1F ) );
				}

				if( OverrideTex2.Len() > 0 )
				{
					OverrideTex2.CopyTo( sTex2F , countof( sTex2F ) );
				}

				if( OverrideTex3.Len() > 0 )
				{
					OverrideTex3.CopyTo( sTex3F , countof( sTex3F ) );
				}

				if( sTex0F[0] == '\0' )
				{
					eg_string_big( "/egdata/textures/default_white" ).CopyTo( sTex0F , countof( sTex0F ) );
				}

				if( sTex1F[0] == '\0' )
				{
					eg_string_big( "/egdata/textures/default_normal" ).CopyTo( sTex1F , countof( sTex1F ) );
				}

				if( sTex2F[0] == '\0' )
				{
					eg_string_big( "/egdata/textures/default_white" ).CopyTo( sTex2F , countof( sTex2F ) );
				}

				EGMaterialDef MtrlDef;
				eg_string_big( sTex0F ).CopyTo( MtrlDef.m_strTex[0] , countof( MtrlDef.m_strTex[0] ) );
				eg_string_big( sTex1F ).CopyTo( MtrlDef.m_strTex[1] , countof( MtrlDef.m_strTex[1] ) );
				eg_string_big( sTex2F ).CopyTo( MtrlDef.m_strTex[2] , countof( MtrlDef.m_strTex[2] ) );
				eg_string_big( sTex3F ).CopyTo( MtrlDef.m_strTex[3] , countof( MtrlDef.m_strTex[3] ) );


#define COPY_COLOR( dest , src ) dest.r = src[0]; dest.g = src[1]; dest.b = src[2]; dest.a = src[3];
				COPY_COLOR( MtrlDef.m_Mtr.Diffuse , Material.diffuse );
				COPY_COLOR( MtrlDef.m_Mtr.Ambient , Material.ambient );
				COPY_COLOR( MtrlDef.m_Mtr.Specular , Material.specular );
				COPY_COLOR( MtrlDef.m_Mtr.Emissive , Material.emissive );
#undef COPY_COLOR

				MtrlDef.m_Mtr.Diffuse.a = Material.transparency;
				MtrlDef.m_Mtr.Power = Material.shininess;

				if( EGString_StrLen( ShaderPath ) )
				{
					EGString_Copy( MtrlDef.m_strPS , ShaderPath , countof( MtrlDef.m_strPS ) );
					EGString_Copy( MtrlDef.m_strVS , ShaderPath , countof( MtrlDef.m_strVS ) );
				}

				Write( MtrlDef.CreateXmlTag( i + 1 ) );
			}
		}

		Write( "\n" );


		///////////////////
		/// Write bones ///
		///////////////////
		const eg_uint NumBones = m_File->GetJoints().LenAs<eg_uint>();
		{
			for( eg_uint i = 0; i < NumBones; i++ )
			{
				Write( "\t<bone id=\"%u\" name=\"%s\"/>\n" , i + 1 , m_File->GetJoints()[i].name );
			}	
			Write( "\n" );
		}

		//////////////////////////////////////////////////////
		/// Write submeshes and get mesh and triangle data ///
		//////////////////////////////////////////////////////

		struct egTriangle
		{
			egv_index i1 = 0;
			egv_index i2 = 0;
			egv_index i3 = 0;
		};

		eg_uint MeshesAdded = 0;
		EGArray<egTriangle> Triangles;
		EGArray<egv_vert_mesh> Verts;

		{
			const eg_uint NumMeshes = m_File->GetGroups().LenAs<eg_uint>();
			for( eg_uint MeshIdx = 0; MeshIdx < NumMeshes; MeshIdx++ )
			{
				const EGMs3DFile::ms3d_group_t& Group = m_File->GetGroups()[MeshIdx];
				eg_cpstr StrComment = m_File->GetCommentForGroup( MeshIdx );

				EGArray<EGMs3DFile::ms3d_full_triangle_t> GroupTriangles;
				EGArray<EGMs3DFile::ms3d_full_vertex_t> GroupVertexes;
				m_File->GetGroupTriangles( MeshIdx , GroupTriangles , GroupVertexes );

				const eg_uint FirstTriangle = Triangles.LenAs<eg_uint>();
				const eg_uint NumTriangles = GroupTriangles.LenAs<eg_uint>();
				const eg_uint FirstVertex = Verts.LenAs<eg_uint>();
				const eg_uint NumVertexes = GroupVertexes.LenAs<eg_uint>();
				const eg_int MaterialIndex = Group.materialIndex;

				eg_bool bAddMesh = true;

				if( EGString_Contains( StrComment , "textnode" ) )
				{
					bAddMesh = false;
				}

				if( !bAddMesh )
				{
					continue;
				}

				MeshesAdded++;

				Write( "\t<mesh id=\"%u\" name=\"%s\" first_triangle=\"%u\" triangles=\"%u\" material=\"%u\"/>\n" ,
					MeshesAdded , Group.name , FirstTriangle + 1 , NumTriangles , MaterialIndex + 1 );

				eg_real MeshMaterialAlpha = 1.f;
				if( m_File->GetMaterials().IsValidIndex( MaterialIndex ) )
				{
					const EGMs3DFile::ms3d_material_t& VertexMaterial = m_File->GetMaterials()[MaterialIndex];
					MeshMaterialAlpha = VertexMaterial.transparency;
				}

				for( eg_uint VertexIdx = 0; VertexIdx < NumVertexes; VertexIdx++ )
				{
					const EGMs3DFile::ms3d_full_vertex_t& Vertex = GroupVertexes[VertexIdx];

					egv_vert_mesh NewVert;
					zero( &NewVert );

					//Copy vertex position (z negative):
					NewVert.Pos = eg_vec4( Vertex.vertex.vertex[0] , Vertex.vertex.vertex[1] , -Vertex.vertex.vertex[2] , 1.f );
					//Normals will be gotten later:
					NewVert.Norm = eg_vec4( Vertex.normal[0] , Vertex.normal[1] , -Vertex.normal[2] , 0.f );
					//Copy texture coord:
					NewVert.Tex0.x = Vertex.s;
					NewVert.Tex0.y = Vertex.t;
					NewVert.Tex1 = NewVert.Tex0;
					NewVert.Color0 = eg_color( 1 , 1 , 1 , MeshMaterialAlpha );


					struct egLocalBoneInfo
					{
						eg_int  Bone = 0;
						eg_real Weight = 0.f;

						egLocalBoneInfo() = default;
						
						egLocalBoneInfo( eg_uint BoneIn , eg_real WeightIn )
						: Bone( BoneIn )
						, Weight( WeightIn )
						{

						}

					} Bones[4];

					Bones[0] = egLocalBoneInfo( Vertex.vertex.boneId + 1 , EG_Clamp( Vertex.ex.weights[0]/100.f , 0.f , 1.f ) );
					Bones[1] = egLocalBoneInfo( Vertex.ex.boneIds[0] + 1 , EG_Clamp( Vertex.ex.weights[1]/100.f , 0.f , 1.f ) );
					Bones[2] = egLocalBoneInfo( Vertex.ex.boneIds[1] + 1 , EG_Clamp( Vertex.ex.weights[2]/100.f , 0.f , 1.f ) );
					Bones[3] = egLocalBoneInfo( Vertex.ex.boneIds[2] + 1 , EG_Clamp( 1.f - Bones[0].Weight - Bones[1].Weight - Bones[2].Weight , 0.f , 1.f ) );


					NewVert.Bone0 = Bones[0].Bone;
					NewVert.Weight0 = Bones[0].Weight;
					NewVert.Bone1 = Bones[1].Bone;
					NewVert.Weight1 = Bones[1].Weight;

					// Not sure why this would happen, but there seems to be a situation where
					// vertexes are assigned to a bone, but there is no bone.
					if( NewVert.Bone0 > NumBones )
					{
						NewVert.Bone0 = 0;
					}
					if( NewVert.Bone1 > NumBones )
					{
						NewVert.Bone1 = 0;
					}

					//Since we are only taking up to two bones at this time. Put any remaining weight into the first bone.
					NewVert.Weight0 = EG_Clamp( 1.f - NewVert.Weight1 , 0.f , 1.f );

					Verts.Append( NewVert );
				}

				for( eg_uint TriIdx = 0; TriIdx < NumTriangles; TriIdx++ )
				{
					const EGMs3DFile::ms3d_full_triangle_t& Triangle = GroupTriangles[TriIdx];

					egTriangle NewTriangle;
					// Triangles are in reverse order from the MilkShape format since we cull the other way in Emergence.
					NewTriangle.i3 = static_cast<eg_uint16>(Triangle.indexes[0] + FirstVertex);
					NewTriangle.i2 = static_cast<eg_uint16>(Triangle.indexes[1] + FirstVertex);
					NewTriangle.i1 = static_cast<eg_uint16>(Triangle.indexes[2] + FirstVertex);

					Triangles.Append( NewTriangle );
				}
			}

			Write( "\n" );
		}

		//////////////////////////
		/// Calculate the AABB ///
		//////////////////////////

		eg_aabb bb( CT_Default );
		if( Verts.Len() > 0 )
		{
			bb.Min = Verts[0].Pos;
			bb.Max = Verts[0].Pos;

			for( eg_uint i = 0; i < Verts.LenAs<eg_uint>(); i++ )
			{
				bb.AddPoint( Verts[i].Pos );
			}
			bb.Min.w = bb.Max.w = 1;
		}

		DefData.aabb = bb;

		if( m_FloatOutput == eg_float_format_t::Text )
		{
			Write( "\t<bounds min=\"%g %g %g\" max=\"%g %g %g\"/>\n" ,
				bb.Min.x , bb.Min.y , bb.Min.z , bb.Max.x , bb.Max.y , bb.Max.z );
		}
		else
		{
			Write( "\t<bounds min=\"%s\" max=\"%s\"/>\n" ,
				*EGBase64_Encode( &bb.Min.x , sizeof( eg_real ) * 3 ) ,
				*EGBase64_Encode( &bb.Max.x , sizeof( eg_real ) * 3 ) );
		}

		Write( "\n" );

		{
			EGArray<egv_index> Indexes;
			Indexes.Reserve( Triangles.Len() * 3 );
			for( const egTriangle& Triangle : Triangles )
			{
				Indexes.Append( Triangle.i1 );
				Indexes.Append( Triangle.i2 );
				Indexes.Append( Triangle.i3 );
			}
			EGVertex_ComputeTangents( Verts.GetArray() , Indexes.GetArray() , Verts.LenAs<eg_uint>() , Indexes.LenAs<eg_uint>()/3 );
		}

		//////////////////////////
		/// Write the vertexes ///
		//////////////////////////
		for( eg_uint i = 0; i < Verts.LenAs<eg_uint>(); i++ )
		{
			Write( EGVertex_ToXmlTag( Verts[i] , i + 1 , m_FloatOutput == eg_float_format_t::Base64 ) );
		}
		Write( "\n" );

		//////////////////////////
		///Write the triangles ///
		//////////////////////////

		for( eg_uint i = 0; i < Triangles.LenAs<eg_uint>(); i++ )
		{
			Write( "\t<triangle id=\"%u\" v1=\"%u\" v2=\"%u\" v3=\"%u\"/>\n" ,
				i + 1 , Triangles[i].i1 + 1 , Triangles[i].i2 + 1 , Triangles[i].i3 + 1 );
		}

		Write( "</emesh>\n" );
	}
}

void EGMs3DCompiler::SaveESkel( EGFileData& FileDataOut ) const
{
	if( nullptr == m_File )
	{
		return;
	}

	auto Write = [&FileDataOut]( auto... Args ) -> void
	{
		eg_char8 Buffer[1024];
		EGString_FormatToBuffer( Buffer , countof( Buffer ) , Args... );
		FileDataOut.WriteStr8( Buffer );
	};

	Write( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" );
	Write( "<eskel version=\"%u\" format=\"%s\" >\n" , EGSkelBase::SKEL_VERSION ,  m_FloatOutput == eg_float_format_t::Text ? "text" : "base64" );


	//////////////////////////////////
	/// Write the single animation ///
	//////////////////////////////////
	GetAnimations( FileDataOut );

	// eg_uint joints = msModel_GetBoneCount( m_pModel );
	// eg_uint frames = msModel_GetTotalFrames( m_pModel );

	Write( "\n" );

	///////////////////////////////
	/// Base skeleton structure ///
	///////////////////////////////

	struct egSkelJoint
	{
		eg_string_big szName;
		eg_string_big szParent;
		eg_uint nParent;
		eg_real fPos[3];
		eg_real fRot[3];

	};

	struct egJointPosition
	{
		eg_real fPos[3];
		eg_real fRot[3];
	};

	struct egFrame
	{
		EGArray<egJointPosition> JointPoses;
		//eg_real fBounds[6];
	};

	//Allocate some memory
	EGArray<egSkelJoint> Joints; // = new SkelJoint[joints];
	EGArray<egFrame> Frames;// = new Frame[frames];

	auto MINUSP = []( auto a ) { return (-a); };
	auto MINUSR = []( auto a ) { return (-a); };

	//First gather the information, then format and write it.
	//Load the base skeleton.
	Joints.Resize( m_File->GetJoints().Len() );
	for( eg_uint i = 0; i < Joints.LenAs<eg_uint>(); i++ )
	{
		const EGMs3DFile::ms3d_joint_t& Joint = m_File->GetJoints()[i];
		egSkelJoint& TargetJoint = Joints[i];
		TargetJoint.nParent = 0;
		TargetJoint.szName = Joint.name;
		TargetJoint.szParent = Joint.parentName;

		TargetJoint.fPos[0] = Joint.position[0];
		TargetJoint.fPos[1] = Joint.position[1];
		TargetJoint.fPos[2] = MINUSP(Joint.position[2]);

		TargetJoint.fRot[0] = Joint.rotation[0];
		TargetJoint.fRot[1] = Joint.rotation[1];
		TargetJoint.fRot[2] = MINUSR(Joint.rotation[2]);
	}

	//Now need to find all the indexes for the parent bones.
	for( eg_uint i = 0; i < Joints.LenAs<eg_uint>(); i++ )
	{
		Joints[i].nParent = 0;
		//If there is no parent the index for the parent bone is 0,
		//note that the index for bones is not zero based.  In other
		//words bone 1 would be m_pBaseSkeleton[0].
		if( Joints[i].szParent.Len() < 1 )
		{
			Joints[i].nParent = 0;
			continue;
		}
		for( eg_uint j = 0; j < Joints.LenAs<eg_uint>(); j++ )
		{
			if( Joints[i].szParent.EqualsI( Joints[j].szName ) )
			{
				Joints[i].nParent = j + 1;
				break;
			}
		}
	}

	//Now that the base skeleton has been obtained write it.
	for( eg_uint i = 0; i < Joints.LenAs<eg_uint>(); i++ )
	{
		const egSkelJoint& Joint = Joints[i];

		if( m_FloatOutput == eg_float_format_t::Base64 )
		{
			Write(
				"\t<joint id=\"%u\" name=\"%s\" parent=\"%u\" position=\"%s\" rotation=\"%s\"/>\n" ,
				i + 1 ,
				Joint.szName.String() , Joint.nParent ,
				*EGBase64_Encode( Joint.fPos , sizeof( eg_real ) * 3 ) ,
				*EGBase64_Encode( Joint.fRot , sizeof( eg_real ) * 3 ) );
		}
		else
		{
			Write(
				"\t<joint id=\"%u\" name=\"%s\" parent=\"%u\" position=\"%g %g %g\" rotation=\"%g %g %g\"/>\n" ,
				i + 1 ,
				Joint.szName.String() , Joint.nParent ,
				Joint.fPos[0] , Joint.fPos[1] , Joint.fPos[2] ,
				Joint.fRot[0] , Joint.fRot[1] , Joint.fRot[2] );
		}
	}

	Write( "\n" );

	EGArray<eg_real> FrameTimes;
	m_File->GetKeyFrameTimes( FrameTimes );

	EGArray<EGMs3DFile::ms3d_full_keyframe_t> KeyFrame;
	for( const eg_real& FrameTime : FrameTimes )
	{
		KeyFrame.Clear( false );
		m_File->GetKeyFrameAtTime( FrameTime , KeyFrame );

		egFrame NewFrame;
		NewFrame.JointPoses.Resize( KeyFrame.Len() );
		for( eg_size_t JointIdx = 0; JointIdx<KeyFrame.Len(); JointIdx++ )
		{
			const EGMs3DFile::ms3d_full_keyframe_t& SourcePose = KeyFrame[JointIdx];
			egJointPosition& TargetPose = NewFrame.JointPoses[JointIdx];

			TargetPose.fRot[0] = SourcePose.rotation[0];
			TargetPose.fRot[1] = SourcePose.rotation[1];
			TargetPose.fRot[2] = MINUSR(SourcePose.rotation[2]);

			TargetPose.fPos[0] = SourcePose.position[0];
			TargetPose.fPos[1] = SourcePose.position[1];
			TargetPose.fPos[2] = MINUSP(SourcePose.position[2]);
		}
		Frames.Append( NewFrame );
	}

	//Now write the joint frame information:
	for( eg_uint i = 0; i < Frames.LenAs<eg_uint>(); i++ )
	{
		Write( "\t<frame id=\"%u\">\n" , i + 1 );

		const egFrame& Frame = Frames[i];

		for( eg_uint j = 0; j < Frame.JointPoses.LenAs<eg_uint>(); j++ )
		{
			const egJointPosition& JointPose = Frame.JointPoses[j];

			Write( "\t\t<joint_pose joint=\"%u\" " , j + 1 );

			if( m_FloatOutput == eg_float_format_t::Base64 )
			{
				Write( "position=\"%s\" rotation=\"%s\"/>\n" ,
					*EGBase64_Encode( JointPose.fPos , sizeof( eg_real ) * 3 ) ,
					*EGBase64_Encode( JointPose.fRot , sizeof( eg_real ) * 3 ) );
			}
			else
			{
				Write( "position=\"%g %g %g\" rotation=\"%g %g %g\"/>\n" ,
					JointPose.fPos[0] , JointPose.fPos[1] , JointPose.fPos[2] ,
					JointPose.fRot[0] , JointPose.fRot[1] , JointPose.fRot[2] );
			}
		}

		Write( "\t</frame>\n" );
		Write( "\n" );
	}

	Write( "</eskel>" );
}

eg_bool EGMs3DCompiler::HasMesh() const
{
	return (m_File && m_File->GetNumVertices() > 0) && !m_bMeshExportDisabled;
}

eg_bool EGMs3DCompiler::HasSkel() const
{
	return (m_File && m_File->GetNumJoints() > 0 && m_File->GetTotalFrames() > 0) && !m_bSkelExportDisabled;
}

eg_uint EGMs3DCompiler::GetTextNodes( EGFileData& File ) const
{
	struct egTextNodeCommentData
	{
		eg_uint       NumLines = 0;
		eg_string_big Text = CT_Clear;
		eg_string_big Font = CT_Clear;
		eg_string_big Justify = CT_Clear;
	};

	auto ParseTextNodeComment = []( eg_cpstr Comment , egTextNodeCommentData* DataOut ) -> void
	{
		DataOut->NumLines = 1;
		DataOut->Text = "Sample Text";
		DataOut->Font = "GenBasB";
		DataOut->Justify = "CENTER";

		auto HandleTextNodeComment = [&DataOut]( const egParseFuncInfoAsEgStrings& ParseInfo ) -> void
		{
			if( ParseInfo.FunctionName.EqualsI( "SetNumLines" ) )
			{
				DataOut->NumLines = ParseInfo.Parms[0].ToUInt();
			}
			else if( ParseInfo.FunctionName.EqualsI( "SetFont" ) )
			{
				DataOut->Font = ParseInfo.Parms[0];
			}
			else if( ParseInfo.FunctionName.EqualsI( "SetText" ) )
			{
				DataOut->Text = ParseInfo.Parms[0];
			}
			else if( ParseInfo.FunctionName.EqualsI( "SetJustification" ) )
			{
				DataOut->Justify = ParseInfo.Parms[0];
			}
		};

		EGParse_ProcessFnCallScript( Comment , EGString_StrLen(Comment) , HandleTextNodeComment );
	};

	eg_uint TextNodes = 0;

	const eg_int NumBones = m_File->GetJoints().LenAs<eg_int>();

	for( const EGMs3DFile::ms3d_comment_t& RawComment : m_File->GetGroupComments() )
	{
		if( m_File->GetGroups().IsValidIndex( RawComment.index ) )
		{
			const EGMs3DFile::ms3d_group_t& Group = m_File->GetGroups()[RawComment.index];
			if( EGString_GetOccurencesOfSubstr( RawComment.comment.GetArray() , "textnode" ) > 0 )
			{
				eg_bool bGotFirstBounds = false;
				eg_aabb Bounds( CT_Clear );
				eg_uint Bone = 0;

				for( const EGMs3DFile::word& TriangleIndex : Group.triangleIndices )
				{
					const EGMs3DFile::ms3d_triangle_t& Triangle = m_File->GetTriangles()[TriangleIndex];
					for( const EGMs3DFile::word& VertexIndex : Triangle.vertexIndices )
					{
						const EGMs3DFile::ms3d_vertex_t& Vertex = m_File->GetVertexes()[VertexIndex];
						eg_vec4 VertexPos( Vertex.vertex[0] , Vertex.vertex[1] , -Vertex.vertex[2] , 1.f );
						if( !bGotFirstBounds )
						{
							bGotFirstBounds = true;
							Bounds.Min = VertexPos;
							Bounds.Max = VertexPos;
						}
						else
						{
							Bounds.AddPoint( VertexPos );
						}

						if( m_File->GetNumJoints() > 0 && Vertex.boneId >= 0 )
						{
							Bone = Vertex.boneId + 1;
						}
					}
				}

				eg_color Color( 1 , 1 , 1 , 1 );

				if( m_File->GetMaterials().IsValidIndex( Group.materialIndex ) )
				{	
					const EGMs3DFile::ms3d_material_t& Material = m_File->GetMaterials()[Group.materialIndex];
					Color = eg_color( Material.diffuse[0] , Material.diffuse[1] , Material.diffuse[2] , Material.diffuse[3] );
				}

				egTextNodeCommentData CommentData;
				ParseTextNodeComment( RawComment.comment.GetArray() , &CommentData );
				TextNodes++;
				eg_char Name[32];
				EGString_Copy( Name , Group.name , countof(Name) );
				eg_char TempMem[2000];
				EGString_FormatToBuffer( TempMem , countof( TempMem ) 
					, "\t<textnode id=\"%s\" font=\"%s\" text=\"%s\" dims=\"%g %g %g\" color=\"%g %g %g %g\" bone=\"%u\" justify=\"%s\">\n"
					, Name
					, CommentData.Font.String()
					, CommentData.Text.String()
					, Bounds.GetWidth() , Bounds.GetHeight() , Bounds.GetHeight() / CommentData.NumLines
					, Color.r , Color.g , Color.b , Color.a
					, Bone
					, CommentData.Justify.String() );

				File.Write( TempMem , strlen( TempMem ) );

				eg_vec4 Center = Bounds.GetCenter();
				EGString_FormatToBuffer( TempMem , countof( TempMem ) , "\t\t<position type=\"ROTXYZ_TRANSXYZ\">0 0 0 %g %g %g</position>\n"
					, Center.x , Center.y , Center.z );

				File.Write( TempMem , strlen( TempMem ) );

				EGString_FormatToBuffer( TempMem , countof( TempMem ) , "\t</textnode>\n" );
				File.Write( TempMem , strlen( TempMem ) );
			}
		}
	}

	return TextNodes;
}

eg_uint EGMs3DCompiler::GetAnimations( EGFileData& File ) const
{
	eg_uint NumAnimations = 0;
	eg_cpstr StrModelComment = m_File->GetCommentForModel();
	const eg_uint TotalFrames = m_File->GetTotalFrames();
	eg_bool bFoundAnim = false;

	auto HandleComment = [&File,&NumAnimations,&TotalFrames,&bFoundAnim]( const egParseFuncInfoAsEgStrings& ParseInfo ) -> void
	{
		eg_string_big Line( CT_Clear );

		if( ParseInfo.FunctionName.EqualsI( "CreateAnimation" ) && ParseInfo.NumParms >= 3 )
		{
			NumAnimations++;

			eg_string_big Name = ParseInfo.Parms[0];
			eg_uint FirstFrame = ParseInfo.Parms[1].ToUInt();
			eg_uint AnimFrames = ParseInfo.Parms[2].ToUInt();
			eg_string_big AnimMode = ParseInfo.NumParms >= 4 ? ParseInfo.Parms[3] : eg_string_big( "" );

			FirstFrame = EG_Clamp<eg_uint>( FirstFrame, 1, TotalFrames );
			AnimFrames = EG_Clamp<eg_uint>( AnimFrames, 1, TotalFrames - FirstFrame + 1 );

			if( 0 == AnimMode.Len() )
			{
				AnimMode = "default";
			}

			Line = EGString_Format( "\t<animation id=\"%u\" name=\"%s\" first_frame=\"%u\" frames=\"%u\" mode=\"%s\" />\n", NumAnimations, Name.String(), FirstFrame, AnimFrames, AnimMode.String() );
			File.Write( Line.String(), Line.Len() );
			bFoundAnim = true;
		}
		else if( ParseInfo.FunctionName.EqualsI("NoSkelExport") )
		{

		}
	};

	EGParse_ProcessFnCallScript( StrModelComment , EGString_StrLen(StrModelComment) , HandleComment );

	if( !bFoundAnim )
	{
		File.WriteStr8( EGString_Format("\t<animation id=\"1\" name=\"%s\" first_frame=\"%u\" frames=\"%u\" mode=\"%s\"/>\n" , "Default" , 1 , TotalFrames , "default") );
	}

	return NumAnimations;
}

void EGMs3DCompiler::ProcessScripts()
{
	auto HandleComment = [this]( const egParseFuncInfoAsEgStrings& ParseInfo ) -> void
	{
		eg_string_big Line( CT_Clear );

		if( ParseInfo.FunctionName.EqualsI("NoSkelExport") )
		{
			m_bSkelExportDisabled = true;
		}
		else if( ParseInfo.FunctionName.EqualsI("NoMeshExport") )
		{
			m_bMeshExportDisabled = true;
		}
	};

	eg_cpstr StrModelComment = m_File->GetCommentForModel();
	EGParse_ProcessFnCallScript( StrModelComment , EGString_StrLen(StrModelComment) , HandleComment );
}
