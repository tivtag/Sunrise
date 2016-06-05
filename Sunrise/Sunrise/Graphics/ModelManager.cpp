#include "ModelManager.hpp"

#include <fstream>
#include "TextureManager.hpp"
#include "../FileSystem.hpp"

sr::ModelManager::ModelManager(sr::TextureManager& _textureManager, sr::FileSystem& _fileSystem)
    : textureManager(_textureManager), fileSystem(_fileSystem)
{
}

bool sr::ModelManager::HasInfo(const std::string& modelName) const
{
    return infos.find(modelName) != infos.end();
}

sr::ModelInfoPtr sr::ModelManager::LoadInfo(const std::string& modelName)
{
    auto itr = infos.find(modelName);

    if(itr == infos.end())
    {
        std::cout << "Loading ModelInfo " << modelName << std::endl;

        const std::string modelFileName = "data\\model\\" + modelName;
        const std::string modelFilePath = fileSystem.GetFullPath(modelFileName);

        std::ifstream modelStream;
	    modelStream.open(modelFilePath, std::ios::in | std::ios::binary);

        if(!modelStream.is_open())
        {
            std::cerr << "Error opening ModelInfo " << modelFilePath << std::endl;
            return nullptr;
        }
        
        sr::ModelInfoPtr modelInfo = std::make_shared<sr::ModelInfo>();
        if(!modelInfo->ReadStream(modelStream))
        {
            std::cerr << "Error reading ModelInfo " << modelFilePath << std::endl;
            return nullptr;
        }

        infos.insert(std::make_pair(modelName, modelInfo));
        return modelInfo;
    }
    else
    {
        return itr->second;
    }
}

bool sr::ModelManager::HasModel(const std::string& modelName) const
{
    return models.find(modelName) != models.end();
}

sr::ModelPtr sr::ModelManager::Load(const std::string& modelName)
{
    sr::ModelInfoPtr modelInfo = LoadInfo(modelName); 

    if(modelInfo)
    {
        return std::make_shared<sr::Model>(modelInfo, textureManager);
    }
    else
    {
        return nullptr;
    }
}

