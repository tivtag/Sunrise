#include "Ground.hpp"

#include <vector>
#include <boost/algorithm/string/replace.hpp>

#include "../Graphics/API.hpp"
#include "GroundShader.hpp"

sr::Ground::Ground()
    : info(nullptr)
{
}

sr::Ground::~Ground()
{
    for(auto itr = partitions.begin(); itr != partitions.end(); ++itr)
    {
        delete *itr;
    }
}

void sr::GroundTexturePartition::AddTriangle(sr::Vertex a, sr::Vertex b, sr::Vertex c)
{
    const glm::vec3 edgeA = b.position - a.position;
    const glm::vec3 edgeB = c.position - a.position;
    const glm::vec3 triangleNormal = glm::normalize(glm::cross(edgeA, edgeB));
    a.normal = b.normal = c.normal = triangleNormal;

    AddVertex(a);
    AddVertex(b);
    AddVertex(c);
}

sr::GroundTexturePartition& sr::Ground::GetPartitionForTexture(const short textureId)
{
    for(auto itr = partitions.begin(); itr != partitions.end(); ++itr)
    {
        GroundTexturePartition& partition = **itr;
      
        if(partition.TextureId() == textureId)
        {
            return partition;
        }
    }

    auto partition = new GroundTexturePartition(textureId, *info);
    partitions.push_back(partition);
    return *partition;
}

void sr::Ground::Load(sr::GroundInfo& info, sr::TextureManager& textureManager)
{    
    this->info = &info;

	const unsigned int w = info.Width();
	const unsigned int h = info.Height();

	for( unsigned int x = 0; x < w; ++x )
	{
		for( unsigned int y = 0; y < h; ++y )
		{
			const sr::GroundInfo::Cell& cell = info.GetCell(x, y);

#if 1 /* TILE_UP */
			if(cell.topSurfaceId != -1)
			{
				const GroundInfo::Surface& surface = info.GetSurface(cell.topSurfaceId);
                GroundTexturePartition& partition = GetPartitionForTexture(surface.textureId);
                                                
				partition.AddTriangle( 
                    sr::Vertex( x + 0, y + 0, -cell.height[0], surface.u[0], surface.v[0] ), // lower left
				    sr::Vertex( x + 1, y + 0, -cell.height[1], surface.u[1], surface.v[1] ), // lower right
				    sr::Vertex( x + 1, y + 1, -cell.height[3], surface.u[3], surface.v[3] ) // upper right
                );

				partition.AddTriangle(
                    sr::Vertex( x + 0, y + 0, -cell.height[0], surface.u[0], surface.v[0] ), // lower left
				    sr::Vertex( x + 1, y + 1, -cell.height[3], surface.u[3], surface.v[3] ), // upper right
				    sr::Vertex( x + 0, y + 1, -cell.height[2], surface.u[2], surface.v[2] )  // upper left
                );
			}
#endif

#if 1 /* TILE_SIDE */
			if(cell.frontSurfaceId != -1) 
			{
				const sr::GroundInfo::Cell& cell2 = info.GetCell(x, y + 1);
				const sr::GroundInfo::Surface& surface = info.GetSurface(cell.frontSurfaceId);
                GroundTexturePartition& partition = GetPartitionForTexture(surface.textureId);
				
				partition.AddTriangle(
                    sr::Vertex( x + 0, y + 1,  -cell.height[2], surface.u[2], surface.v[2] ), // lower left
				    sr::Vertex( x + 1, y + 1,  -cell.height[3], surface.u[3], surface.v[3] ), // lower right
				    sr::Vertex( x + 1, y + 1, -cell2.height[1], surface.u[1], surface.v[1] )  // upper right
                );

				partition.AddTriangle(
                    sr::Vertex( x + 0, y + 1, -cell2.height[0], surface.u[0], surface.v[0] ), // upper left
				    sr::Vertex( x + 0, y + 1,  -cell.height[2], surface.u[2], surface.v[2] ),  // lower left
				    sr::Vertex( x + 1, y + 1, -cell2.height[1], surface.u[1], surface.v[1] )  // upper right
                );
			}
#endif

#if 1 /* TILE_ASIDE */
			if(cell.rightSurfaceId != -1) 
			{
				const GroundInfo::Cell& cell2 = info.GetCell(x + 1, y);
				const GroundInfo::Surface& surface = info.GetSurface(cell.rightSurfaceId);
                GroundTexturePartition& partition = GetPartitionForTexture(surface.textureId);

                partition.AddTriangle(
                    sr::Vertex( x + 1, y + 1, -cell.height[3],  surface.u[2], surface.v[2] ), // Lower left
				    sr::Vertex( x + 1, y + 0, -cell.height[1],  surface.u[3], surface.v[3] ), // Lower right
				    sr::Vertex( x + 1, y + 1, -cell2.height[2], surface.u[0], surface.v[0] )  // Upper left
                );

				partition.AddTriangle(
                    sr::Vertex( x + 1, y + 0, -cell.height[1], surface.u[3], surface.v[3] ),  // Lower right
				    sr::Vertex( x + 1, y + 0, -cell2.height[0], surface.u[1], surface.v[1] ), // Upper right
				    sr::Vertex( x + 1, y + 1, -cell2.height[2], surface.u[0], surface.v[0] )  // Upper left
                );
			}
#endif
		}
	}

    for(auto itr = partitions.begin(); itr != partitions.end(); ++itr)
    {
        (*itr)->Load(textureManager);
    }
}

void sr::Ground::Render(sr::GroundShader& shader)
{
    for(auto itr = partitions.begin(); itr != partitions.end(); ++itr)
    {
        (*itr)->Render(shader);
    }
}

sr::GroundTexturePartition::GroundTexturePartition(const short _textureId, const sr::GroundInfo& _info)
    : textureId(_textureId), buffer(nullptr), info(_info)
{
}

sr::GroundTexturePartition::~GroundTexturePartition()
{
    delete buffer;
}

void sr::GroundTexturePartition::Load(sr::TextureManager& textureManager)
{    
    GLuint vboId;
	glGenBuffers(1, &vboId);

	GLuint vertexCount = vertices.size();
	GLuint vertexStride = sizeof(Vertex); // 3 floats for the pos, 2 for the UVs, 3 for normal
    
	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, vboId);

	// Set the buffer's data
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Load Texture
    std::string textureName = std::string("data\\texture\\") + info.GetTexture(textureId);
    sr::TexturePtr texture = textureManager.LoadTexture(textureName);
    
    // Create Buffer
    buffer = new sr::TexturedTriangleBuffer(vboId, vertexStride, vertexCount, texture);
}

void sr::GroundTexturePartition::Render(sr::GroundShader& shader)
{
    shader.SetTexture(buffer->Texture());
    buffer->Render(false);
}
