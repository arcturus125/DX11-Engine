//--------------------------------------------------------------------------------------
// Class encapsulating a mesh
//--------------------------------------------------------------------------------------
// The mesh class splits the mesh into sub-meshes that only use one texture each.
// The class also doesn't load textures, filters or shaders as the outer code is
// expected to select these things. A later lab will introduce a more robust loader.

#include "Mesh.h"
#include "Shader.h" // Needed for helper function CreateSignatureForVertexLayout
#include "CVector2.h" 
#include "CVector3.h" 
#include "GraphicsHelpers.h" // Helper functions to unclutter the code here

#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <memory>


// Pass the name of the mesh file to load. Uses assimp (http://www.assimp.org/) to support many file types
// Optionally request tangents to be calculated (for normal and parallax mapping - see later lab)
// Will throw a std::runtime_error exception on failure (since constructors can't return errors).
Mesh::Mesh(const std::string& fileName, bool requireTangents /*= false*/)
{
    Assimp::Importer importer;

    // Flags for processing the mesh. Assimp provides a huge amount of control - right click any of these
    // and "Peek Definition" to see documention above each constant
    unsigned int assimpFlags = aiProcess_MakeLeftHanded |
                               aiProcess_GenSmoothNormals |
                               aiProcess_FixInfacingNormals |
                               aiProcess_GenUVCoords | 
                               aiProcess_TransformUVCoords |
                               aiProcess_FlipUVs |
                               aiProcess_FlipWindingOrder |
                               aiProcess_Triangulate |
                               aiProcess_JoinIdenticalVertices |
                               aiProcess_ImproveCacheLocality |
                               aiProcess_SortByPType |
                               aiProcess_FindInvalidData | 
                               aiProcess_OptimizeMeshes |
                               aiProcess_FindInstances |
                               aiProcess_FindDegenerates |
                               aiProcess_RemoveRedundantMaterials |
                               aiProcess_Debone |
                               aiProcess_RemoveComponent;

    // Flags to specify what mesh data to ignore
    int removeComponents = aiComponent_LIGHTS | aiComponent_CAMERAS | aiComponent_TEXTURES | aiComponent_COLORS | 
                           aiComponent_BONEWEIGHTS | aiComponent_ANIMATIONS | aiComponent_MATERIALS;

    // Add / remove tangents as required by user
    if (requireTangents)
    {
        assimpFlags |= aiProcess_CalcTangentSpace;
    }
    else
    {
        removeComponents |= aiComponent_TANGENTS_AND_BITANGENTS;
    }

    // Other miscellaneous settings
    importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f); // Smoothing angle for normals
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);  // Remove points and lines (keep triangles only)
    importer.SetPropertyBool(AI_CONFIG_PP_FD_REMOVE, true);                 // Remove degenerate triangles
    importer.SetPropertyBool(AI_CONFIG_PP_DB_ALL_OR_NONE, true);            // Default to removing bones/weights from meshes that don't need skinning
  
    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, removeComponents);

    // Import mesh with assimp given above requirements - log output
    Assimp::DefaultLogger::create("", Assimp::DefaultLogger::VERBOSE);
    const aiScene* scene = importer.ReadFile(fileName, assimpFlags);
    Assimp::DefaultLogger::kill();
    if (scene == nullptr)  throw std::runtime_error("Error loading mesh (" + fileName + "). " + importer.GetErrorString());
    if (scene->mNumMeshes == 0)  throw std::runtime_error("No usable geometry in mesh: " + fileName);


    //-----------------------------------

    //******************************************//
    // Read geometry - multiple parts supported //

    // A mesh is made of sub-meshes, each one can have a different material (texture)
    // Import each sub-mesh in the file to seperate index / vertex buffer (could share buffers between sub-meshes but that would make things more complex)
    mSubMeshes.resize(scene->mNumMeshes);
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
    {
        aiMesh* assimpMesh = scene->mMeshes[m];
        std::string subMeshName = assimpMesh->mName.C_Str();
        auto& subMesh = mSubMeshes[m]; // Short name for the submesh we're currently preparing - makes code below more readable

    
        //-----------------------------------

        // Check for presence of position and normal data. Tangents and UVs are optional.
        std::vector<D3D11_INPUT_ELEMENT_DESC> vertexElements;
        unsigned int offset = 0;
    
        if (!assimpMesh->HasPositions())  throw std::runtime_error("No position data for sub-mesh " + subMeshName + " in " + fileName);
        unsigned int positionOffset = offset;
        vertexElements.push_back( { "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, positionOffset, D3D11_INPUT_PER_VERTEX_DATA, 0 } );
        offset += 12;

        if (!assimpMesh->HasNormals())  throw std::runtime_error("No normal data for sub-mesh " + subMeshName + " in " + fileName);
        unsigned int normalOffset = offset;
        vertexElements.push_back( { "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, normalOffset, D3D11_INPUT_PER_VERTEX_DATA, 0 } );
        offset += 12;

        unsigned int tangentOffset = offset;
        if (requireTangents)
        {
            if (!assimpMesh->HasTangentsAndBitangents())  throw std::runtime_error("No tangent data for sub-mesh " + subMeshName + " in " + fileName);
            vertexElements.push_back( { "Tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, tangentOffset, D3D11_INPUT_PER_VERTEX_DATA, 0 } );
            offset += 12;
        }
    
        unsigned int uvOffset = offset;
        if (assimpMesh->GetNumUVChannels() > 0 && assimpMesh->HasTextureCoords(0))
        {
            if (assimpMesh->mNumUVComponents[0] != 2)  throw std::runtime_error("Unsupported texture coordinates in " + subMeshName + " in " + fileName);
            vertexElements.push_back( { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, uvOffset, D3D11_INPUT_PER_VERTEX_DATA, 0 } );
            offset += 8;
        }

        subMesh.vertexSize = offset;


        // Create a "vertex layout" to describe to DirectX what is data in each vertex of this mesh
        auto shaderSignature = CreateSignatureForVertexLayout(vertexElements.data(), static_cast<int>(vertexElements.size()));
        HRESULT hr = gD3DDevice->CreateInputLayout(vertexElements.data(), static_cast<UINT>(vertexElements.size()),
                                                   shaderSignature->GetBufferPointer(), shaderSignature->GetBufferSize(),
                                                   &subMesh.vertexLayout);
        if (shaderSignature)  shaderSignature->Release();
        if (FAILED(hr))  throw std::runtime_error("Failure creating input layout for " + fileName);



        //-----------------------------------

        // Create CPU-side buffers to hold current mesh data - exact content is flexible so can't use a structure for a vertex - so just a block of bytes
        // Note: for large arrays a unique_ptr is better than a vector because vectors default-initialise all the values which is a waste of time.
        subMesh.numVertices = assimpMesh->mNumVertices;
        subMesh.numIndices  = assimpMesh->mNumFaces * 3;
        auto vertices = std::make_unique<unsigned char[]>(subMesh.numVertices * subMesh.vertexSize);
        auto indices  = std::make_unique<unsigned char[]>(subMesh.numIndices * 4); // Using 32 bit indexes (4 bytes) for each indeex


        //-----------------------------------

        // Copy mesh data from assimp to our CPU-side vertex buffer

        CVector3* assimpPosition = reinterpret_cast<CVector3*>(assimpMesh->mVertices);
        unsigned char* position = vertices.get() + positionOffset;
        unsigned char* positionEnd = position + subMesh.numVertices * subMesh.vertexSize;
        while (position != positionEnd)
        {
            *(CVector3*)position = *assimpPosition;
            position += subMesh.vertexSize;
            ++assimpPosition;
        }

        CVector3* assimpNormal = reinterpret_cast<CVector3*>(assimpMesh->mNormals);
        unsigned char* normal = vertices.get() + normalOffset;
        unsigned char* normalEnd = normal + subMesh.numVertices * subMesh.vertexSize;
        while (normal != normalEnd)
        {
            *(CVector3*)normal = *assimpNormal;
            normal += subMesh.vertexSize;
            ++assimpNormal;
        }

        if (requireTangents)
        {
          CVector3* assimpTangent = reinterpret_cast<CVector3*>(assimpMesh->mTangents);
          unsigned char* tangent =  vertices.get() + tangentOffset;
          unsigned char* tangentEnd = tangent + subMesh.numVertices * subMesh.vertexSize;
          while (tangent != tangentEnd)
          {
            *(CVector3*)tangent = *assimpTangent;
            tangent += subMesh.vertexSize;
            ++assimpTangent;
          }
        }

        if (assimpMesh->GetNumUVChannels() > 0 && assimpMesh->HasTextureCoords(0))
        {
            aiVector3D* assimpUV = assimpMesh->mTextureCoords[0];
            unsigned char* uv = vertices.get() + uvOffset;
            unsigned char* uvEnd = uv + subMesh.numVertices * subMesh.vertexSize;
            while (uv != uvEnd)
            {
                *(CVector2*)uv = CVector2(assimpUV->x, assimpUV->y);
                uv += subMesh.vertexSize;
                ++assimpUV;
            }
        }


        //-----------------------------------

        // Copy face data from assimp to our CPU-side index buffer
        if (!assimpMesh->HasFaces())  throw std::runtime_error("No face data in " + subMeshName + " in " + fileName);

        DWORD* index = reinterpret_cast<DWORD*>(indices.get());
        for (unsigned int face = 0; face < assimpMesh->mNumFaces; ++face)
        {
            *index++ = assimpMesh->mFaces[face].mIndices[0];
            *index++ = assimpMesh->mFaces[face].mIndices[1];
            *index++ = assimpMesh->mFaces[face].mIndices[2];
        }


        //-----------------------------------

        D3D11_BUFFER_DESC bufferDesc;
        D3D11_SUBRESOURCE_DATA initData;

        // Create GPU-side vertex buffer and copy the vertices imported by assimp into it
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Indicate it is a vertex buffer
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;          // Default usage for this buffer - we'll see other usages later
        bufferDesc.ByteWidth = subMesh.numVertices * subMesh.vertexSize; // Size of the buffer in bytes
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        initData.pSysMem = vertices.get(); // Fill the new vertex buffer with data loaded by assimp
    
        hr = gD3DDevice->CreateBuffer(&bufferDesc, &initData, &subMesh.vertexBuffer);
        if (FAILED(hr))  throw std::runtime_error("Failure creating vertex buffer for " + fileName);


        // Create GPU-side index buffer and copy the vertices imported by assimp into it
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER; // Indicate it is an index buffer
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;         // Default usage for this buffer - we'll see other usages later
        bufferDesc.ByteWidth = subMesh.numIndices * sizeof(DWORD); // Size of the buffer in bytes
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        initData.pSysMem = indices.get(); // Fill the new index buffer with data loaded by assimp

        hr = gD3DDevice->CreateBuffer(&bufferDesc, &initData, &subMesh.indexBuffer);
        if (FAILED(hr))  throw std::runtime_error("Failure creating index buffer for " + fileName);
    }



    //*********************************************************************//
    // Read node hierachy - each node has a matrix and contains sub-meshes //

    // Uses recursive helper functions to build node hierarchy    
    mNodes.resize(CountNodes(scene->mRootNode));
    ReadNodes(scene->mRootNode, 0, 0);
}


Mesh::~Mesh()
{
    for (auto& subMesh : mSubMeshes)
    {
        if (subMesh.indexBuffer)   subMesh.indexBuffer ->Release();
        if (subMesh.vertexBuffer)  subMesh.vertexBuffer->Release();
        if (subMesh.vertexLayout)  subMesh.vertexLayout->Release();
    }
}

// Helper function for Render function - sends the world matrix for the next object to render over to the GPU
void Mesh::SetWorldMatrixOnGPU(CMatrix4x4 worldMatrix)
{
    gPerModelConstants.worldMatrix = worldMatrix; // Update C++ side constant buffer
    UpdateConstantBuffer(gPerModelConstantBuffer, gPerModelConstants); // Send to GPU

    // Indicate that the constant buffer we just updated is for use in the vertex shader (VS) and pixel shader (PS)
    gD3DContext->VSSetConstantBuffers(1, 1, &gPerModelConstantBuffer); // First parameter must match constant buffer number in the shader
    gD3DContext->PSSetConstantBuffers(1, 1, &gPerModelConstantBuffer);
}

// Helper function for Render function - renders all the submeshes of the given node. World matrix must already be set
void Mesh::RenderNodeSubMeshes(unsigned int nodeIndex)
{
    auto& node = mNodes[nodeIndex];
    for (auto& subMeshIndex : node.subMeshes)
    {
        auto& subMesh = mSubMeshes[subMeshIndex];

        // Set vertex buffer as next data source for GPU
        UINT stride = subMesh.vertexSize;
        UINT offset = 0;
        gD3DContext->IASetVertexBuffers(0, 1, &subMesh.vertexBuffer, &stride, &offset);

        // Indicate the layout of vertex buffer
        gD3DContext->IASetInputLayout(subMesh.vertexLayout);

        // Set index buffer as next data source for GPU, indicate it uses 32-bit integers
        gD3DContext->IASetIndexBuffer(subMesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // Using triangle lists only in this class
        gD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Render mesh
        gD3DContext->DrawIndexed(subMesh.numIndices, 0, 0);
    }
}


// Render a given node in the mesh. Recursive function.
// - modelMatrices are sent from the Model - one matrix for each node in the mesh, representing it's current "pose". Matrices are relative to the parent node.
// - nodeIndex is the index of the current node, for accessing the vectors of matrices and node information
// - parentWorldMatrix is the world matrix that was calculated for the parent in a previous call, it's used to make relative matrices into absolute matrices
void Mesh::RenderRecursive(std::vector<CMatrix4x4>& modelMatrices, unsigned int nodeIndex/* = 0*/, CMatrix4x4 parentWorldMatrix /*= MatrixIdentity()*/)
{
    // 1. Calculate absolute world matrix for the current node index. The modelMatrices are relative to the parent node so:
    //       Absolute world matrix = model matrix of this node * parent's world matrix
    CMatrix4x4 nodeWorldMatrix = modelMatrices[nodeIndex] * parentWorldMatrix;

    // 2. Set that absolute world matrix on the GPU (call the function shortly above)
    SetWorldMatrixOnGPU(nodeWorldMatrix);
    
    // 3. Now the world matrix is ready on the GPU, render the sub-meshes for this node (function above)
    RenderNodeSubMeshes(nodeIndex);

    // 4. Loop through all the child nodes of this node and render each one with this function (recursive call)
    //    Use mNodes[nodeIndex] to access node information. Think carefully what the parent world matrix parameter should be in the call

    for (int i = 1; i < mNodes[nodeIndex].childNodes.size(); i++)
    {
        RenderRecursive(modelMatrices, i + nodeIndex, nodeWorldMatrix);
    }
}


// Render all the nodes in the mesh without recursion, faster alternative to above (lab exercise)
void Mesh::Render(std::vector<CMatrix4x4>& modelMatrices)
{
    // Loop through all nodes. Node 0 is the root and the remaining nodes are in "flattened" depth-first order.
    // Parent nodes will always come before their children in the vector and we use that fact to avoid recursion.
    // We only needed the recursion to pass the parent's absolute world matrix to the children. Instead, store the
    // absolute world matrix then the children can refer back to their parent to get it. 
    for (unsigned int nodeIndex = 0; nodeIndex < mNodes.size(); ++nodeIndex)
    {
        Node& thisNode = mNodes[nodeIndex]; // Use a reference to this node to make the variable name more readable. Use this trick to simplify your own code (no cost)

        // 1. Calculate absolute world matrix for the current node index and store it in thisNode.absoluteMatrix
        //        If we are on the root node
        //            Absolute world matrix is just the model matrix for the node
        //        Otherwise
        //            Absolute world matrix = model matrix of this node * parent's absolute matrix (get the parent index from thisNode)
        //    It would be even faster to "unroll" the first iteration of this loop: write the code for the root, then a loop for the remaining nodes.
        if (nodeIndex == 0)
            thisNode.absoluteMatrix = modelMatrices[0];
        else
        {
            thisNode.absoluteMatrix = modelMatrices[nodeIndex] * mNodes[thisNode.parentIndex].absoluteMatrix;
        }

        // 2. Set that absolute world matrix on the GPU (call the function shortly above)
        SetWorldMatrixOnGPU(thisNode.absoluteMatrix);
    
        // 3. Now the world matrix is ready on the GPU, render the sub-meshes for this node (function above)
        RenderNodeSubMeshes(nodeIndex);
    }
}



//--------------------------------------------------------------------------------------
// Helper functions
//--------------------------------------------------------------------------------------

// Count the number of nodes with given assimp node as root - recursive
unsigned int Mesh::CountNodes(aiNode* assimpNode)
{
    unsigned int count = 1;
    for (unsigned int child = 0; child < assimpNode->mNumChildren; ++child)
        count += CountNodes(assimpNode->mChildren[child]);
    return count;
}


// Help build the arrays of submeshes and nodes from the assimp data - recursive
unsigned int Mesh::ReadNodes(aiNode* assimpNode, unsigned int nodeIndex, unsigned int parentIndex)
{
    auto& node = mNodes[nodeIndex];
    node.parentIndex = parentIndex;
    unsigned int thisIndex = nodeIndex;
    ++nodeIndex;

    node.defaultMatrix.SetValues(&assimpNode->mTransformation.a1);
    node.defaultMatrix.Transpose(); // Assimp stores matrices differently to this app

    node.subMeshes.resize(assimpNode->mNumMeshes);
    for (unsigned int i = 0; i < assimpNode->mNumMeshes; ++i)
    {
        node.subMeshes[i] = assimpNode->mMeshes[i];
    }

    node.childNodes.resize(assimpNode->mNumChildren);
    for (unsigned int i = 0; i < assimpNode->mNumChildren; ++i)
    {
        node.childNodes[i] = nodeIndex;
        nodeIndex = ReadNodes(assimpNode->mChildren[i], nodeIndex, thisIndex);
    }

    return nodeIndex;
}
