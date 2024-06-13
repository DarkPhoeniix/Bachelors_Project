#include "pch.h"
#include "Scene.h"

Scene::Scene()
    : _root{}
{   }

Scene::~Scene()
{   }

std::string Scene::GetName() const
{
    return _name;
}

std::shared_ptr<Node> Scene::GetRootNodes() const
{
    return _root;
}

bool Scene::Parse(const std::vector<FbxScene*>& fbxScenes)
{
    _name = fbxScenes[0]->GetName();

    std::vector<FbxNode*> fbxLODs;
    for (int i = 0; i < fbxScenes.size(); ++i)
    {
        fbxLODs.push_back(fbxScenes[i]->GetRootNode());
    }
    _root = std::make_shared<Node>();

    return _root->Parse(fbxLODs);
}

bool Scene::Save(const std::string& path) const
{
    std::string rootPath = path + _name + ".scene";
    std::ofstream out(rootPath, std::fstream::out | std::ios_base::binary);

    Json::StreamWriterBuilder builder;
    const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

    Json::Value jsonRoot;
    Json::Value nodes(Json::arrayValue);

    jsonRoot["Name"] = _name.c_str();
    for (const auto& node : _root->GetChildren())
    {
        node->Save(path);

        nodes.append((node->GetName() + ".node").c_str());
    }
    jsonRoot["Nodes"] = nodes;

    writer->write(jsonRoot, &out);

    return false;
}
