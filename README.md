# StrangeEngine
![StrangeEngine Icon](https://github.com/arcturus125/StrangeEngine/blob/main/Media/icon.png)

## History
Originally a part of an assignment for the computer graphics module on my games development course.
after finishing the assignment, this project closely resembled the start of a game engine. 
Suppose i got lucky as i was planning on making a game engine as my final year project.
this was a good starting point.

## Contents

### New Project Wizard
here you with find an executable, use this to create a new StrangeEngine project wverever you like. 
once a new project is created, it should be linked to StrangeEngine already the engine should be reasy-to-use
NOTE: if the engine doesnt work out of the box after creating a new project, you need to clone this repo into a specific location or it wont work. (see installation instructions below)

### StrangeEngine MK2
this is the engine itself. 
it is named Mark 2 because while i was converting the engine to a .dll format that multiple projects could use, i had 2 concurrent versions of the engine i was working on.
Mark 1 was a .exe file where each game needed to be hardcoded into scene.cpp and there was significantly less flexibility (Mark 1 was also my assignment submission for anyone interested)
Mark 2 is the latest and greatest version of the engine, still a work in progress, but miles more flexible than previous versions

### StrangeEngine Runnable
this is a project that is directly linked to the engine, Think of it as a Technical Demo.
For anyone who is modifying the engine, i suggest doing all your tests in this project as this and the engine are in the same solution so it is very easy to work on both

## Installation Instructions
When you clone/download this repository, all  the contents of the repository must be stored in the followign directory:

    C:\StrangeEngine
    
 **if you do not store this repository in the correct location, it will not work**
 
 i realise this may not be very friendly, but i am very new to .dll projects and linker issues, to be honest, it's a miracle i got even this working

# Project properties

## StrangeEngineMK3

#### General
 Output Directory:
    `$(ProjectDir)$(Configuration)\`
    
 IntermediateDirectory:
    `$(Configuration)\`
    
### VC++ Directories
 Include Directories: 
    `$(DXSDK_DIR)Include;$(IncludePath)`

 Library Directories:
    `$(DXSDK_DIR)Lib\$(PlatformTarget);$(LibraryPath)`
    
### Linker > Input

Additional Dependencies (Debug): `d3d11.lib;d3dx11.lib;D3DCompiler.lib;dxerr.lib;dxgi.lib;dxguid.lib;%(AdditionalDependencies)`
Additional Dependencies (Release): `d3d11.lib;d3dx11d.lib;D3DCompiler.lib;dxerr.lib;dxgi.lib;dxguid.lib;%(AdditionalDependencies)`

## Project (StrangeEngine_Runnable and StrangeEngine_BlankProject)

### General
 Output Directory: `$(ProjectDir)$(Configuration)\`

 Intermediate Directory: `$(Configuration)\`
 
 
### VC++ Directories
 Include Directories: 
    `$(DXSDK_DIR)Include;$(IncludePath)`

 Library Directories:
    `$(DXSDK_DIR)Lib\$(PlatformTarget);$(LibraryPath)`
    
    
### Linker > General

Additional Library Directories:  `C:\StrangeEngine\StrangeEngineMK3\$(IntDir);%(AdditionalLibraryDirectories)`

### Linker > Input

Additional Dependencies (Debug): `StrangeEngineMK3.lib;d3d11.lib;d3dx11d.lib;D3DCompiler.lib;dxerr.lib;dxgi.lib;dxguid.lib;%(AdditionalDependencies)`
Additional Dependencies (Release): `StrangeEngineMK3.lib;d3d11.lib;d3dx11.lib;D3DCompiler.lib;dxerr.lib;dxgi.lib;dxguid.lib;%(AdditionalDependencies)`

### Build Events > Pots-Build Event

Command line; `xcopy /y /d "C:\StrangeEngine\StrangeEngineMK3\$(IntDir)StrangeEngineMK3.dll" "$(ProjectDir)$(Configuration)"`

    


