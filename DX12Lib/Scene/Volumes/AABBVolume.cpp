#include "stdafx.h"

#include "AABBVolume.h"

#include "Scene/Mesh.h"

namespace
{
    const DirectX::XMFLOAT3 BOX_MIN = DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f);
    const DirectX::XMFLOAT3 BOX_MAX = DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f);
    const DirectX::XMFLOAT3 BOX_VERTS[8] =
    {
        DirectX::XMFLOAT3(BOX_MAX.x, BOX_MAX.y, BOX_MIN.z),
        DirectX::XMFLOAT3(BOX_MAX.x, BOX_MIN.y, BOX_MIN.z),
        DirectX::XMFLOAT3(BOX_MAX),
        DirectX::XMFLOAT3(BOX_MAX.x, BOX_MIN.y, BOX_MAX.z),
        DirectX::XMFLOAT3(BOX_MIN.x, BOX_MAX.y, BOX_MIN.z),
        DirectX::XMFLOAT3(BOX_MIN),
        DirectX::XMFLOAT3(BOX_MIN.x, BOX_MAX.y, BOX_MAX.z),
        DirectX::XMFLOAT3(BOX_MIN.x, BOX_MIN.y, BOX_MAX.z)
    };

    const std::vector<UINT> BOX_INDICES =
    {
        4, 2, 0,
        2, 7, 3,
        6, 5, 7,
        1, 7, 5,
        0, 3, 1,
        4, 1, 5,
        4, 6, 2,
        2, 6, 7,
        6, 4, 5,
        1, 3, 7,
        0, 2, 3,
        4, 0, 1
    };
}

AABBVolume::AABBVolume(DirectX::XMVECTOR min, DirectX::XMVECTOR max)
{
    this->min = min;
    this->max = max;

    {
        mesh = std::make_shared<Mesh>();

        std::vector<VertexData> vertices(8);
        vertices[0].Position = { DirectX::XMVectorGetX(max), DirectX::XMVectorGetY(max), DirectX::XMVectorGetZ(min) };
        vertices[1].Position = { DirectX::XMVectorGetX(max), DirectX::XMVectorGetY(min), DirectX::XMVectorGetZ(min) };
        vertices[2].Position = { DirectX::XMVectorGetX(max), DirectX::XMVectorGetY(max), DirectX::XMVectorGetZ(max) };
        vertices[3].Position = { DirectX::XMVectorGetX(max), DirectX::XMVectorGetY(min), DirectX::XMVectorGetZ(max) };
        vertices[4].Position = { DirectX::XMVectorGetX(min), DirectX::XMVectorGetY(max), DirectX::XMVectorGetZ(min) };
        vertices[5].Position = { DirectX::XMVectorGetX(min), DirectX::XMVectorGetY(min), DirectX::XMVectorGetZ(min) };
        vertices[6].Position = { DirectX::XMVectorGetX(min), DirectX::XMVectorGetY(max), DirectX::XMVectorGetZ(max) };
        vertices[7].Position = { DirectX::XMVectorGetX(min), DirectX::XMVectorGetY(min), DirectX::XMVectorGetZ(max) };

        vertices[0].Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        vertices[1].Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        vertices[2].Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        vertices[3].Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        vertices[4].Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        vertices[5].Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        vertices[6].Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        vertices[7].Color = { 1.0f, 1.0f, 1.0f, 1.0f };

        mesh->SetVertices(vertices);
        mesh->SetIndices(BOX_INDICES);
    }
}
