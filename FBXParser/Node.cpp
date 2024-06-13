#include "pch.h"
#include "Node.h"

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

    void ReadPosition(FbxMesh* fbxMesh, int polygonIndex, int vertexIndex, XMVECTOR& outPosition)
    {
        FbxVector4 position = fbxMesh->GetControlPointAt(fbxMesh->GetPolygonVertex(polygonIndex, vertexIndex));
        outPosition = XMVectorSet((float)position.mData[0], (float)position.mData[1], (float)position.mData[2], (float)position.mData[3]);
    }

    void ReadNormal(FbxMesh* fbxMesh, int controlPointIndex, int vertexIndex, XMVECTOR& outNormal)
    {
        FbxGeometryElementNormal* vertexNormal = fbxMesh->GetElementNormal(0);
        XMFLOAT4 norm;
        switch (vertexNormal->GetMappingMode())
        {
        case FbxGeometryElement::eByControlPoint:
            switch (vertexNormal->GetReferenceMode())
            {
            case FbxGeometryElement::eDirect:
            {
                norm = {
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[0]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[1]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[2]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[3])
                };
            }
            break;

            case FbxGeometryElement::eIndexToDirect:
            {
                int index = vertexNormal->GetIndexArray().GetAt(controlPointIndex);

                norm = {
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[3])
                };
            }
            break;

            default:
                throw std::exception("Invalid Reference");
            }
            break;

        case FbxGeometryElement::eByPolygonVertex:
            switch (vertexNormal->GetReferenceMode())
            {
            case FbxGeometryElement::eDirect:
            {
                norm = {
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexIndex).mData[0]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexIndex).mData[1]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexIndex).mData[2]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexIndex).mData[3])
                };
            }
            break;

            case FbxGeometryElement::eIndexToDirect:
            {
                int index = vertexNormal->GetIndexArray().GetAt(vertexIndex);

                norm = {
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]),
                    static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[3])
                };
            }
            break;

            default:
                throw std::exception("Invalid Reference");
            }
            break;
        }

        outNormal = XMVectorSet(norm.x, norm.y, norm.z, norm.w);
    }

    void ReadColor(fbxsdk::FbxMesh* fbxMesh, int polygonIndex, int controlPointIndex, int vertexIndex, XMVECTOR& outColor)
    {
        outColor = { 0.5f, 0.5f, 0.5f, 1.0f };

        FbxSurfaceLambert* mat = nullptr;
        for (int l = 0; l < fbxMesh->GetElementMaterialCount(); l++)
        {
            FbxGeometryElementMaterial* leVtxc = fbxMesh->GetElementMaterial(l);

            auto mapM = leVtxc->GetMappingMode();
            switch (leVtxc->GetMappingMode())
            {
            default:
                break;
            case FbxGeometryElement::eByControlPoint:
                OutputDebugStringA("eByControlPoint\n");
                switch (leVtxc->GetReferenceMode())
                {
                case FbxGeometryElement::eDirect:
                {
                    //DisplayColor(header, leVtxc->GetDirectArray().GetAt(controlPointIndex));
                    //fbxColor = leVtxc->GetDirectArray().GetAt(controlPointIndex);
                    //std::string str = "Color: " + std::to_string(fbxColor.mRed) + ", " + std::to_string(fbxColor.mGreen) + ", " + std::to_string(fbxColor.mBlue) + "\n";
                    //OutputDebugStringA(str.c_str());
                }
                break;
                case FbxGeometryElement::eIndexToDirect:
                {
                    int id = leVtxc->GetIndexArray().GetAt(controlPointIndex);
                    //DisplayColor(header, leVtxc->GetDirectArray().GetAt(id));
                    //fbxColor = leVtxc->GetDirectArray().GetAt(id);
                    //std::string str = "Color: " + std::to_string(fbxColor.mRed) + ", " + std::to_string(fbxColor.mGreen) + ", " + std::to_string(fbxColor.mBlue) + "\n";
                    //OutputDebugStringA(str.c_str());
                }
                break;
                default:
                    break; // other reference modes not shown here!
                }
                break;

            case FbxGeometryElement::eByPolygonVertex:
            {
                OutputDebugStringA("eByPolygonVertex\n");
                switch (leVtxc->GetReferenceMode())
                {
                case FbxGeometryElement::eDirect:
                {
                    //DisplayColor(header, leVtxc->GetDirectArray().GetAt(vertexId));

                    //fbxColor = leVtxc->GetDirectArray().GetAt(vertexIndex);
                    //std::string str = "Color: " + std::to_string(fbxColor.mRed) + ", " + std::to_string(fbxColor.mGreen) + ", " + std::to_string(fbxColor.mBlue) + "\n";
                    //OutputDebugStringA(str.c_str());
                }
                break;
                case FbxGeometryElement::eIndexToDirect:
                {
                    int id = leVtxc->GetIndexArray().GetAt(vertexIndex);
                    //DisplayColor(header, leVtxc->GetDirectArray().GetAt(id));
                    //fbxColor = leVtxc->GetDirectArray().GetAt(id);
                    //std::string str = "Color: " + std::to_string(fbxColor.mRed) + ", " + std::to_string(fbxColor.mGreen) + ", " + std::to_string(fbxColor.mBlue) + "\n";
                    //OutputDebugStringA(str.c_str());
                }
                break;
                default:
                    break; // other reference modes not shown here!
                }
            }
            break;

            case FbxGeometryElement::eByPolygon:
            {
                mat = (FbxSurfaceLambert*)fbxMesh->GetNode()->GetMaterial(leVtxc->GetIndexArray().GetAt(polygonIndex));
                auto amb = mat->Ambient;
                //std::string str = "Color: " + std::to_string(mat->Diffuse.Get()[0]) + ", " + std::to_string(mat->Diffuse.Get()[1]) + ", " + std::to_string(mat->Diffuse.Get()[2]) + "\n";
                //OutputDebugStringA(str.c_str());
                //OutputDebugStringA("ByPolygon\n");
                break;
            }
            case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
            {
                mat = (FbxSurfaceLambert*)fbxMesh->GetNode()->GetMaterial(0);
                auto amb = mat->Ambient;
                //OutputDebugStringA("AllSame\n");
            }
            case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
                break;
            }
        }

        if (mat)
        {
            outColor = XMVectorSet(
                (float)mat->Diffuse.Get()[0],
                (float)mat->Diffuse.Get()[1],
                (float)mat->Diffuse.Get()[2],
                (float)mat->Diffuse.Get()[3]
            );
        }
    }

    void readUV(fbxsdk::FbxMesh* fbxMesh, int vertexIndex, int uvIndex, XMFLOAT2& outUV) {

        fbxsdk::FbxLayerElementUV* pFbxLayerElementUV = fbxMesh->GetLayer(0)->GetUVs();

        if (pFbxLayerElementUV == nullptr) {
            return;
        }

        switch (pFbxLayerElementUV->GetMappingMode()) {
        case FbxLayerElementUV::eByControlPoint:
        {
            switch (pFbxLayerElementUV->GetReferenceMode()) {
            case FbxLayerElementUV::eDirect:
            {
                fbxsdk::FbxVector2 fbxUv = pFbxLayerElementUV->GetDirectArray().GetAt(vertexIndex);

                outUV.x = fbxUv.mData[0];
                outUV.y = fbxUv.mData[1];

                break;
            }
            case FbxLayerElementUV::eIndexToDirect:
            {
                int id = pFbxLayerElementUV->GetIndexArray().GetAt(vertexIndex);
                fbxsdk::FbxVector2 fbxUv = pFbxLayerElementUV->GetDirectArray().GetAt(id);

                outUV.x = fbxUv.mData[0];
                outUV.y = fbxUv.mData[1];

                break;
            }
            }
            break;
        }
        case FbxLayerElementUV::eByPolygonVertex:
        {
            switch (pFbxLayerElementUV->GetReferenceMode()) {
                // Always enters this part for the example model
            case FbxLayerElementUV::eDirect:
            case FbxLayerElementUV::eIndexToDirect:
            {
                outUV.x = pFbxLayerElementUV->GetDirectArray().GetAt(uvIndex).mData[0];
                outUV.y = pFbxLayerElementUV->GetDirectArray().GetAt(uvIndex).mData[1];
                break;
            }
            }
            break;
        }
        }
    }
}

