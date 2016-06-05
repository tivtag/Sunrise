#include "Texture.hpp"

#include <iostream>
#include "API.hpp"
#include "Image.hpp"

sr::Texture::Texture()
	: size(0), id(0)
{
}

bool sr::Texture::LoadFromFile(const std::string& relativePath, const sr::FileSystem& fileSystem)
{
    Unload();
    Name(relativePath);

   // std::cout << "Texture: " << relativePath << std::endl;
    const std::string fullPath = fileSystem.GetFullPath(relativePath); 

    sr::Image image(fullPath);
    if(!image.Loaded())
        return false;

    const glm::uvec2 size = image.Size();
    const unsigned char* bits = image.Bits();

    if(size.x == 0 || size.y == 0 || !bits)
    {
        return false;
    }

    // Configure
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    	
	GLfloat maxAniso(0.0f);
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);

    // Generate
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0 /* mipmap level */, GL_RGBA /* internal format */, size.x, size.y, 0 /* border */, GL_RGBA /* image format */, GL_UNSIGNED_BYTE, bits);
        
    if(id != 0)
    {
        this->size = size;
        Loaded(true);
        return true;
    }
    else
    {
        return false;
    }
}

void sr::Texture::Unload()
{
    if(id)
    {
	    glDeleteTextures(1, &id);
        id = 0;
    }

    Loaded(false);
    size = glm::uvec2();
}

void sr::Texture::Bind()
{
	glBindTexture(GL_TEXTURE_2D, id);
}
