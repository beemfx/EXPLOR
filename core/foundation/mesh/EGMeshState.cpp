// (c) 2017 Beem Media

#include "EGMeshState.h"

void EGMeshState::Update(eg_real fTime)
{
	//If this mesh doesn't have a skeleton there is nothing to do:
	if(0 == m_AnmCur.Skel)return;
	if( !m_IsPlaying )return;

	m_fElapsed += fTime;

	//If we've surpased the animation speed there are some things to do:
	if(m_fElapsed > m_AnmCur.Speed)
	{
		//Since it is possible that a transition completed
		//we'll set the speed to the post trans speed, which
		//should contain the desired speed.
		m_AnmPrev.Skel = 0;

		//If there is a next skeleton we'll start transitioning:
		if(0 != m_AnmNext.Skel)
		{
			m_AnmPrev.Skel = m_AnmCur.Skel;
			m_AnmPrev.SkelAnimCrc= m_AnmCur.SkelAnimCrc;
			m_AnmPrev.Progress = 1.f;
			m_AnmCur.Skel = m_AnmNext.Skel;
			m_AnmCur.SkelAnimCrc = m_AnmNext.SkelAnimCrc;
			m_AnmNext.Skel = 0;
		}
		else
		{
			m_AnmCur.Speed = m_AnmNext.Speed;
		}

		m_fElapsed = 0;
	}

	//Update the progress of the animation...
	m_AnmCur.LastProgress = m_AnmCur.Progress;
	m_AnmCur.Progress = 0 == m_AnmCur.Speed ? 0.0f : (m_fElapsed/m_AnmCur.Speed)*1.0f;
}
