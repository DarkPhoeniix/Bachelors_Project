#pragma once

struct LOD
{
    std::vector<DirectX::XMVECTOR> vertices = {};
    std::vector<DirectX::XMVECTOR> normals = {};
    std::vector<DirectX::XMVECTOR> colors = {};
    std::vector<DirectX::XMFLOAT2> UVs = {};
    std::vector<UINT64> indices = {};
};

class Node
{
public:
    std::string GetName() const;

    const std::vector<std::shared_ptr<Node>>& GetChildren() const;

    DirectX::XMMATRIX GetTransform() const;
    std::pair<DirectX::XMVECTOR, DirectX::XMVECTOR> GetAABB() const;

    const std::vector<DirectX::XMVECTOR>& GetVertices(int lod) const;
    const std::vector<DirectX::XMVECTOR>& GetNormals(int lod) const;
    const std::vector<DirectX::XMVECTOR>& GetColors(int lod) const;
    const std::vector<DirectX::XMFLOAT2>& GetUVs(int lod) const;
    const std::vector<UINT64>& GetIndices(int lod) const;

    std::string GetTextureName() const;

    bool Parse(FbxNode* fbxNode);
    bool Parse(std::vector<FbxNode*> fbxLODs);

    bool Save(const std::string& path) const;

private:
    bool ParseMesh(FbxMesh* fbxMesh, int lod);

    bool SaveChildren(const std::string& path) const;
    bool SaveMesh(const std::string& path, int lod) const;
    bool SaveMaterial(const std::string& path) const;

    std::string _name;

    DirectX::XMMATRIX _transform;

    std::vector<std::shared_ptr<Node>> _children;

    std::pair<DirectX::XMVECTOR, DirectX::XMVECTOR> _aabb;
    std::vector<LOD> _lods;
    std::string _textureName;
    bool _isOccluder;
};
