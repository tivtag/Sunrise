#include "GroundInfo.hpp"
#include "..\FileSystem.hpp"
#include "..\GravityResourceFile.hpp"

using namespace sr;

static GroundInfo::Lightmap g_emptyLightmap = {
	{
		{ 0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0 }
	},{
		{ {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0} },
		{ {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0} },
		{ {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0} },
		{ {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0} },
		{ {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0} },
		{ {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0} },
		{ {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0} },
		{ {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0} }
	}
};
static GroundInfo::Surface g_emptySurface = {
	0,0,0,0,
	0,0,0,0,
	-1,
	0xFFFF,
	0,0,0,0
};
static GroundInfo::Cell g_emptyCell = {
	0,0,0,0,
	-1,
	-1,
	-1
};

GroundInfo::GroundInfo()
	: textures(0)
{		
	width = 0;
	height = 0;
	zoom = 10.0f;
	textureCount = 0;
	textureSize = 0;
}

const GroundInfo::Cell& GroundInfo::GetCell(unsigned int cellx, unsigned int celly) const
{
	if (cellx < width && celly < height)
		return cells[cellx + celly * width];
	else
		return g_emptyCell;
}

const GroundInfo::Surface& GroundInfo::GetSurface(unsigned int idx) const {
	if (idx < surfaces.size())
		return(surfaces[idx]);
	return(g_emptySurface);
}

bool GroundInfo::ReadHeader(std::istream &s) 
{
	s.read((char*)&magic, magicSize);
	s.read((char*)&version, 2);
	return !s.fail();
}

bool GroundInfo::CheckHeader(const std::string& s) const 
{
	return !strncmp(s.c_str(), magic, magicSize);
}

void GroundInfo::Reset() 
{
	valid = false;
	width = 0;
	height = 0;
	zoom = 10.0f;
	textureCount = 0;
	textureSize = 0;
	
	if( textures != NULL )
	{
		delete[] textures;
		textures = 0;
	}

	lightmaps.clear();
	surfaces.clear();
	cells.clear();
}

bool GroundInfo::ReadStream(std::istream& s)
{
	Reset();

	if( !ReadHeader(s) )
	{
		return false;
	}
	
	if( !CheckHeader("GRGN") ) {
		//_log(ROINT__DEBUG, "Invalid GND header (%c%c%c%c)", magic[0], magic[1], magic[2], magic[3]);
		return false;
	}

	if( !(version.cver.major == 1 && version.cver.minor == 7) ) {
		//_log(ROINT__DEBUG, "Unsupported GND version (%u.%u)", m_version.cver.major, m_version.cver.minor);
		return false;
	}

	int i;
	int width, height;
	int textureCount, textureSize;

	s.read((char*)&width, sizeof(int));
	s.read((char*)&height, sizeof(int));
	s.read((char*)&zoom, sizeof(float));
	s.read((char*)&textureCount, sizeof(int));
	s.read((char*)&textureSize, sizeof(int));
	if( s.fail() ) 
	{
		Reset();
		return false;
	}

	// Size
	if( width < 0 || height < 0 )
	{
		//_log(ROINT__DEBUG, "Invalid GND size (width=%d height=%d)", width, height);
		Reset();
		return false;
	}

	this->width = (unsigned int)width;
	this->height = (unsigned int)height;

	// Read Textures
	if( textureCount < 0 || textureSize < 0 )
	{
		//_log(ROINT__DEBUG, "Invalid GND texture size (nTextures=%d textureSize=%d)", nTextures, textureSize);
		Reset();
		return false;
	}

	const unsigned int totalTextureSize = textureCount * textureSize;
	this->textureCount = (unsigned int)textureCount;
	this->textureSize = (unsigned int)textureSize;
	this->textures = new char[totalTextureSize];

	s.read(textures, totalTextureSize);
	
	for( i = 0; i < textureCount; ++i )
	{
		char* texture = &textures[i * textureSize];
		texture[textureSize - 1] = '\0';
	}

	// Read Lightmaps
	int lightmapCount, lmapWidth, lmapHeight, lmapCells;
	s.read((char*)&lightmapCount, sizeof(int));
	s.read((char*)&lmapWidth, sizeof(int));
	s.read((char*)&lmapHeight, sizeof(int));
	s.read((char*)&lmapCells, sizeof(int));
	if (s.fail()) {
		Reset();
		return false;
	}

	if (lightmapCount < 0 || lmapWidth != 8 || lmapHeight != 8 || lmapCells != 1) {
		//_log(ROINT__DEBUG, "Invalid GND lightmap size (nLightmaps=%d lmapWidth=%d lmapHeight=%d lmapCells=%d)", nLightmaps, lmapWidth, lmapHeight, lmapCells);
		Reset();
		return false;
	}

	lightmaps.resize((unsigned int)lightmapCount);
	for(i = 0; i < lightmapCount; ++i)
	{
		Lightmap& lightmap = lightmaps[i];
		s.read((char*)&lightmap, sizeof(Lightmap));
	}

	// read surfaces
	int surfaceCount;
	s.read((char*)&surfaceCount, sizeof(int));
	if( s.fail() )
    {
		Reset();
		return false;
	}

	if(surfaceCount < 0)
    {
		//_log(ROINT__DEBUG, "Invalid GND surface size (nSurfaces=%d)", nSurfaces);
		Reset();
		return false;
	}

	int size = sizeof(Surface);
	std::cout << size;

	surfaces.resize((unsigned int)surfaceCount);
	for(i = 0; i < surfaceCount; ++i)
    {
		Surface& surface = surfaces[i];
		s.read((char*)&surface, sizeof(Surface));
		
		if(s.fail()) 
		{
			Reset();
			return false;
		}
	}

	if(s.fail())
    {
		Reset();
		return false;
	}

	// read cells
	int cellCount = width * height;
	cells.resize(static_cast<std::size_t>(cellCount));

    const float HeightFactor = 10.0f;

	for(i = 0; i < cellCount; ++i) 
    {
		Cell& cell = cells[i];
		s.read((char*)&cell, sizeof(Cell));

		for(int c = 0; c < 4; ++c)
		{
			cell.height[c] = cell.height[c] / HeightFactor;
		}
	}

	if (s.fail()) {
		Reset();
		return false;
	}

	valid = true;
	return true;
}

std::string sr::GroundInfo::GetTexture(const std::size_t index) const
{
    if(index >= 0 && index < textureCount)
    {
        const std::size_t lineIndex = index * textureSize;
		const char* ptr = &textures[lineIndex];
        return std::string(ptr);
    }
    else
    {
	    return "";
    }
}

void sr::GroundInfo::ExtractFiles(sr::GravityResourceFile& grf, sr::FileSystem& fileSystem)
{
    for(int i = 0; i < TextureCount(); ++i)
    {
        std::string texture = "data\\texture\\" + GetTexture(i);
            
        if( grf.HasFile(texture))
        {
            std::cout << "found " << texture << std::endl;
        
            std::string targetFile = fileSystem.GetFullPath( texture );
            std::string targetDir = fileSystem.GetFolder( targetFile );
            fileSystem.CreateFolder(targetDir);
        
            grf.SaveFile( texture, targetFile );
        
            //sr::Image img(targetFile);
            //boost::ireplace_last(targetFile, "BMP", "png" );
            //img.SaveAsPng(targetFile);
        }
        else
        {
            std::cout << "missing " << texture << std::endl;
        }
    }         
}