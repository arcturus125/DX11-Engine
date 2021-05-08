//--------------------------------------------------------------------------------------
// Commonly used definitions across entire project
//--------------------------------------------------------------------------------------
#ifndef _COMMON_H_INCLUDED_
#define _COMMON_H_INCLUDED_

#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <string>

//#include "CVector3.h"
#include "CMatrix4x4.h"
#include "Light.h"
#include <vector>
#include "Camera.h"
#include "Texture.h"

class Shader;


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// Make global Variables from various files available to other files. "extern" means
// this variable is defined in another file somewhere. We should use classes and avoid
// use of globals, but done this way to keep code simpler so the DirectX content is
// clearer. However, try to architect your own code in a better way.

// Windows variables
extern HWND gHWnd;

// Viewport size
extern int gViewportWidth;
extern int gViewportHeight;


// Important DirectX variables
extern ID3D11Device*           gD3DDevice;
extern ID3D11DeviceContext*    gD3DContext;
extern IDXGISwapChain*         gSwapChain;
extern ID3D11RenderTargetView* gBackBufferRenderTarget;  // Back buffer is where we render to
extern ID3D11DepthStencilView* gDepthStencil;            // The depth buffer contains a depth for each back buffer pixel

// Input constsnts
extern const float ROTATION_SPEED;
extern const float MOVEMENT_SPEED;


// A global error message to help track down fatal errors - set it to a useful message
// when a serious error occurs
extern std::string gLastError;

extern std::vector<std::string> shaderPaths;
extern std::vector<std::string> mediaPaths;

extern Camera* mainCamera;
extern std::vector <Light*> gLights; 
extern std::vector <Texture*> textures;
extern std::vector <Shader*> shaders;


//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
// Variables sent over to the GPU each frame

// Data that remains constant for an entire frame, updated from C++ to the GPU shaders *once per frame*
// We hold them together in a structure and send the whole thing to a "constant buffer" on the GPU each frame when
// we have finished updating the scene. There is a structure in the shader code that exactly matches this one
struct PerFrameConstants
{
    // These are the matrices used to position the camera
    CMatrix4x4 viewMatrix;
    CMatrix4x4 projectionMatrix;
    CMatrix4x4 viewProjectionMatrix; // The above two matrices multiplied together to combine their effects


    Light::PointLightData light[4];
    Light::SpotLightData spotLight[4];
    Light::DirectionalLightData directionalLight[4];


    float timer;


    CVector3   ambientColour;
    float      specularPower;

    CVector3   cameraPosition;
    float      parallaxDepth;  // Depth of the parallax mapping effect

    CVector3   outlineColour;    // Cell shading outline colour
    float      outlineThickness; // Controls thickness of outlines for cell shading


    int numLights;
    int numSpotLights;
    int numDirectionalLights;


};


extern PerFrameConstants gPerFrameConstants;      // This variable holds the CPU-side constant buffer described above
extern ID3D11Buffer*     gPerFrameConstantBuffer; // This variable controls the GPU-side constant buffer matching to the above structure



// This is the matrix that positions the next thing to be rendered in the scene. Unlike the structure above this data can be
// updated and sent to the GPU several times every frame (once per model). However, apart from that it works in the same way.
struct PerModelConstants
{
    CMatrix4x4 worldMatrix;
    CVector3   objectColour; // Allows each light model to be tinted to match the light colour they cast
    float      padding6;
};
extern PerModelConstants gPerModelConstants;      // This variable holds the CPU-side constant buffer described above
extern ID3D11Buffer*     gPerModelConstantBuffer; // This variable controls the GPU-side constant buffer related to the above structure


#endif //_COMMON_H_INCLUDED_
