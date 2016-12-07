#include "TextureManager.hpp"
#include <GLES2\gl2ext.h>

//**********************************************
//Singleton Texture Manager class
//Written by Ben English
//benjamin.english@oit.edu
//
//For use with OpenGL and the FreeImage library
//**********************************************

TextureManager* TextureManager::m_inst(0);

TextureManager* TextureManager::Inst()
{
	if(!m_inst)
		m_inst = new TextureManager();

	return m_inst;
}

TextureManager::TextureManager()
{
	// call this ONLY when linking with FreeImage as a static library
	#ifdef FREEIMAGE_LIB
		FreeImage_Initialise();
	#endif
}

//these should never be called
//TextureManager::TextureManager(const TextureManager& tm){}
//TextureManager& TextureManager::operator=(const TextureManager& tm){}
	
TextureManager::~TextureManager()
{
	// call this ONLY when linking with FreeImage as a static library
	#ifdef FREEIMAGE_LIB
		FreeImage_DeInitialise();
	#endif

	//UnloadAllTextures();
	m_inst = 0;
}


unsigned DLL_CALLCONV 
_ReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle) {
	return (unsigned)fread(buffer, size, count, (FILE *)handle);
}

unsigned DLL_CALLCONV 
_WriteProc(void *buffer, unsigned size, unsigned count, fi_handle handle) {
	return (unsigned)fwrite(buffer, size, count, (FILE *)handle);
}

int DLL_CALLCONV
_SeekProc(fi_handle handle, long offset, int origin) {
	return fseek((FILE *)handle, offset, origin);
}

long DLL_CALLCONV
_TellProc(fi_handle handle) {
	return ftell((FILE *)handle);
}


GLuint TextureManager::LoadTexture(const char* filename, GLenum image_format, GLint internal_format, GLint level, GLint border)
{
	//image format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	//pointer to the image, once loaded
	FIBITMAP *dib(0);
	//pointer to the image data
	BYTE* bits(0);
	//image width and height
	unsigned int width(0), height(0);
	//OpenGL's image ID to map to
	GLuint gl_texID;
	
	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(filename, 0);
	//if still unknown, try to guess the file format from the file extension
	if(fif == FIF_UNKNOWN) 
		fif = FreeImage_GetFIFFromFilename(filename);
	//if still unkown, return failure
	if(fif == FIF_UNKNOWN)
		return 0;

	//check that the plugin has reading capabilities and load the file
	if(FreeImage_FIFSupportsReading(fif))
	{		
		dib = FreeImage_Load(fif, filename, PNG_DEFAULT);
	}


	//if the image failed to load, return failure
	if(!dib)
		return 0;

	//retrieve the image data
	bits = FreeImage_GetBits(dib);
	//get the image width and height
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	
	//if this somehow one of these failed (they shouldn't), return failure
	if((bits == 0) || (width == 0) || (height == 0))
		return 0;
	
	// Calculate the number of bytes per pixel (3 for 24-bit or 4 for 32-bit)
	const int bpp = FreeImage_GetLine(dib) / FreeImage_GetWidth(dib);

	// Exchange red and blue channel because FreeImage loads in BGRA format; while we need it as RGBA:
	for(unsigned y = 0; y < height; ++y)
	{
		BYTE* bits = FreeImage_GetScanLine(dib, y);

		for( unsigned x = 0; x < width; ++x ) 
		{
			BYTE r = bits[FI_RGBA_BLUE];			
			BYTE b = bits[FI_RGBA_RED];
			bits[FI_RGBA_RED]   = r;
			bits[FI_RGBA_BLUE]  = b;

			// jump to next pixel
			bits += bpp;
		}
	}

	//generate an OpenGL texture ID for this texture
	glGenTextures(1, &gl_texID);
	//store the texture ID mapping

	//bind to the new texture ID
	glBindTexture(GL_TEXTURE_2D, gl_texID);
	//store the texture data for OpenGL use
	glTexImage2D(GL_TEXTURE_2D, level, internal_format, width, height,
		border, image_format, GL_UNSIGNED_BYTE, bits);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	
	GLfloat maxAniso(0.0f);
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT      , &maxAniso);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);

	//Free FreeImage's copy of the data
	FreeImage_Unload(dib);

	//return success
	return gl_texID;
}

bool TextureManager::UnloadTexture(const unsigned int texID)
{
	bool result(true);

	
	glDeleteTextures(1, &texID);

	////if this texture ID mapped, unload it's texture, and remove it from the map
	//if(m_texID.find(texID) != m_texID.end())
	//{
	//	glDeleteTextures(1, &(m_texID[texID]));
	//	m_texID.erase(texID);
	//}
	////otherwise, unload failed
	//else
	//{
	//	result = false;
	//}

	return result;
}

//bool TextureManager::BindTexture(const unsigned int texID)
//{
//	bool result(true);
//	//if this texture ID mapped, bind it's texture as current
//	if(m_texID.find(texID) != m_texID.end())
//		glBindTexture(GL_TEXTURE_2D, m_texID[texID]);
//	//otherwise, binding failed
//	else
//		result = false;
//
//	return result;
//}
//
//void TextureManager::UnloadAllTextures()
//{
//	//start at the begginning of the texture map
//	std::map<unsigned int, GLuint>::iterator i = m_texID.begin();
//
//	//Unload the textures untill the end of the texture map is found
//	while(i != m_texID.end())
//		UnloadTexture(i->first);
//
//	//clear the texture map
//	m_texID.clear();
//}