
#include "stdafx.h"

#include "SceneNode.h"

#include "DXObjects/Texture.h"
#include "DXObjects/GraphicsCommandList.h"
#include "Scene/Camera.h"
#include "Scene/Scene.h"
#include "Volumes/FrustumVolume.h"

using namespace DirectX;

namespace
{
    XMMATRIX GetNodeLocalTransform(FbxNode* fbxNode)
    {
        FbxAMatrix fbxTransform = fbxNode->EvaluateLocalTransform();
        XMMATRIX transform =
        {
            (float)fbxTransform.mData[0][0], (float)fbxTransform.mData[0][1], (float)fbxTransform.mData[0][2], (float)fbxTransform.mData[0][3],
            (float)fbxTransform.mData[1][0], (float)fbxTransform.mData[1][1], (float)fbxTransform.mData[1][2], (float)fbxTransform.mData[1][3],
            (float)fbxTransform.mData[2][0], (float)fbxTransform.mData[2][1], (float)fbxTransform.mData[2][2], (float)fbxTransform.mData[2][3],
            (float)fbxTransform.mData[3][0], (float)fbxTransform.mData[3][1], (float)fbxTransform.mData[3][2], (float)fbxTransform.mData[3][3],
        };

        return transform;
    }

    std::string GetDiffuseTextureName(FbxNode* fbxNode)
    {
        std::string name;

        if (FbxSurfaceMaterial* material = fbxNode->GetMaterial(0))
        {
            FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
            if (prop.GetSrcObjectCount<FbxFileTexture>() > 0)
            {
                if (FbxFileTexture* texture = prop.GetSrcObject<FbxFileTexture>(0))
                {
                    name = (const char*)(FbxPathUtils::GetFileName(texture->GetFileName()));
                }
            }
        }

        return name;
    }
}

SceneNode::SceneNode()
    : ISceneNode()
    , _DXDevice(Core::Device::GetDXDevice())
    , _mesh(nullptr)
    , _texture(nullptr)
    , _isOccluder(false)
    , _vertexBuffer{}
    , _indexBuffer{}
    , _modelMatrix(nullptr)
    , _AABBVertexBuffer(nullptr)
    , _AABBIndexBuffer(nullptr)
    , _AABB{}
    , _AABBVBO{}
    , _AABBIBO{}
    , _VBO{}
    , _IBO{}
{
}

SceneNode::SceneNode(Scene* scene, SceneNode* parent)
    : ISceneNode(scene, parent)
    , _DXDevice(Core::Device::GetDXDevice())
    , _mesh(nullptr)
    , _texture(nullptr)
    , _isOccluder(false)
    , _vertexBuffer{}
    , _indexBuffer{}
    , _modelMatrix(nullptr)
    , _AABBVertexBuffer(nullptr)
    , _AABBIndexBuffer(nullptr)
    , _AABB{}
    , _AABBVBO{}
    , _AABBIBO{}
    , _VBO{}
    , _IBO{}
{   }

SceneNode::~SceneNode()
{
    _DXDevice = nullptr;

    for (ComPtr<ID3D12Resource> intermediate : intermediates)
    {
        intermediate = nullptr;
    }
}

void SceneNode::RunOcclusion(Core::GraphicsCommandList& commandList, const FrustumVolume& frustum) const
{
    for (const std::shared_ptr<ISceneNode> node : _childNodes)
    {
        node->RunOcclusion(commandList, frustum);
    }

    if (!_isOccluder)
    {
        _scene->_occlusionQuery.Run(this, commandList, frustum);
    }
}

void SceneNode::Draw(Core::GraphicsCommandList& commandList, const Camera& camera) const
{
    for (const std::shared_ptr<ISceneNode> node : _childNodes)
    {
        node->Draw(commandList, camera);
    }

    _DrawCurrentNode(commandList, camera);
}

