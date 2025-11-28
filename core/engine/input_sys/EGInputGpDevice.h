///////////////////////////////////////////////////////////////////////////////
// EGGpDevice - Class for keeping track of a game pad.
// (c) 2015 Beem Software
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "EGInputButtons.h"

class EGGpDevice
{
public:

	static void InitDevices();
	static void DeinitDevices();

public:

	EGGpDevice();
	~EGGpDevice();

	void Init( eg_uint Id );

	void Update();
	eg_bool IsConnected()const{ return m_Connected; }
	eg_bool IsHeld( eg_gp_btn Btn )const;
	eg_real GetRAxisX()const{ return m_RAxis.x; }
	eg_real GetRAxisY()const{ return m_RAxis.y; }
	eg_real GetLAxisX()const{ return m_LAxis.x; }
	eg_real GetLAxisY()const{ return m_LAxis.y; }
	const eg_vec2& GetRAxis( eg_bool bSmoothed ) const { return bSmoothed ? m_RSmoothAxis : m_RAxis; }
	const eg_vec2& GetLAxis( eg_bool bSmoothed ) const { return bSmoothed ? m_LSmoothAxis : m_LAxis; }
	void SetRSmoothAxis( const eg_vec2& In ) { m_RSmoothAxis = In; }
	void SetLSmoothAxis( const eg_vec2& In ) { m_LSmoothAxis = In; }

	eg_bool HadInputThisFrame() const{ return m_bHadInputThisFrame; }
	eg_uint GetId() const { return m_Id; }

private:

	eg_uint  m_Id;
	eg_flags m_BtnState;
	eg_vec2  m_RAxis;
	eg_vec2  m_LAxis;
	eg_vec2  m_RSmoothAxis;
	eg_vec2  m_LSmoothAxis;
	eg_bool  m_Connected:1;
	eg_bool  m_bHadInputThisFrame:1;

private:

	static eg_flag ButtonToFlag( eg_gp_btn Btn );
};