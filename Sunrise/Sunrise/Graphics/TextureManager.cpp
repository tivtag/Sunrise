#include "TextureManager.hpp"

sr::TextureManager::TextureManager(sr::FileSystem& _fileSystem)
    : fileSystem(_fileSystem)
{
#ifdef FREEIMAGE_LIB
	FreeImage_Initialise();
#endif
}

sr::TextureManager::~TextureManager()
{
#ifdef FREEIMAGE_LIB
	FreeImage_DeInitialise();
#endif

	//UnloadAllTextures();
}

sr::TexturePtr sr::TextureManager::LoadTexture(const std::string& relativePath)
{
    sr::TexturePtr texture = std::make_shared<sr::Texture>();	
    texture->LoadFromFile(relativePath, fileSystem);

    return texture;
}