std::string Node::GetName() const
{
    return _name;
}

const std::vector<std::shared_ptr<Node>>& Node::GetChildren() const
{
    return _children;
}

DirectX::XMMATRIX Node::GetTransform() const
{
    return _transform;
}

std::pair<DirectX::XMVECTOR, DirectX::XMVECTOR> Node::GetAABB() const
{
    return _aabb;
}

const std::vector<DirectX::XMVECTOR>& Node::GetVertices(int lod) const
{
    return _lods[lod].vertices;
}

const std::vector<DirectX::XMVECTOR>& Node::GetNormals(int lod) const
{
    return _lods[lod].normals;
}

const std::vector<DirectX::XMVECTOR>& Node::GetColors(int lod) const
{
    return _lods[lod].colors;
}

const std::vector<DirectX::XMFLOAT2>& Node::GetUVs(int lod) const
{
    return _lods[lod].UVs;
}

const std::vector<UINT64>& Node::GetIndices(int lod) const
{
    return _lods[lod].indices;
}

std::string Node::GetTextureName() const
{
    return _textureName;
}

bool Node::Parse(FbxNode* fbxNode)
{
    _name = fbxNode->GetName();

    _transform = GetNodeLocalTransform(fbxNode);

    // Setup mesh
    if (FbxMesh* fbxMesh = fbxNode->GetMesh())
    {
        FbxVector4 min, max, center;

        fbxNode->EvaluateGlobalBoundingBoxMinMaxCenter(min, max, center);
        _aabb.first = XMVectorSet(min.mData[0], min.mData[1], min.mData[2], min.mData[3]);
        _aabb.second = XMVectorSet(max.mData[0], max.mData[1], max.mData[2], max.mData[3]);

        ParseMesh(fbxMesh, 0);

        _textureName = GetDiffuseTextureName(fbxNode);
    }

    // Setup child nodes
    for (int childIndex = 0; childIndex < fbxNode->GetChildCount(); ++childIndex)
    {
        auto childNode = std::make_shared<Node>();
        childNode->Parse(fbxNode->GetChild(childIndex));
        _children.push_back(childNode);
    }

    return true;
}

