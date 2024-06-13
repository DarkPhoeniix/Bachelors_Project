#pragma once

#include "Scene.h"

class FBXParser
{
public:
    FBXParser();
    ~FBXParser();

    int Run(int argc, char* argv[]);

private:
    int Parse(const std::vector<std::string>& filepath);
    bool ImportFbxScene(const std::vector<std::string>& filepath);
    bool ParseFbxScene();

    int Save();

    Scene _scene;

    std::vector<FbxScene*> _fbxLODScenes;
    FbxManager* _fbxManager;
};
