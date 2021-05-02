//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#ifndef _SCENE_H_INCLUDED_
#define _SCENE_H_INCLUDED_

//--------------------------------------------------------------------------------------
// Scene Geometry and Layout
//--------------------------------------------------------------------------------------


#include "pch.h"
#include "Scene.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "State.h"
#include "Shader.h"
#include "Input.h"
#include "Common.h"

#include "CVector2.h" 
#include "CVector3.h" 
#include "CMatrix4x4.h"
#include "MathHelpers.h"     // Helper functions for maths
#include "GraphicsHelpers.h" // Helper functions to unclutter the code here

#include "ColourRGBA.h" 

#include <sstream>
#include <memory>
#include "Texture.h"

// Prepare the geometry required for the scene
// Returns true on success
__declspec(dllexport) bool InitGeometry();

// Layout the scene
// Returns true on success
__declspec(dllexport) bool InitScene();

// Release the geometry resources created above
__declspec(dllexport) void ReleaseResources();


//--------------------------------------------------------------------------------------
// Scene Render and Update
//--------------------------------------------------------------------------------------

__declspec(dllexport) void RenderScene();

// frameTime is the time passed since the last frame
__declspec(dllexport) void UpdateScene(float frameTime);


#endif //_SCENE_H_INCLUDED_
