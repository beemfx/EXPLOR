// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "EGCircleArray.h"

enum class eg_force_sampler_smooth_t
{
	None,
	TimeBased,
	SampleBased,
};


class EGForceSampler
{
public:

	static const eg_int MAX_SAMPLES = 30;

private:

	struct egSample
	{
		eg_vec3 AxisValue = CT_Clear;
		eg_real Age = 0.f;

		egSample() = default;
		egSample( const eg_vec3& rhs ): AxisValue( rhs ) , Age(0.f) { }
	};

	typedef EGCircleArray<egSample,MAX_SAMPLES> EGSampleBuffer;

private:

	EGSampleBuffer m_SampleBuffer;
	eg_vec3 m_SmoothedSample = CT_Clear;
	eg_force_sampler_smooth_t m_SmoothType = eg_force_sampler_smooth_t::SampleBased;
	eg_int m_MaxSamples = MAX_SAMPLES;
	eg_real m_MaxAge = 1.f;

public:
	
	void SetSmoothType( eg_force_sampler_smooth_t InType ) { m_SmoothType = InType; }
	void SetMaxSamples( eg_int InMaxSamples ) { m_MaxSamples = InMaxSamples; }
	void SetMaxAge( eg_real InAge ) { m_MaxAge = InAge; }

	void Update( const eg_vec3& FrameSample , eg_real DeltaTime );
	const eg_vec3& GetSmoothValue() const { return m_SmoothedSample; }

private:

	void Update_Internal( const eg_vec3& NewestFrame , eg_real DeltaTime );

};
