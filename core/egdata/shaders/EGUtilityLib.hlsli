// (c) 2020 Beem Media. All rights reserved.

void EG_SelectAnimFrame( out float2 OutCoord , in float2 InCoord , in uint SelectedFrame , in const uint SPRITE_COLUMNS , in const uint SPRITE_ROWS )
{
	const uint NUM_FRAMES = SPRITE_COLUMNS*SPRITE_ROWS;

	uint nFrame = SelectedFrame;
	nFrame %= NUM_FRAMES;
	nFrame = clamp(nFrame,0,NUM_FRAMES-1); // May not need this but it does make sure we never go out of range...

	uint nX = nFrame%SPRITE_COLUMNS;
	uint nY = nFrame/SPRITE_COLUMNS;


	OutCoord = InCoord + float2(nX*1.f/SPRITE_COLUMNS, nY*1.f/SPRITE_ROWS);
}
