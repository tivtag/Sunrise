#pragma once

#include <vector>

#include "../GravityFile.hpp"

namespace sr
{
    class FileSystem;
    class GravityResourceFile;

    class GroundInfo : public GravityFile, boost::noncopyable
    {
    public:
    #pragma pack(push,1)
	    /// Lightmap, represents the shadow projected on a surface.
	    struct Lightmap {
		    unsigned char brightness[8][8];

		    struct {
			    unsigned char r;
			    unsigned char g;
			    unsigned char b;
			    inline operator unsigned char* () { return &r; }
			    inline operator const unsigned char* () const { return &r; }
		    } 
		    color[8][8];
	    };

	    /// Surface, represents the visual aspect of a quad.
	    /// The vertices to use depend on which side of the cell is using the surface.
	    /// top = T0->T1->T2->T3
	    /// front = T2->T3->F0->F1
	    /// right = T3->T1->R2->R0
	    /// 2---3
	    /// | F | (cell y+1)
	    /// 0---1
	    /// 2---3 2---3
	    /// | T | | R | (cell x+1)
	    /// 0---1 0---1
	    struct Surface {
		    float u[4]; //< west->east, south->north ordering; 0=left 1=right
		    float v[4]; //< west->east, south->north ordering; 0=up 1=down
		    short textureId; //< -1 for none
    //		unsigned short lightmapId; //< -1 for none?
		    short lightmapId; //< -1 for none?

		    struct {
			    unsigned char b;
			    unsigned char g;
			    unsigned char r;
			    unsigned char a;
			    inline operator unsigned char* () { return &b; }
			    inline operator const unsigned char* () const { return &b; }
		    }
		    color;//< BGRA -- "A" seems to be ignored by the official client
	    };

	    struct Cell {
		    float height[4]; //< west->east, south->north ordering
		    int topSurfaceId; //< -1 for none
		    int frontSurfaceId; //< -1 for none
		    int rightSurfaceId; //< -1 for none
	    };
    #pragma pack(pop)
	
    public:
	    GroundInfo();
	    bool ReadStream(std::istream&);

	    ~GroundInfo()
	    {
		    Reset();
	    }

        float Zoom() const { return zoom; }
	    unsigned int Width() const { return width; }
	    unsigned int Height() const { return width; }

	    const Cell& GetCell(unsigned int cellx, unsigned int celly) const;
	    const Surface& GetSurface(unsigned int idx) const;

        std::size_t TextureCount() const { return textureCount; }    
        std::string GetTexture(const std::size_t index) const;

        void ExtractFiles(sr::GravityResourceFile&, sr::FileSystem&);

    private:
	    void Reset();
	    bool ReadHeader(std::istream&);
	    bool CheckHeader(const std::string&) const;

    private:
	    unsigned int width;
	    unsigned int height;
	    float zoom;

	    unsigned int textureCount;
	    unsigned int textureSize;
	    char* textures;

	    std::vector<Lightmap> lightmaps;
	    std::vector<Surface> surfaces;
	    std::vector<Cell> cells; //< west->east, south->north ordering
    };
}