void SceneNode::DrawOccluders(Core::GraphicsCommandList& commandList, const Camera& camera) const
{
    for (const std::shared_ptr<ISceneNode> node : _childNodes)
    {
        node->DrawOccluders(commandList, camera);
    }

    if (_isOccluder)
    {
        _DrawCurrentNode(commandList, camera);
    }
}

void SceneNode::DrawOccludees(Core::GraphicsCommandList& commandList, const Camera& camera) const
{
    for (const std::shared_ptr<ISceneNode> node : _childNodes)
    {
        node->DrawOccludees(commandList, camera);
    }

    if (!_isOccluder)
    {
        _DrawCurrentNode(commandList, camera);
    }
}

void SceneNode::DrawAABB(Core::GraphicsCommandList& commandList) const
{
    for (const std::shared_ptr<ISceneNode> node : _childNodes)
    {
        node->DrawAABB(commandList);
    }

    if (_LODs.empty())
    {
        return;
    }

    commandList.SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);

    commandList.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

    commandList.SetConstants(0, 3, &_AABB.min.m128_f32, 0);
    commandList.SetConstants(0, 3, &_AABB.max.m128_f32, 4);

    commandList.Draw(1);
}

void SceneNode::TestAABB(Core::GraphicsCommandList& commandList) const
{
    XMMATRIX* modelMatrixData = (XMMATRIX*)_modelMatrix->Map();
    *modelMatrixData = GetGlobalTransform();
    commandList.SetSRV(3, _modelMatrix->OffsetGPU(0));

    commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList.SetVertexBuffer(0, _AABBVBO);
    commandList.SetIndexBuffer(_AABBIBO);

    commandList.DrawIndexed(_AABB.mesh->GetIndices().size());
}

const AABBVolume& SceneNode::GetAABB() const
{
    return _AABB;
}

bool SceneNode::IsOccluder() const
{
    return _isOccluder;
}

