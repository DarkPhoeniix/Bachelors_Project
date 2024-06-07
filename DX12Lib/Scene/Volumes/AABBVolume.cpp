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
        for (size_t i = 0 ; i < 8; ++i)
        {
            vertices[i].Position = BOX_VERTS[i];
        }
        mesh->SetVertices(vertices);
        mesh->SetIndices(BOX_INDICES);
    }
}
