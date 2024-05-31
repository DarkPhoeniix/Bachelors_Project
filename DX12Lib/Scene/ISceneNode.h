#pragma once

#include "Volumes/AABBVolume.h"
#include "Volumes/FrustumVolume.h"

class Camera;
class Scene;
class FrustumVolume;

namespace Core
{
    class GraphicsCommandList;
} // namespace Core

class ISceneNode
{
public:
    ISceneNode();
    ISceneNode(Scene* scene, ISceneNode* parent = nullptr);
    virtual ~ISceneNode();

    DirectX::XMMATRIX GetLocalTransform() const;
    void SetLocalTransform(const DirectX::XMMATRIX& transform);

    DirectX::XMMATRIX GetGlobalTransform() const;

    virtual void RunOcclusion(Core::GraphicsCommandList& commandList, const FrustumVolume& frustum) const = 0;
    virtual void Draw(Core::GraphicsCommandList& commandList, const Camera& camera) const = 0;
    virtual void DrawOccluders(Core::GraphicsCommandList& commandList, const Camera& camera) const = 0;
    virtual void DrawOccludees(Core::GraphicsCommandList& commandList, const Camera& camera) const = 0;
    virtual void DrawAABB(Core::GraphicsCommandList& commandList) const = 0;

    virtual const AABBVolume& GetAABB() const = 0;
    virtual bool IsOccluder() const = 0;

    virtual void LoadNode(const std::string& filepath, Core::GraphicsCommandList& commandList) = 0;

protected:
    friend Scene;

    std::string _name;

    Scene* _scene;
    ISceneNode* _parent;
    std::vector<std::shared_ptr<ISceneNode>> _childNodes;

    DirectX::XMMATRIX _transform;
};
