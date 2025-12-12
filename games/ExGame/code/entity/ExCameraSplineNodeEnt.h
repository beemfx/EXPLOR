// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExEnt.h"
#include "ExCameraSplineNodeEnt.reflection.h"

egreflect class ExCameraSplineNodeEnt : public egprop ExEnt
{
	EG_CLASS_BODY( ExCameraSplineNodeEnt , ExEnt )
	EG_FRIEND_RFL( ExCameraSplineNodeEnt )

private:

	eg_string_crc m_SplineId;
	eg_int m_SplineNode;

private:

	virtual void OnCreate( const egEntCreateParms& Parms ) override;
	virtual void OnDataReplicated( const void* Offset , eg_size_t Size ) override;
	virtual void OnEnterWorld() override;
	virtual void OnLeaveWorld() override;

	void ParseInitString( const eg_d_string& InitString );

public:

	eg_string_crc GetSplineId() const { return m_SplineId; }
	eg_int GetSplineNodeIndex() const { return m_SplineNode; }

};