void SceneNode::LoadNode(const std::string& filepath, Core::GraphicsCommandList& commandList)
{
    Logger::Log(LogType::Info, "Parsing node " + filepath);

    std::ifstream in(filepath, std::ios_base::in | std::ios_base::binary);
    Json::Value root;
    in >> root;

    _name = root["Name"].asCString();

    _transform = XMMatrixSet(
        root["Transform"]["r0"]["x"].asFloat(), root["Transform"]["r0"]["y"].asFloat(), root["Transform"]["r0"]["z"].asFloat(), root["Transform"]["r0"]["w"].asFloat(),
        root["Transform"]["r1"]["x"].asFloat(), root["Transform"]["r1"]["y"].asFloat(), root["Transform"]["r1"]["z"].asFloat(), root["Transform"]["r1"]["w"].asFloat(),
        root["Transform"]["r2"]["x"].asFloat(), root["Transform"]["r2"]["y"].asFloat(), root["Transform"]["r2"]["z"].asFloat(), root["Transform"]["r2"]["w"].asFloat(),
        root["Transform"]["r3"]["x"].asFloat(), root["Transform"]["r3"]["y"].asFloat(), root["Transform"]["r3"]["z"].asFloat(), root["Transform"]["r3"]["w"].asFloat()
    );

    {
        DirectX::XMVECTOR min = XMVectorSet(root["AABB"]["Min"]["x"].asFloat(), root["AABB"]["Min"]["y"].asFloat(), root["AABB"]["Min"]["z"].asFloat(), root["AABB"]["Min"]["w"].asFloat());
        DirectX::XMVECTOR max = XMVectorSet(root["AABB"]["Max"]["x"].asFloat(), root["AABB"]["Max"]["y"].asFloat(), root["AABB"]["Max"]["z"].asFloat(), root["AABB"]["Max"]["w"].asFloat());
        
        _AABB = AABBVolume(min, max);

        ComPtr<ID3D12Resource> vertexBuffer;
        _UploadData(commandList, &vertexBuffer, _AABB.mesh->GetVertices().size(), sizeof(VertexData), _AABB.mesh->GetVertices().data());
        _AABBVertexBuffer = std::make_shared<Core::Resource>();
        _AABBVertexBuffer->InitFromDXResource(vertexBuffer);
        _AABBVertexBuffer->SetName(_name + "_AABB_VB");

        _AABBVBO = D3D12_VERTEX_BUFFER_VIEW();
        _AABBVBO.BufferLocation = _AABBVertexBuffer->OffsetGPU(0);
        _AABBVBO.SizeInBytes = static_cast<UINT>(_AABB.mesh->GetVertices().size() * sizeof(_AABB.mesh->GetVertices()[0]));
        _AABBVBO.StrideInBytes = sizeof(VertexData);

        ComPtr<ID3D12Resource> indexBuffer;
        _UploadData(commandList, &indexBuffer, _AABB.mesh->GetIndices().size(), sizeof(UINT), _AABB.mesh->GetIndices().data());
        _AABBIndexBuffer = std::make_shared<Core::Resource>();
        _AABBIndexBuffer->InitFromDXResource(indexBuffer);
        _AABBIndexBuffer->SetName(_name + "_AABB_IB");

        _AABBIBO = D3D12_INDEX_BUFFER_VIEW();
        _AABBIBO.BufferLocation = _AABBIndexBuffer->OffsetGPU(0);
        _AABBIBO.Format = DXGI_FORMAT_R32_UINT;
        _AABBIBO.SizeInBytes = static_cast<UINT>(_AABB.mesh->GetIndices().size() * sizeof(_AABB.mesh->GetIndices()[0]));
    }

    auto LODs = root["LODs"];
    if (!LODs.isNull())
    {
        for (int i = 0; i < LODs.size(); ++i)
        {
            _LODs.push_back(std::make_shared<Mesh>());
            _LODs.back()->LoadMesh(_scene->_name + '\\' + LODs[i].asCString());
        }
    }

    if (!root["Material"].isNull())
    {
        std::ifstream inMat(_scene->_name + '\\' + root["Material"].asCString(), std::ios_base::in | std::ios_base::binary);
        Json::Value mat;
        inMat >> mat;
        if (_texture = std::move(Core::Texture::LoadFromFile(mat["Diffuse"].asCString())))
        {
            _scene->_UploadTexture(_texture.get(), commandList);
        }
    }

    _isOccluder = root["IsOccluder"].asBool();

    auto children = root["Nodes"];
    for (int i = 0; i < children.size(); ++i)
    {
        std::shared_ptr<SceneNode> child = std::make_shared<SceneNode>(_scene, this);
        child->LoadNode(_scene->_name + '\\' + children[i].asCString(), commandList);
        _childNodes.push_back(child);
    }

    {
        Core::EResourceType SRVType = Core::EResourceType::Dynamic | Core::EResourceType::Buffer;

        Core::ResourceDescription desc;
        desc.SetResourceType(SRVType);
        desc.SetSize({ sizeof(XMMATRIX), 1 });
        desc.SetStride(1);
        desc.SetFormat(DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);

        _modelMatrix = std::make_shared<Core::Resource>(desc);
        _modelMatrix->CreateCommitedResource(D3D12_RESOURCE_STATE_GENERIC_READ);
        _modelMatrix->SetName(_name + "_ModelMatrix");
    }


    if (!_LODs.empty())
    {
        for (int i = 0; i < _LODs.size(); ++i)
        {
            ComPtr<ID3D12Resource> vertexBuffer;
            _UploadData(commandList, &vertexBuffer, _LODs[i]->GetVertices().size(), sizeof(VertexData), _LODs[i]->GetVertices().data());
            _vertexBuffer.push_back(std::make_shared<Core::Resource>());
            _vertexBuffer[i]->InitFromDXResource(vertexBuffer);
            _vertexBuffer[i]->SetName(_name + "_VB");

            _VBO.emplace_back(D3D12_VERTEX_BUFFER_VIEW());
            _VBO[i].BufferLocation = _vertexBuffer[i]->OffsetGPU(0);
            _VBO[i].SizeInBytes = static_cast<UINT>(_LODs[i]->GetVertices().size() * sizeof(_LODs[i]->GetVertices()[0]));
            _VBO[i].StrideInBytes = sizeof(VertexData);
        }

        for (int i = 0; i < _LODs.size(); ++i)
        {
            ComPtr<ID3D12Resource> indexBuffer;
            _UploadData(commandList, &indexBuffer, _LODs[i]->GetIndices().size(), sizeof(UINT), _LODs[i]->GetIndices().data());
            _indexBuffer.push_back(std::make_shared<Core::Resource>());
            _indexBuffer[i]->InitFromDXResource(indexBuffer);
            _indexBuffer[i]->SetName(_name + "_IB");

            _IBO.emplace_back(D3D12_INDEX_BUFFER_VIEW());
            _IBO[i].BufferLocation = _indexBuffer[i]->OffsetGPU(0);
            _IBO[i].Format = DXGI_FORMAT_R32_UINT;
            _IBO[i].SizeInBytes = static_cast<UINT>(_LODs[i]->GetIndices().size() * sizeof(_LODs[i]->GetIndices()[0]));
        }
    }
}

