#pragma once

#include "DXObjects/Texture.h"
#include "Scene/Mesh.h"
#include "Scene/ISceneNode.h"
#include "Scene/Volumes/AABBVolume.h"

#include <fbxsdk.h>

class Scene;

class SceneNode : public ISceneNode
{
public:
    SceneNode();
    SceneNode(Scene* scene, SceneNode* parent = nullptr);
    ~SceneNode();

    void RunOcclusion(Core::GraphicsCommandList& commandList, const FrustumVolume& frustum) const override;
    void Draw(Core::GraphicsCommandList& commandList, const Camera& camera) const override;
    void DrawOccluders(Core::GraphicsCommandList& commandList, const Camera& camera) const override;
    void DrawOccludees(Core::GraphicsCommandList& commandList, const Camera& camera) const override;
    void DrawAABB(Core::GraphicsCommandList& commandList) const override;
    void TestAABB(Core::GraphicsCommandList& commandList) const override;

    const AABBVolume& GetAABB() const override;
    bool IsOccluder() const override;

    void LoadNode(const std::string& filepath, Core::GraphicsCommandList& commandList) override;

protected:
    void _UploadData(Core::GraphicsCommandList& commandList,
                     ID3D12Resource** destinationResource,
                     size_t numElements,
                     size_t elementSize,
                     const void* bufferData,
                     D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
    void _DrawCurrentNode(Core::GraphicsCommandList& commandList, const Camera& camera) const;

private:
    ComPtr<ID3D12Device2> _DXDevice;

    std::shared_ptr<Mesh> _mesh;
    std::vector<std::shared_ptr<Mesh>> _LODs;
    AABBVolume _AABB;
    bool _isOccluder;

    std::shared_ptr<Core::Texture> _texture;

    std::shared_ptr<Core::Resource> _modelMatrix;
    std::vector<std::shared_ptr<Core::Resource>> _vertexBuffer;
    std::vector<std::shared_ptr<Core::Resource>> _indexBuffer;

    std::vector<D3D12_VERTEX_BUFFER_VIEW> _VBO;
    std::vector<D3D12_INDEX_BUFFER_VIEW>_IBO;

    std::shared_ptr<Core::Resource> _AABBVertexBuffer;
    std::shared_ptr<Core::Resource> _AABBIndexBuffer;

    D3D12_VERTEX_BUFFER_VIEW _AABBVBO;
    D3D12_INDEX_BUFFER_VIEW _AABBIBO;
    
    std::vector<ComPtr<ID3D12Resource>> intermediates;
};

