#pragma once

#include <map>
#include <boost\noncopyable.hpp>
#include "Texture.hpp"

namespace sr
{
    class TextureManager : public boost::noncopyable
    {
    public:
	    TextureManager(sr::FileSystem&);
	    virtual ~TextureManager();

	    sr::TexturePtr LoadTexture(const std::string& relativePath);
        
	    //free all texture memory
	    //void UnloadAllTextures();
    private:
        sr::FileSystem& fileSystem;
    };
}
