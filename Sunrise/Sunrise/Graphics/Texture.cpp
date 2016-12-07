#include "Texture.hpp"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <FreeImage/FreeImage.h>

sr::Texture::Texture()
	: size(0), id(0)
{
}

bool sr::Texture::LoadFromFile(const std::string& relativePath, const sr::FileSystem& fileSystem)
{
    Unload();
    Name(relativePath);
    
    const std::string fullPath   = fileSystem.GetFullPath(relativePath);
    const char* filename         = fullPath.c_str();
    const GLenum image_format    = GL_RGB;    // format the image is in
    const GLint  internal_format = GL_RGB;    // format to store the image in
    const GLint  level           = 0;	        // mipmapping level
    const GLint  border          = 0;	        // border size
        	
	//check the file signature and deduce its format
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(filename, 0);

    // Try to guess the file format from the file extension:
	if(fif == FIF_UNKNOWN) 
		fif = FreeImage_GetFIFFromFilename(filename);

	// if still unkown, return failure
	if(fif == FIF_UNKNOWN || !FreeImage_FIFSupportsReading(fif))
		return false;

	//check that the plugin has reading capabilities and load the file
    FIBITMAP* const dib = FreeImage_Load(fif, filename, PNG_DEFAULT);
	if(!dib) 
        return false;
    
	BYTE* const bits = FreeImage_GetBits(dib);
	const unsigned int width = FreeImage_GetWidth(dib);
	const unsigned int height = FreeImage_GetHeight(dib);
	
	// if this somehow one of these failed (they shouldn't), return failure
	if((bits == nullptr) || (width == 0) || (height == 0))
		return false;
	
	// Calculate the number of bytes per pixel (3 for 24-bit or 4 for 32-bit)
	const int bpp = FreeImage_GetLine(dib) / FreeImage_GetWidth(dib);

	// Exchange red and blue channel because FreeImage loads in BGRA format; while we need it as RGBA:
	for(unsigned y = 0; y < height; ++y)
	{
		BYTE* line = FreeImage_GetScanLine(dib, y);

		for( unsigned x = 0; x < width; ++x ) 
		{
			const BYTE r = line[FI_RGBA_BLUE];			
			const BYTE b = line[FI_RGBA_RED];
			line[FI_RGBA_RED]  = r;
			line[FI_RGBA_BLUE] = b;

			// jump to next pixel
			line += bpp;
		}
	}

    // Generate
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, level, internal_format, width, height, border, image_format, GL_UNSIGNED_BYTE, bits);

    // Configure
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    	
	GLfloat maxAniso(0.0f);
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);

	// Free FreeImage's copy of the data
	FreeImage_Unload(dib);

    if(id != 0)
    {
        size = glm::uvec2(width, height);
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
