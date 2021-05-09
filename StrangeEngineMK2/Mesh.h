//--------------------------------------------------------------------------------------
// Class encapsulating a mesh
//--------------------------------------------------------------------------------------
// The mesh class splits the mesh into sub-meshes that only use one texture each.
// The class also doesn't load textures, filters or shaders as the outer code is
// expected to select these things

#include "common.h"

#include <assimp/scene.h>

#include <string>
#include <vector>

#ifndef _MESH_H_INCLUDED_
#define _MESH_H_INCLUDED_

class Mesh
{
//--------------------------------------------------------------------------------------
// Construction / Usage
//--------------------------------------------------------------------------------------
public:
    std::string filename;

    // Pass the name of the mesh file to load. Uses assimp (http://www.assimp.org/) to support many file types
    // Optionally request tangents to be calculated (for normal and parallax mapping - see later lab)
    // Will throw a std::runtime_error exception on failure (since constructors can't return errors).
    __declspec(dllexport) Mesh(const std::string& fileName, bool requireTangents = false);
    ~Mesh();


    // How many nodes are in this mesh (seperate movable parts)
    unsigned int NumberNodes()  { return static_cast<unsigned int>(mNodes.size()); }

    // The default matrix for a given node - used to set the initial position for a new model
    CMatrix4x4 GetNodeDefaultMatrix(unsigned int node) { return mNodes[node].defaultMatrix; }


    // Render a given node in the mesh. Recursive function.
    // - modelMatrices are sent from the Model - one matrix for each node in the mesh, representing it's current "pose". Matrices are relative to the parent node.
    // - nodeIndex is the index of the current node, for accessing the vectors of matrices and node information
    // - parentWorldMatrix is the world matrix that was calculated for the parent in a previous call, it's used to make relative matrices into absolute matrices
    void RenderRecursive(std::vector<CMatrix4x4>& modelMatrices, unsigned int nodeIndex = 0, CMatrix4x4 parentWorldMatrix = MatrixIdentity());

    // Render all the nodes in the mesh without recursion, faster alternative to above (lab exercise)
    void Render(std::vector<CMatrix4x4>& modelMatrices);


//--------------------------------------------------------------------------------------
// Helper functions
//--------------------------------------------------------------------------------------
private:

    // Count the number of nodes with given assimp node as root
    unsigned int CountNodes(aiNode* assimpNode);

    // Help build the arrays of submeshes and nodes from the assimp data - recursive
    unsigned int ReadNodes(aiNode* assimpNode,unsigned int nodeIndex, unsigned int parentIndex);

    // Helper function for Render function - sends the world matrix for the next object to render over to the GPU
    void SetWorldMatrixOnGPU(CMatrix4x4 worldMatrix);

    // Helper function for Render function - renders all the submeshes of the given node. World matrix must already be set
    void RenderNodeSubMeshes(unsigned int nodeIndex);



//--------------------------------------------------------------------------------------
// Private data structures
//--------------------------------------------------------------------------------------
private:

    // A mesh is made of multiple sub-meshes. Each one uses a single material (texture).
    // Each sub-mesh has a vertex / index buffer on the GPU. Could share buffers for performance but that would be complex.
    struct SubMesh
    {
        unsigned int       vertexSize = 0;         // Size in bytes of a single vertex (depends on what it contains, uvs, tangents etc.)
        ID3D11InputLayout* vertexLayout = nullptr; // DirectX specification of data held in a single vertex

        // GPU-side vertex and index buffers
        unsigned int       numVertices = 0;
        ID3D11Buffer*      vertexBuffer = nullptr;

        unsigned int       numIndices = 0;
        ID3D11Buffer*      indexBuffer  = nullptr;
    };


    // A mesh contains a hierarchy of nodes. A node represents a seperate animatable part of the mesh
    // A node can contain several sub-meshes (because a single node might use multiple textures)
    // A node can also have child nodes. The children will follow the motion of the parent node
    // Each node has a default matrix which is it's initial/ default position. Models using this mesh are
    // given these default matrices as a starting position.
    struct Node
    {
        CMatrix4x4                defaultMatrix;  // Starting position/rotation/scale for this node. Relative to parent. Used when first creating a model from this mesh

        CMatrix4x4                absoluteMatrix; // Absolute matrix for this node for the model currently being rendered. Not persistent data, used in Render function only
        unsigned int              parentIndex;    // Index of the parent node (from the mNodes vector below). Root node refers to itself (0)

        std::vector<unsigned int> childNodes;     // Child nodes that are controlled by this node (indexes into the mNodes vector below)
        std::vector<unsigned int> subMeshes;      // The geometry representing this node (indexes into the mSubMeshes vector below)
    };


//--------------------------------------------------------------------------------------
// Member data
//--------------------------------------------------------------------------------------
private:

    std::vector<SubMesh> mSubMeshes; // The mesh geometry. Nodes refer to sub-meshes in this vector
    std::vector<Node>    mNodes;     // The mesh hierarchy. First entry is root. remainder aree stored in depth-first order
};


#endif //_MESH_H_INCLUDED_

