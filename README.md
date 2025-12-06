# E.X.P.L.O.R.: A New World

Updated: December 6, 2025

(c) 2025 Beem Media

Welcome to the release of the source code for E.X.P.L.O.R.: A New World. This repository includes the game code, the Emergence Game Engine, tools, and demo content.

## History

I first started developing E.X.P.L.O.R.: A New World around 2000. The earliest log entry I could find for it is dated August 6, 2000, so I know it was at least started by then. The actual start date could have been much earlier, possibly even a year or two earlier. The retail version of the game was released June 11, 2021 so, regardless of when I started it, I can proudly say it took over 20 years to make!

I started making the game because I had the NES version of Might and Magic. My copy of that game would constantly reset its save state so I had trouble making progress in the game. My answer to those problems was to program my own Might and Magic game and thus began the project.

I'm proud I finished making the game and sent it to market. I made the game I wanted to play and I myself find it fun. A few others have found it fun as well and I'm thankful for that.

In 2021 I was proud of the source code. Since that time I have written much better code, so I admit when I look at the code now it's kind of embarassing, but I still want to present it for anyone that is curious.

For historical versions of the game, and additional information, see: https://github.com/beemfx/EXPLOR-Betas

## Building

We have done our best to create as smooth as possible an experience to build the source code. You sould be able to build the source with a default installation of Visual Studio 2022 Community Edition that includes the "Desktop Development with C++" workload. A connection to the internet may be required for Visual Studio and MSBuild to acquire required NuGet packages.

To get a basic build of the Game, Editor, and Tools you may run BuildDefaultGame.bat. This will work if Visual Studio is installed to the default location. Output will be found in the _BUILD directory.

You may also open EG.sln with Visual Studio 2022. When buildng this way, you'll want to set the Solution Configuration to "DebugEditor" or "ReleaseEditor", set the Platform to "x64", set the default project to "ExGame". Building ExGame will build everything needed to run the game and tools. Running  the editor will run the game in "Editor Mode". When running the game in Editor Mode it will build all the game data and run the game as if it were a normal game. (There is a bug where if you run the game in editor mode and the data build lasts a long time the game will exit, so you may have to run the game twice to actually get into the game if you do this.)

If you only want to run the game against pre-built data (such as data purchased through Steam), you may select "Debug" or "Release" for the configuration instead. If building the source code in this manner you should then specify the debug working directory as the directory where the game content is located.

For example:
<pre>
C:\Program Files (x86)\Steam\steamapps\common\EXPLOR A New World
</pre>

## Modifying the Game

If you want to build your own game using the source code you can use EGEdApp_x64.exe (this will be built when running BuildDefaultGame.bat, but if building from Visual Studio you may need to build this yourself). This will open an asset browser that provides an interface for opening editor applications than can modify and create game assets. Note that we never optimized how the editor builds assets so modifying a single asset may cause long data build times for the entire game. Also note that the editor is primitive and not a commercial product so using it will be very finicky and there is no documentation.

If you want to make new map for the game, we recommend Grid Cartographer (https://store.steampowered.com/app/684690/Grid_Cartographer/) this is the software we used to create maps during development. Grid Cartographer maps are not natively supported, they first need to be exported to XML format and then imported into the game. You can find an example of the raw maps in the ExGame\data_src\mapsf directory. EGEdApp_x64.exe can be used to import assets from the data_src directory into the game.

Note that this source code has some differences from the retail version of the game, but it should be fully compatible with the retail data.

## About the Licensing

### Source Code

Beem Media is releasing the source code found in this repository under the license indicated by the LICENSE document found in the root directory of the repository. This applies to source code created by Beem Media specifically for EXPLOR. Additional source code may be subject to its own copyright and license, such instances will generally be noted as such in the source code itself or in adjacent documents.

The Emergence Engine source code is licensed by the LICENSE file found in "core/LICENSE" which is a less strict license than the game code. See the associated core/README.md for more information about the Emergence Engine.

### Demo Assets

Beem Media is including demo assets so that the source code may be built and tested without requiring the commerical version of the game. Assets that are not copyrighted by Beem Media are indicated as such in the assets themselves or in adjacent documents. Any assets that have been created by Beem Media are subject to the Creative Commons
Attribution-NonCommercial-NoDerivatives 4.0 International (https://creativecommons.org/licenses/by-nc-nd/4.0/).
