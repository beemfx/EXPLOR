// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

enum class eg_tween_animation_t
{
	Linear,
	Cubic,
};


template<typename T>
class EGTween
{
private:

	T StartValue;
	T EndValue;
	T CurrentValue;
	eg_real Duration = 0.f;
	eg_tween_animation_t TweenType = eg_tween_animation_t::Linear;
	eg_real TimeElapsed = 0.f;
	eg_bool bActive = false;

public:

	EGTween() = default;
	EGTween( eg_ctor_t Ct ): CurrentValue( Ct ) , StartValue( Ct ) , EndValue( Ct ) { }
	EGTween( const T& rhs ): CurrentValue( rhs ) , StartValue( rhs ) , EndValue( rhs ) { }

	eg_bool IsAnimating() const { return bActive; }

	const T& GetCurrentValue() const { return CurrentValue; }
	const T& GetEndValue() const { return EndValue; }

	void SetValue( const T& InValue )
	{
		StartValue = InValue;
		EndValue = InValue;
		CurrentValue = InValue;
		bActive = false;
		Duration = 0.f;
		TimeElapsed = 0.f;
		TweenType = eg_tween_animation_t::Linear;
	}

	void MoveTo( const T& NewValue , eg_real InDuration , eg_tween_animation_t InTweenType )
	{
		StartValue = CurrentValue;
		EndValue = NewValue;
		bActive = true;
		TimeElapsed = 0.f;
		Duration = InDuration;
		TweenType = InTweenType;

		Update( 0.f );
	}

	void Update( eg_real DeltaTime )
	{
		if( bActive )
		{
			TimeElapsed += DeltaTime;
			if( TimeElapsed >= Duration || Duration <= 0.f )
			{
				CurrentValue = EndValue;
				bActive = false;
			}
			else
			{
				eg_real NormalizedTransition = TimeElapsed / Duration;
				if( TweenType == eg_tween_animation_t::Cubic )
				{
					NormalizedTransition = EGMath_CubicInterp( 0.f , 0.f , 1.f , 0.f , NormalizedTransition );
				}

				ApplyTransitionTime( NormalizedTransition );
			}
		}
	}

	void ApplyTransitionTime( eg_real NormalizedTransition );
};

template<typename T>
inline void EGTween<T>::ApplyTransitionTime( eg_real NormalizedTransition )
{
	CurrentValue = StartValue * (1.f - NormalizedTransition) + EndValue * NormalizedTransition;
}

template<>
void EGTween<eg_transform>::ApplyTransitionTime( eg_real NormalizedTransition );