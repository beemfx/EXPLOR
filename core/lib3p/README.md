# lib3p
3rd Party libraries used within Emergence Engine Technology.

This folder contains all the Third Party Libraries that Emergence Engine
utilizes. Each of these is subject to their own copyright and license. Most of
these libraries have been modified in the sense that only the applicable code is
included in this distribution, excluding samples or otherwise unused code. None
of the included code is modified except in cases that it was needed to be built
and is indicated as such. Please refer to the associated license files for each
library to understand their respective licenses. A breif synopsis 
is found below.

## DirectXTKMar2015
* Developer: Microsoft
* License: MIT License
* License File: DirectXTKMar2015/MIT.txt

## expat
* Developer: Thai Open Source Software Center Ltd, and Clark Cooper, Expat maintainers
* License: Proprietary
* License File: expat/COPYING

## FreeType
* Developer: The FreeType Project
* License: The FreeType Project License
* License File: freetype225/FTL.txt

## fs_sys2
* Developer: Beem Media
* License: Licensed with this Distribution of Emergence Engine Technology
* License File: N/A

## libogg
* Developer: Xiph.org Foundation
* License: Proprietary
* License File: libogg132/COPYING

## libvorbis
* Developer: Xiph.org Foundation
* License: Proprietary
* License File: libvorbis135/COPYING

## xaudio2
* Developer: Microsoft
* License: MICROSOFT SOFTWARE LICENSE
* License File: N/A
* Note: Not included with the public release of Emergence Engine Technlogy since
technically the license does not allow for that, but the source code file is
linked to a NuGet package that should automatically install when building the
project in Visual Studio (or with MSBuild).

## xinput
* Developer: Microsoft
* License: MICROSOFT SOFTWARE LICENSE
* License File: N/A
* Note: Not included with the public release of Emergence Engine Technlogy since
technically the license does not allow for that, but it can be obtained in the
last release of the DirectX 9 SDK. Also, the game will build with XInput 1.4
which is available during a stardard installation of Visual Studio 2022.
