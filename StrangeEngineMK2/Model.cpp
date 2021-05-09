//--------------------------------------------------------------------------------------
// Class encapsulating a model
//--------------------------------------------------------------------------------------
// Holds a pointer to a mesh as well as position, rotation and scaling, which are converted to a world matrix when required
// This is more of a convenience class, the Mesh class does most of the difficult work.


#include "pch.h"
#include "Model.h"

#include "Scene.h"
#include "Common.h"
#include "GraphicsHelpers.h"
#include "Mesh.h"


std::vector< ID3D11SamplerState*> samplers;
std::vector< ID3D11BlendState*> blenders;
std::vector< ID3D11RasterizerState*> cullers;
std::vector< ID3D11DepthStencilState*> depthStates;

void SetupInterface()
{
	samplers = { gPointSampler, gTrilinearSampler, gAnisotropic4xSampler };
	blenders = { gNoBlendingState,gAdditiveBlendingState,gAlphaBlendingState };
	cullers = { gCullBackState, gCullFrontState, gCullNoneState };
	depthStates = { gUseDepthBufferState, gDepthReadOnlyState, gNoDepthBufferState };
}

Model::Model(Mesh* mesh, CVector3 position /*= { 0,0,0 }*/, CVector3 rotation /*= { 0,0,0 }*/, float scale /*= 1*/, bool autoRender /* = true*/)
    : mMesh(mesh)
{
	name = mMesh->filename;
	SetupInterface();
	// Set default matrices from mesh
	mWorldMatrices.resize(mesh->NumberNodes());
	for (int i = 0; i < mWorldMatrices.size(); ++i)
		mWorldMatrices[i] = mesh->GetNodeDefaultMatrix(i);
	// create the default pass of the renderer for the object
	RendererPass temp;
	renderPass.push_back(temp);

	SetShader(new Shader("PixelLighting"));
	SetSampler(gAnisotropic4xSampler);
	SetBlendingState(gNoBlendingState);
	SetCullingState(gCullBackState);
	SetDepthBufferState(gUseDepthBufferState);

	if(autoRender)
		autoRenderList.push_back(this);
	
}


void Model::AddRendererPass()
{
	RendererPass temp;
	renderPass.push_back(temp);
}
void Model::SetShader(Shader* s, int renderPassIndex)
{	 
	renderPass[renderPassIndex].shader = s;
}
void Model::AddTexture(Texture* t, int renderPassIndex)
{	 
	renderPass[renderPassIndex].textures.push_back(t);
}
void Model::SetSampler(ID3D11SamplerState* s, int renderPassIndex)
{	 
	renderPass[renderPassIndex].sampler.clear();	 
	renderPass[renderPassIndex].sampler.push_back(s);
}
void Model::AddSampler(ID3D11SamplerState* s, int renderPassIndex)
{	 
	renderPass[renderPassIndex].sampler.push_back(s);
}
void Model::SetSampler(int s, int renderPassIndex)
{
	renderPass[renderPassIndex].sampler.clear();
	renderPass[renderPassIndex].sampler.push_back(samplers[s]);
}
void Model::AddSampler(int s, int renderPassIndex)
{
	renderPass[renderPassIndex].sampler.push_back(samplers[s]);
}
void Model::SetBlendingState(ID3D11BlendState* b, int renderPassIndex)
{	 
	renderPass[renderPassIndex].blender = b;
}
void Model::SetBlendingState(int b, int renderPassIndex)
{
	renderPass[renderPassIndex].blender = blenders[b];
}
void Model::SetCullingState(ID3D11RasterizerState* c, int renderPassIndex)
{
	renderPass[renderPassIndex].culling = c;
}
void Model::SetCullingState(int c, int renderPassIndex)
{
	renderPass[renderPassIndex].culling = cullers[c];
}
void Model::SetDepthBufferState(ID3D11DepthStencilState* d, int renderPassIndex)
{
	renderPass[renderPassIndex].depth = d;
}
void Model::SetDepthBufferState(int d, int renderPassIndex)
{
	renderPass[renderPassIndex].depth = depthStates[d];
}



// The render function simply passes this model's matrices over to Mesh:Render.
// All other per-frame constants must have been set already along with shaders, textures, samplers, states etc.
void Model::Render()
{
    //mMesh->RenderRecursive(mWorldMatrices);
    mMesh->Render(mWorldMatrices);
}

void Model::AutoRender(ID3D11DeviceContext* cBufferConstants, int rendererIndex)
{
	cBufferConstants->VSSetShader(renderPass[rendererIndex].shader->vertexShader, nullptr, 0);
	cBufferConstants->PSSetShader(renderPass[rendererIndex].shader->pixelShader, nullptr, 0);
	cBufferConstants->OMSetBlendState(renderPass[rendererIndex].blender, nullptr, 0xffffff);
	cBufferConstants->OMSetDepthStencilState(renderPass[rendererIndex].depth, 0);

	cBufferConstants->RSSetState(renderPass[rendererIndex].culling);

	for (int i = 0; i < renderPass[rendererIndex].sampler.size(); i++)
	{
		cBufferConstants->PSSetSamplers(i, 1, &renderPass[rendererIndex].sampler[i]);
	}

	for (int i = 0; i < renderPass[rendererIndex].textures.size(); i++)
	{
		cBufferConstants->PSSetShaderResources(i, 1, &renderPass[rendererIndex].textures[i]->gTextureMapSRV);
	}
	mMesh->Render(mWorldMatrices);
}


// Control a given node in the model using keys provided. Amount of motion performed depends on frame time
void Model::Control(int node, float frameTime, KeyCode turnUp, KeyCode turnDown, KeyCode turnLeft, KeyCode turnRight,
                                               KeyCode turnCW, KeyCode turnCCW, KeyCode moveForward, KeyCode moveBackward)
{
    auto& matrix = mWorldMatrices[node]; // Use reference to node matrix to make code below more readable

	if (KeyHeld( turnUp ))
	{
		matrix = MatrixRotationX(ROTATION_SPEED * frameTime) * matrix;
	}
	if (KeyHeld( turnDown ))
	{
		matrix = MatrixRotationX(-ROTATION_SPEED * frameTime) * matrix;
	}
	if (KeyHeld( turnRight ))
	{
		matrix = MatrixRotationY(ROTATION_SPEED * frameTime) * matrix;
	}
	if (KeyHeld( turnLeft ))
	{
		matrix = MatrixRotationY(-ROTATION_SPEED * frameTime) * matrix;
	}
	if (KeyHeld( turnCW ))
	{
		matrix = MatrixRotationZ(ROTATION_SPEED * frameTime) * matrix;
	}
	if (KeyHeld( turnCCW ))
	{
		matrix = MatrixRotationZ(-ROTATION_SPEED * frameTime) * matrix;
	}

	// Local Z movement - move in the direction of the Z axis, get axis from world matrix
    CVector3 localZDir = Normalise(matrix.GetRow(2)); // normalise axis in case world matrix has scaling
	if (KeyHeld( moveForward ))
	{

		matrix.SetRow(3, matrix.GetRow(3) + localZDir * MOVEMENT_SPEED * frameTime);
	}
	if (KeyHeld( moveBackward ))
	{
		matrix.SetRow(3, matrix.GetRow(3) - localZDir * MOVEMENT_SPEED * frameTime);
	}
}
