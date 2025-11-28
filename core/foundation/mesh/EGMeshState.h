// (c) 2017 Beem Media

#pragma once

#include "EGRendererTypes.h"

class EGMesh;

class EGMeshState
{
public:

	struct egAnim
	{
		eg_uint       Skel;
		eg_string_crc SkelAnimCrc;
		eg_real       Speed;         //Number of seconds animations lasts for.
		eg_real       Progress;      //(0-100)
		eg_real       LastProgress;  //What the progress was on the last frame.

		egAnim(): Skel( 0 ) , SkelAnimCrc( eg_crc( "" ) ) , Speed( 0 ) , Progress( 0 ) , LastProgress( 0 ){}	
		void Reset(){ Skel = 0; SkelAnimCrc = eg_crc( "" ); Speed = 0; Progress = 0; LastProgress = 0; }
	};

public:

	EGMeshState()
	: m_AnmCur()
	, m_AnmPrev()
	, m_AnmNext()
	, m_fElapsed( 0 )
	, m_IsPlaying( false )
	, m_IsHidden( false )
	, m_DefaultBonesSet( false )
	, m_Bones()
	, m_AttachBones()
	{

	}

	~EGMeshState()
	{

	}

	void Update( eg_real fTime );

	void Reset()
	{
		m_fElapsed = 0;
		m_IsPlaying = false;
		m_DefaultBonesSet = false;
		m_AnmCur.Reset();
		m_AnmPrev.Reset();
		m_AnmNext.Reset();
		m_Bones.Clear();
		m_AttachBones.Clear();
	}

public:

	//Data that changes as the animation changes:
	egAnim m_AnmCur;  //Current animation.
	egAnim m_AnmPrev; //Transitioning animation.
	egAnim m_AnmNext; //Que for transitioning.
	EGArray<eg_transform> m_Bones;
	EGArray<eg_transform> m_AttachBones;

	EGArray<egv_material> m_Materials;

	eg_real m_fElapsed;      //Amount of time elapsed since an update (in seconds).
	eg_bool m_IsPlaying:1;
	eg_bool m_IsHidden:1;
	eg_bool m_DefaultBonesSet:1;
};