bool Node::Parse(std::vector<FbxNode*> fbxLODs)
{
    if (fbxLODs.empty())
    {
        return false;
    }

    _lods.resize(fbxLODs.size());

    _name = fbxLODs[0]->GetName();

    _transform = GetNodeLocalTransform(fbxLODs[0]);

    FbxProperty prop = fbxLODs[0]->FindProperty("is_occluder");
    if (prop.IsValid())
    {
        _isOccluder = prop.EvaluateValue<bool>();
    }

    for (int i = 0; i < fbxLODs.size(); ++i)
    {
        // Setup mesh
        if (FbxMesh* fbxMesh = fbxLODs[i]->GetMesh())
        {
            FbxVector4 min, max, center;

            fbxLODs[i]->EvaluateGlobalBoundingBoxMinMaxCenter(min, max, center);
            _aabb.first = XMVectorSet(min.mData[0], min.mData[1], min.mData[2], min.mData[3]);
            _aabb.second = XMVectorSet(max.mData[0], max.mData[1], max.mData[2], max.mData[3]);

            ParseMesh(fbxMesh, i);

            _textureName = GetDiffuseTextureName(fbxLODs[i]);
        }
    }

    // Setup child nodes
    for (int childIndex = 0; childIndex < fbxLODs[0]->GetChildCount(); ++childIndex)
    {
        FbxNode* child = fbxLODs[0]->GetChild(childIndex);

        std::vector<FbxNode*> children;
        for (int i = 0; i < fbxLODs.size(); ++i)
        {
            children.push_back(fbxLODs[i]->FindChild(child->GetName()));
        }

        auto childNode = std::make_shared<Node>();
        childNode->Parse(children);
        _children.push_back(childNode);
    }

    return true;
}

