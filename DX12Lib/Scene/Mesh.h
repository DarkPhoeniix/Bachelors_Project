#pragma once

#include <fbxsdk.h>

struct VertexData
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT4 Color;
    DirectX::XMFLOAT2 UV;
};

class Mesh
{
public:
    Mesh() = default;
    ~Mesh() = default;

    void SetVertices(const std::vector<VertexData>& vertexData);
    const std::vector<VertexData>& GetVertices() const;

    void SetIndices(const std::vector<UINT>& indexData);
    const std::vector<UINT>& GetIndices() const;

    void LoadMesh(const std::string& filepath);

private:
    std::vector<VertexData> _rawVertexData;
    std::vector<UINT> _rawIndexData;
};