void SceneNode::_UploadData(Core::GraphicsCommandList& commandList,
    ID3D12Resource** destinationResource,
    size_t numElements,
    size_t elementSize,
    const void* bufferData,
    D3D12_RESOURCE_FLAGS flags)
{
    size_t bufferSize = numElements * elementSize;

    CD3DX12_HEAP_PROPERTIES heapTypeDefault(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC bufferWithFlags = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

    Helper::throwIfFailed(_DXDevice->CreateCommittedResource(
        &heapTypeDefault,
        D3D12_HEAP_FLAG_NONE,
        &bufferWithFlags,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(destinationResource)));

    CD3DX12_HEAP_PROPERTIES heapTypeUpload(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC buffer = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

    ComPtr<ID3D12Resource> intermediateResource;

    if (bufferData)
    {
        Helper::throwIfFailed(_DXDevice->CreateCommittedResource(
            &heapTypeUpload,
            D3D12_HEAP_FLAG_NONE,
            &buffer,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&intermediateResource)));
        intermediates.push_back(intermediateResource);

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = bufferSize;
        subresourceData.SlicePitch = subresourceData.RowPitch;

        UpdateSubresources(commandList.GetDXCommandList().Get(),
            *destinationResource, intermediateResource.Get(),
            0, 0, 1, &subresourceData);
    }
}

void SceneNode::_DrawCurrentNode(Core::GraphicsCommandList& commandList, const Camera& camera) const
{
    if (_LODs.empty())
    {
        return;
    }

    if (!Intersect(camera.GetViewFrustum(), _AABB))
    {
        return;
    }

    if (_texture)
    {
        commandList.SetDescriptorHeaps({ _scene->_texturesTable->GetDescriptorHeap().GetDXDescriptorHeap().Get() });

        commandList.SetConstant(1, true);
        commandList.SetDescriptorTable(4, _scene->_texturesTable->GetResourceGPUHandle(_texture->GetName()));
    }
    else
    {
        commandList.SetConstant(1, false);
    }

    _scene->_occlusionQuery.SetPredication(this, commandList);

    DirectX::XMVECTOR center = _AABB.min + ((_AABB.max - _AABB.min) * 0.5f);
    float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(camera.Position() - center));

    int lod = distance / 200.0f;
    int lodIndex = (lod >= _LODs.size()) ? (_LODs.size() - 1) : lod;

    XMMATRIX* modelMatrixData = (XMMATRIX*)_modelMatrix->Map();
    *modelMatrixData = GetGlobalTransform();
    commandList.SetSRV(3, _modelMatrix->OffsetGPU(0));

    commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList.SetVertexBuffer(0, _VBO[lodIndex]);
    commandList.SetIndexBuffer(_IBO[lodIndex]);

    commandList.DrawIndexed(_LODs[lodIndex]->GetIndices().size());
}