bool Node::Save(const std::string& path) const
{
    std::string rootPath = path + _name + ".node";
    std::ofstream out(rootPath, std::fstream::out | std::ios_base::binary);

    Json::StreamWriterBuilder builder;
    const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

    Json::Value jsonRoot;
    jsonRoot["Name"] = _name.c_str();

    Json::Value lods(Json::arrayValue);
    for (int lod = 0; lod < _lods.size(); ++lod)
    {
        if (_lods[lod].vertices.empty())
        {
            continue;
        }
        // Save mesh data
        const std::string lodName = "LOD" + std::to_string(lod);
        std::string meshFilepath = (_name + '_' + lodName + ".mesh").c_str();
        SaveMesh(path + meshFilepath, lod);
        lods.append(meshFilepath.c_str());

        // Save material data
        std::string materialFilepath = (_name + ".mat").c_str();
        jsonRoot["Material"] = materialFilepath.c_str();
        SaveMaterial(path + materialFilepath);
    }
    if (!lods.empty())
    {
        jsonRoot["LODs"] = lods;
    }

    Json::Value jsonTransform;
    jsonTransform["r0"]["x"] = XMVectorGetX(_transform.r[0]);
    jsonTransform["r0"]["y"] = XMVectorGetY(_transform.r[0]);
    jsonTransform["r0"]["z"] = XMVectorGetZ(_transform.r[0]);
    jsonTransform["r0"]["w"] = XMVectorGetW(_transform.r[0]);

    jsonTransform["r1"]["x"] = XMVectorGetX(_transform.r[1]);
    jsonTransform["r1"]["y"] = XMVectorGetY(_transform.r[1]);
    jsonTransform["r1"]["z"] = XMVectorGetZ(_transform.r[1]);
    jsonTransform["r1"]["w"] = XMVectorGetW(_transform.r[1]);

    jsonTransform["r2"]["x"] = XMVectorGetX(_transform.r[2]);
    jsonTransform["r2"]["y"] = XMVectorGetY(_transform.r[2]);
    jsonTransform["r2"]["z"] = XMVectorGetZ(_transform.r[2]);
    jsonTransform["r2"]["w"] = XMVectorGetW(_transform.r[2]);

    jsonTransform["r3"]["x"] = XMVectorGetX(_transform.r[3]);
    jsonTransform["r3"]["y"] = XMVectorGetY(_transform.r[3]);
    jsonTransform["r3"]["z"] = XMVectorGetZ(_transform.r[3]);
    jsonTransform["r3"]["w"] = XMVectorGetW(_transform.r[3]);
    jsonRoot["Transform"] = jsonTransform;

    // Save AABB data
    Json::Value jsonAABB;
    jsonAABB["Min"]["x"] = XMVectorGetX(_aabb.first);
    jsonAABB["Min"]["y"] = XMVectorGetY(_aabb.first);
    jsonAABB["Min"]["z"] = XMVectorGetZ(_aabb.first);
    jsonAABB["Min"]["w"] = XMVectorGetW(_aabb.first);
    jsonAABB["Max"]["x"] = XMVectorGetX(_aabb.second);
    jsonAABB["Max"]["y"] = XMVectorGetY(_aabb.second);
    jsonAABB["Max"]["z"] = XMVectorGetZ(_aabb.second);
    jsonAABB["Max"]["w"] = XMVectorGetW(_aabb.second);
    jsonRoot["AABB"] = jsonAABB;

    jsonRoot["IsOccluder"] = _isOccluder;

    Json::Value nodes(Json::arrayValue);
    for (const auto& node : _children)
    {
        node->Save(path);
        nodes.append((node->GetName() + ".node").c_str());
    }
    jsonRoot["Nodes"] = nodes;

    writer->write(jsonRoot, &out);

    return false;
}

