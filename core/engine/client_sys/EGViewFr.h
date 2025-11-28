/* EGViewFr - The view frustrum, manages the projection matrix, and determines 
if objects are within the view frustrum. Note that all visibility determination
functions should be called only for objects that have been transformed to view
space.
*/

#pragma once

#include "EGFoundationTypes.h"

class EGViewFr
{
public:
	EGViewFr();
	EGViewFr(const EGViewFr& rhs);
	~EGViewFr();

	void Initialize( eg_real fFOVDeg , eg_real fAspect , eg_real fNear , eg_real fFar );
	void Initialize( const eg_t_sphere& sph , eg_real fNear , eg_real fFar );

	void AdjustAboutSphere(const eg_t_sphere& sph);

	eg_real GetAspect()const{ return m_fAspect; }

	eg_bool IsSphereVisible(const eg_t_sphere* pSphere)const;

private:
	eg_real m_fFOV;
	eg_real m_fAspect;
	eg_rectr m_rcVis;

	//The frustrum planes.
	eg_plane m_plnN; //Near
	eg_plane m_plnF; //Far
	eg_plane m_plnL; //Left
	eg_plane m_plnR; //Right
	eg_plane m_plnT; //Top
	eg_plane m_plnB; //Bottom
};

