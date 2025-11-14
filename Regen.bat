echo off

rem Remove visual studio project files
del *.sln
rmdir /s /q .vs
rmdir /s /q Binaries
rmdir /s /q Intermediate
rmdir /s /q Saved
rmdir /s /q DerivedDataCache

rem Set the paths to Version Selector and Build Tool
set UVS="C:\Program Files (x86)\Epic Games\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe"
set UBT="D:\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe"

rem Set project path
set ProjPath="%cd%\ProjectRobot"

rem Regenerate Project Files
%UVS% /projectfiles %ProjPath%.uproject

%UBT% Development Win64 -Project=%ProjPath%.uproject -TargetType=Editor -Progress -NoEngineChanges -NoHotReloadFromIDE

rem Open up Visual Studio again
%ProjPath%.sln