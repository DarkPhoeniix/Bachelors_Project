#pragma once

#include "Node.h"

class Scene
{
public:
    Scene();
    ~Scene();

    std::string GetName() const;
    std::shared_ptr<Node> GetRootNodes() const;

    bool Parse(const std::vector<FbxScene*>& fbxScene);

    bool Save(const std::string& path) const;

private:
    std::string _name;
    std::shared_ptr<Node> _root;
};
