#pragma once

#include "Scene/Volumes/IVolume.h"

class Mesh;

class AABBVolume : public IVolume
{
public:
    AABBVolume() = default;
    AABBVolume(DirectX::XMVECTOR min, DirectX::XMVECTOR max);
    ~AABBVolume() = default;

    DirectX::XMVECTOR min;
    DirectX::XMVECTOR max;

    std::shared_ptr<Mesh> mesh;
};
