#include "Image.hpp"

#include <FreeImage/FreeImage.h>
#include "../Log.hpp"

sr::Image::Image(const std::string& fullPath)
    : Resource(fullPath), dib(nullptr)
{
    LoadFromFile(fullPath);
}

void ExchangeChannelsRedBlue(FIBITMAP* dib)
{
    // Calculate the number of bytes per pixel (3 for 24-bit or 4 for 32-bit)
	const unsigned int width  = FreeImage_GetWidth(dib);
	const unsigned int height = FreeImage_GetHeight(dib);
	const unsigned int bpp    = FreeImage_GetLine(dib) / width;

	// Exchange red and blue channel because FreeImage loads in BGRA format; while we need it as RGBA:
	for(unsigned y = 0; y < height; ++y)
	{
		BYTE* line = FreeImage_GetScanLine(dib, y);

		for( unsigned x = 0; x < width; ++x ) 
		{
			const BYTE b = line[FI_RGBA_BLUE];			
			const BYTE r = line[FI_RGBA_RED];
			line[FI_RGBA_RED]  = b;
			line[FI_RGBA_BLUE] = r;

            if(r==255 && b==255)
            {
			    line[FI_RGBA_ALPHA] = 0;
            }

			// jump to next pixel
			line += bpp;
		}
	}    
}

void sr::Image::LoadFromFile(const std::string& fullPath)
{
    const char* filename = fullPath.c_str();
        	
	//check the file signature and deduce its format
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(filename, 0);

    // Try to guess the file format from the file extension:
	if(fif == FIF_UNKNOWN) 
		fif = FreeImage_GetFIFFromFilename(filename);

	// if still unkown, return failure
	if(fif == FIF_UNKNOWN || !FreeImage_FIFSupportsReading(fif))
		return;

	//check that the plugin has reading capabilities and load the file
    FIBITMAP* dib = FreeImage_Load(fif, filename, PNG_DEFAULT);
	if(!dib)
    {
        std::cerr << std::endl << "Could not load image " << filename << std::endl;
        return;
    }

	BYTE* const bits = FreeImage_GetBits(dib);
	const unsigned int width = FreeImage_GetWidth(dib);
	const unsigned int height = FreeImage_GetHeight(dib);
	
	// if this somehow one of these failed (they shouldn't), return failure
	if((bits == nullptr) || (width == 0) || (height == 0))
    {
        std::cerr << "Errornous image " << filename << std::endl;
		return;
    }

    auto colorType = FreeImage_GetColorType(dib);
    
    // Convert to RGBA
    {
        FIBITMAP* convertedDib = FreeImage_ConvertTo32Bits(dib);
        FreeImage_Unload(dib);
        dib = convertedDib;
    }
    
    ExchangeChannelsRedBlue(dib);
	
    this->dib = dib;
    Loaded(true);
}
        
void sr::Image::Unload()
{
    if(dib)
    {
    	FreeImage_Unload(dib);
        dib = nullptr;
    }
    
    Loaded(false);
}

unsigned char* sr::Image::Bits() const 
{
    return FreeImage_GetBits(dib);
}

glm::uvec2 sr::Image::Size() const
{
    return glm::uvec2( FreeImage_GetWidth(dib), FreeImage_GetHeight(dib) );
}

bool sr::Image::SaveAsPng(const std::string& fullPath) const
{
    const char* filename = fullPath.c_str(); 
    BOOL result = FreeImage_Save(FIF_PNG, dib, filename, PNG_Z_DEFAULT_COMPRESSION);
    return result == TRUE;
}