# E.X.P.L.O.R.: A New World

Updated: November 28, 2025

(c) 2025 Beem Media

Welcome to the release of the source code for E.X.P.L.O.R.: A New World. This repository includes the game code, the Emergence Game Engine, tools, and demo content.

## Building

We have done our best to create as smooth as possible an experience to build the source code. You sould be able to build the source with a default installation of Visual Studio 2022 Community Edition that includes the "Desktop Development with C++" workload. A connection to the internet may be required for Visual Studio and MSBuild to acquire required NuGet packages.

To get a basic build of the Game, Editor, and Tools you may run BuildDefaultGame.bat. This will work if Visual Studio is installed to the default location. Output will be found in the _BUILD directory.

You may also open EG.sln with Visual Studio 2022. When buildng this way, you'll want to set the Solution Configuration to "DebugEditor" or "ReleaseEditor", set the Platform to "x64", set the default project to "ExGame". Building ExGame will build everything needed to run the game and tools. Running  the editor will run the game in "Editor Mode". When running the game in Editor Mode it will build all the game data and run the game as if it were a normal game.

If you only want to run the game against pre-built data (such as data purchased through Steam), you may select "Debug" or "Release" for the configuration instead. If building the source code in this manner you should then specify the debug working directory as the directory where the game content is located.

## Modifying the Game

If you want to build your own game using the source code you can use EGEdApp_x64.exe (this will be built when running BuildDefaultGame.bat, but if building from Visual Studio you may need to build this yourself). This will open an asset browser that provides an interface for opening editor applications than can modify and create game assets. Note that we never optimized how the editor builds assets so modifying a single asset may cause long asset build times for the entire game. Also note that the editor is primitive and not a commercial product so using it will be very finicky.

If you want to make new map for the game, we recommend Grid Cartographer (https://store.steampowered.com/app/684690/Grid_Cartographer/) this is the software we used to create maps during development. Grid Cartographer maps are not natively supported, they first need to be exported to XML format and then imported into the game. You can find an example of the raw maps in the rawassets\ExGame\mapsf directory. EGEdApp_x64.exe can be used to import assets from the rawassets directory into the game.

For example:
<pre>
C:\Program Files (x86)\Steam\steamapps\common\EXPLOR A New World
</pre>

## About the Licensing

### Source Code

Beem Media is releasing the source code found in this repository under the license indicated by the LICENSE document found in the root directory of the repository. This applies to source code created by Beem Media. Additional source code may be subject to its own copyright and license, such instances will generally be noted as such in the source code itself or in adjacent documents.

### Demo Assets

Beem Media is including demo assets so that the source code may be built and tested without requiring the commerical version of the game. Assets that are not copyrighted by Beem Media are indicated as such in the assets themselves or in adjacent documents. Any assets that have been created by Beem Media are subject to the Creative Commons
Attribution-NonCommercial-NoDerivatives 4.0 International (https://creativecommons.org/licenses/by-nc-nd/4.0/).
