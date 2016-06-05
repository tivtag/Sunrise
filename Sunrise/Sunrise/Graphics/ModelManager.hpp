#pragma once

#include <map>
#include <string>
#include "ModelInfo.hpp"
#include "Model.hpp"

namespace sr
{
    class FileSystem;
    class TextureManager;

	/*
        Responsible for loading models from disc.
	*/
    class ModelManager
    {
    public:
        ModelManager(TextureManager&, FileSystem&);

        bool HasInfo(const std::string&) const;
        sr::ModelInfoPtr LoadInfo(const std::string&);
                
        bool HasModel(const std::string&) const;
        sr::ModelPtr Load(const std::string&);

    private:
        TextureManager& textureManager;
        FileSystem& fileSystem;

        std::map<std::string, sr::ModelInfoPtr> infos;
        std::map<std::string, sr::ModelPtr> models;
    };
}
