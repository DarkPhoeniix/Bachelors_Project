#include "stdafx.h"

#include "OcclusionQuery.h"

#include "Scene/Volumes/FrustumVolume.h"
#include "Scene/Volumes/AABBVolume.h"

namespace Core
{
    OcclusionQuery::OcclusionQuery()
        : _DXDevice(Device::GetDXDevice())
        , _queryHeap(nullptr)
    {
    }

    OcclusionQuery::~OcclusionQuery()
    {
        _DXDevice = nullptr;
        _queryHeap = nullptr;
    }

    void OcclusionQuery::Create(int numObjects)
    {
        // Describe and create a heap for occlusion queries.
        D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
        queryHeapDesc.Count = numObjects;
        queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
        _DXDevice->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&_queryHeap));

        for (int i = 0; i < numObjects; ++i)
        {
            ResourceDescription desc(CD3DX12_RESOURCE_DESC::Buffer(8));
            _queryResults.emplace_back(desc);
            _queryResults.back().CreateCommitedResource(D3D12_RESOURCE_STATE_PREDICATION);
            _queryResults.back().SetName("Query Result " + std::to_string(i));
        }
    }

    void OcclusionQuery::Run(const ISceneNode* node, GraphicsCommandList& commandList, const FrustumVolume& frustum)
    {
        int index = -1;
        for (int i = 0; i < _queryResources.size(); ++i)
        {
            if (_queryResources[i] == node)
            {
                index = i;
            }
        }

        if (index == -1)
        {
            _queryResources.push_back(node);
            index = _queryResources.size() - 1;
        }

        commandList.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

        const AABBVolume& aabb = node->GetAABB();
        commandList.SetConstants(0, 3, &aabb.min.m128_f32, 0);
        commandList.SetConstants(0, 3, &aabb.max.m128_f32, 4);

        commandList.BeginQuery(_queryHeap, D3D12_QUERY_TYPE_BINARY_OCCLUSION, index);
        commandList.Draw(1);
        commandList.EndQuery(_queryHeap, D3D12_QUERY_TYPE_BINARY_OCCLUSION, index);

        commandList.TransitionBarrier(_queryResults[index], D3D12_RESOURCE_STATE_COPY_DEST);
        commandList.ResolveQueryData(_queryHeap, D3D12_QUERY_TYPE_BINARY_OCCLUSION, index, _queryResults[index], 0);
        commandList.TransitionBarrier(_queryResults[index], D3D12_RESOURCE_STATE_PREDICATION);
    }

    void OcclusionQuery::SetPredication(const ISceneNode* node, GraphicsCommandList& commandList)
    {
        for (int i = 0; i < _queryResources.size(); ++i)
        {
            if (_queryResources[i] == node)
            {
                commandList.SetPredication(&_queryResults[i], 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
                return;
            }
        }
        commandList.SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
    }
}
