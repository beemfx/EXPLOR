// (c) 2020 Beem Media. All Rights Reserved.

#include "EGForceSampler.h"

void EGForceSampler::Update( const eg_vec3& FrameSample , eg_real DeltaTime )
{
	Update_Internal( FrameSample , DeltaTime );
}

void EGForceSampler::Update_Internal( const eg_vec3& NewestFrame , eg_real DeltaTime )
{
	eg_uint NumSamplesToUse = 1;

	switch( m_SmoothType )
	{
	case eg_force_sampler_smooth_t::None:
	{
		NumSamplesToUse = 2; // Secretly we really use two samples as a single sample generally will feel jittery with slight movements of the mouse.
	} break;
	case eg_force_sampler_smooth_t::TimeBased:
	{
		// Update age of all samples
		{
			const eg_uint NumSamples = m_SampleBuffer.Len();
			eg_uint NumToPurge = 0;
			for( eg_uint i = 0; i < NumSamples; i++ )
			{
				m_SampleBuffer[i].Age += DeltaTime;
				if( m_SampleBuffer[i].Age >= m_MaxAge )
				{
					NumToPurge++;
				}
			}

			for( eg_uint i = 0; i < NumToPurge; i++ )
			{
				m_SampleBuffer.RemoveFirst();
			}

			NumSamplesToUse = MAX_SAMPLES;
		}
	} break;
	case eg_force_sampler_smooth_t::SampleBased:
	{
		NumSamplesToUse = m_MaxSamples + 1; // We always add 1 so there are at least two samples. (Setting is from 1-X so it's really 2-(X+1)).
		NumSamplesToUse = EG_Clamp<eg_uint>( NumSamplesToUse , 2 , MAX_SAMPLES );
	} break;
	}

	assert( 0 < NumSamplesToUse && NumSamplesToUse <= MAX_SAMPLES );

	// Doesn't actually matter if there are 0 samples since the value will be zero.
	while( m_SampleBuffer.IsFull() || m_SampleBuffer.Len() >= NumSamplesToUse )
	{
		m_SampleBuffer.RemoveFirst();
	}
	m_SampleBuffer.InsertLast( NewestFrame );
	assert( m_SampleBuffer.Len() <= NumSamplesToUse );

	const eg_uint NumSmoothSamples = m_SampleBuffer.Len();

	// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "Mouse Samples: {0}" , NumSmoothSamples ) );

	eg_vec3 AveragedDelta( CT_Clear );

	if( NumSmoothSamples > 0 )
	{
		for( eg_uint i = 0; i < NumSmoothSamples; i++ )
		{
			// TODO: Weight later samples to a lesser degree.
			const eg_vec3& Sample = m_SampleBuffer[(NumSmoothSamples - 1) - i].AxisValue;
			AveragedDelta += Sample;

			// EGLogToScreen::Get().Log( this , __LINE__+i , 1.f , *EGSFormat8( "Samples[{0}]: {1} {2}" , i , Sample.x , Sample.y ) );
		}
		AveragedDelta /= EG_To<eg_real>( NumSmoothSamples );
	}

	m_SmoothedSample = AveragedDelta;
}
