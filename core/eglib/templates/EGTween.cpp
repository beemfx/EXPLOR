// (c) 2020 Beem Media. All Rights Reserved.

#include "EGTween.h"

template<>
void EGTween<eg_transform>::ApplyTransitionTime( eg_real NormalizedTransition )
{
	CurrentValue = eg_transform::Lerp( StartValue , EndValue , NormalizedTransition );
}
