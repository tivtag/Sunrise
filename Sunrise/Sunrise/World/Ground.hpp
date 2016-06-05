#pragma once

#include <vector>
#include <string>
#include <boost/noncopyable.hpp>

#include "GroundInfo.hpp"

#include "../Graphics/TextureManager.hpp"
#include "../Graphics/Vertex.hpp"
#include "../Graphics/TexturedTriangleBuffer.hpp"

namespace sr
{
    class GroundShader;

    class GroundTexturePartition : boost::noncopyable
    {       
    public:
        GroundTexturePartition(const short textureId, const sr::GroundInfo& info);
        ~GroundTexturePartition();
        
        short TextureId() const { return textureId; }
        void AddTriangle(sr::Vertex a, sr::Vertex b, sr::Vertex c);

        void Load(sr::TextureManager& textureManager);
        void Render(sr::GroundShader& shader);

    private:
        void AddVertex(const sr::Vertex& vertex) { vertices.push_back(vertex); }

    private:
        const short textureId;
        std::vector<sr::Vertex> vertices;

        sr::TexturedTriangleBuffer* buffer;
        const sr::GroundInfo& info;
    };

    class Ground
    {
    public:
        Ground();
        ~Ground();
        void Load(sr::GroundInfo&, sr::TextureManager& textureManager);
                
        void Render(sr::GroundShader& shader);
    private:
        sr::GroundTexturePartition& GetPartitionForTexture(const short textureId);

    private:
        std::vector<GroundTexturePartition*> partitions;
        sr::GroundInfo* info;
    };
}
