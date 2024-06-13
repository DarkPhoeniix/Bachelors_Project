#include "pch.h"

#include "FBXParser.h"

using namespace DirectX;

namespace
{
    constexpr char SCENE_EXT[] = ".scene";
    constexpr char MESH_EXT[] = ".mesh";
    constexpr char MATERIAL_EXT[] = ".mat";
}

FBXParser::FBXParser()
    : _scene{}
    , _fbxLODScenes{}
{
    _fbxManager = FbxManager::Create();

    // create an IOSettings object
    FbxIOSettings* ios = FbxIOSettings::Create(_fbxManager, IOSROOT);
    _fbxManager->SetIOSettings(ios);
}

FBXParser::~FBXParser()
{
    if (_fbxManager)
    {
        _fbxManager->Destroy();
    }
}

int FBXParser::Run(int argc, char* argv[])
{
    if (argc <= 1)
    {
        return 1;
    }

    std::vector<std::string> filepath;
    for (int i = 1; i < argc; ++i)
    {
        filepath.push_back(argv[i]);
    }

    int parseResult = Parse(filepath);
    if (parseResult != 0)
    {
        return parseResult;
    }

    Save();

    return parseResult;
}

int FBXParser::Parse(const std::vector<std::string>& filepath)
{
    if (!ImportFbxScene(filepath))
    {
        return 3;
    }

    if (!ParseFbxScene())
    {
        return 4;
    }

    return 0;
}

bool FBXParser::ImportFbxScene(const std::vector<std::string>& filepath)
{
    bool lStatus = false;

    //create and init the scene
    FbxScene* fbxScene = FbxScene::Create(_fbxManager, "");

    // Create an importer.
    FbxImporter* lImporter = FbxImporter::Create(_fbxManager, "");

    for (int sceneIndex = 0; sceneIndex < filepath.size(); ++sceneIndex)
    {
        // Initialize the importer by providing a filename.
        bool lImportStatus = lImporter->Initialize(filepath[sceneIndex].c_str(), -1, _fbxManager->GetIOSettings());
        //lImporter->SetEmbeddingExtractionFolder("Res");

        if (!lImportStatus)
        {
            // Destroy the importer
            lImporter->Destroy();
            return false;
        }

        if (lImporter->IsFBX())
        {
            // Set the import states. By default, the import states are always set to 
            // true. The code below shows how to change these states.
            (*(_fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_MATERIAL, true);
            (*(_fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_TEXTURE, true);
            (*(_fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_LINK, true);
            (*(_fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_SHAPE, true);
            (*(_fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_GOBO, true);
            (*(_fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_ANIMATION, true);
            (*(_fbxManager->GetIOSettings())).SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
        }

        std::string sceneName = std::filesystem::path(filepath[sceneIndex]).stem().string();
        _fbxLODScenes.push_back(FbxScene::Create(this->_fbxManager, sceneName.c_str()));

        // Import the scene
        lStatus = lImporter->Import(_fbxLODScenes[sceneIndex]);

        _fbxLODScenes[sceneIndex]->SetName(sceneName.c_str());

        // Triangulate all nodes in the scene
        FbxGeometryConverter geometryConverter(_fbxManager);
        geometryConverter.Triangulate(_fbxLODScenes[sceneIndex], true);
    }

    // Destroy the importer
    lImporter->Destroy();

    return lStatus;
}

bool FBXParser::ParseFbxScene()
{
    return _scene.Parse(_fbxLODScenes);
}

int FBXParser::Save()
{
    std::string outDirectory = _scene.GetName() + '\\';
    std::filesystem::create_directory(_scene.GetName());
    _scene.Save(outDirectory);

    return 0;
}