bool Node::SaveChildren(const std::string& path) const
{
    return false;
}

bool Node::SaveMesh(const std::string& path, int lod) const
{
    std::ofstream out(path, std::fstream::out);

    for (const auto& vertex : _lods[lod].vertices)
    {
        out << "v " << XMVectorGetX(vertex) << ' ' << XMVectorGetY(vertex) << ' ' << XMVectorGetZ(vertex) << ' ' << XMVectorGetW(vertex) << '\n';
    }

    for (const auto& normal : _lods[lod].normals)
    {
        out << "vn " << XMVectorGetX(normal) << ' ' << XMVectorGetY(normal) << ' ' << XMVectorGetZ(normal) << ' ' << XMVectorGetW(normal) << '\n';
    }

    for (const auto& color : _lods[lod].colors)
    {
        out << "vc " << XMVectorGetX(color) << ' ' << XMVectorGetY(color) << ' ' << XMVectorGetZ(color) << ' ' << 1.0f << '\n';
    }

    for (const auto& uv : _lods[lod].UVs)
    {
        out << "vt " << uv.x << ' ' << uv.y << " 0" << '\n';
    }

    for (int i = 0; i < _lods[lod].indices.size(); i += 3)
    {
        out << "f " <<
            _lods[lod].indices[i]     << '/' << _lods[lod].indices[i]     << '/' << _lods[lod].indices[i] << ' ' <<
            _lods[lod].indices[i + 1] << '/' << _lods[lod].indices[i + 1] << '/' << _lods[lod].indices[i + 1] << ' ' <<
            _lods[lod].indices[i + 2] << '/' << _lods[lod].indices[i + 2] << '/' << _lods[lod].indices[i + 2] << '\n';
    }

    return true;
}

bool Node::SaveMaterial(const std::string& path) const
{
    std::ofstream out(path, std::fstream::out | std::ios_base::binary);

    Json::Value jsonMaterial;
    jsonMaterial["Diffuse"] = _textureName.c_str();

    Json::StreamWriterBuilder builder;
    const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(jsonMaterial, &out);

    return true;
}

bool Node::ParseMesh(FbxMesh* fbxMesh, int lod)
{
    for (int polygonIndex = 0; polygonIndex < fbxMesh->GetPolygonCount(); ++polygonIndex)
    {
        for (int vertexIndex = 0; vertexIndex < fbxMesh->GetPolygonSize(polygonIndex); ++vertexIndex)
        {
            int vertexAbsoluteIndex = polygonIndex * 3 + vertexIndex;

            XMVECTOR position;
            XMVECTOR normal;
            XMVECTOR color;
            XMFLOAT2 uv;

            ReadPosition(fbxMesh, polygonIndex, vertexIndex, position);
            ReadNormal(fbxMesh, fbxMesh->GetPolygonVertex(polygonIndex, vertexIndex), vertexAbsoluteIndex, normal);
            ReadColor(fbxMesh, polygonIndex, fbxMesh->GetPolygonVertex(polygonIndex, vertexIndex), vertexAbsoluteIndex, color);
            readUV(fbxMesh, vertexIndex, fbxMesh->GetTextureUVIndex(polygonIndex, vertexIndex), uv);

            _lods[lod].vertices.push_back(position);
            _lods[lod].normals.push_back(XMVector3Normalize(normal));
            _lods[lod].colors.push_back(color);
            _lods[lod].UVs.push_back(uv);
            _lods[lod].indices.push_back(vertexAbsoluteIndex);
        }
    }

    return false;
}
