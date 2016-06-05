#include "ModelNode.hpp"
#include "Model.hpp"
#include "TexturedTriangleBuffer.hpp"
#include "Vertex.hpp"
#include "TextureShader.hpp"

sr::ModelNode::ModelNode(sr::Model& _parent, const sr::ModelInfo::Node& _info)
    : info(_info), parent(_parent), transform(_info.Origin())
{
    Create();
}

sr::ModelNode::~ModelNode()
{
    for(auto itr = std::begin(buffers); itr != std::end(buffers); ++itr)
    {
        delete *itr;
    }
}

void sr::ModelNode::Create()
{
    const std::size_t faceCount = info.FaceCount();
    const std::size_t vertexCount = info.VertexCount();
    const std::size_t textureCount = parent.TextureCount();

    // Generate vertices
    std::vector<Vertex> vertices;
    
    for(std::size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
    {
        const sr::ModelInfo::Vertex& v = info.GetVertex(vertexIndex);        
        vertices.push_back(Vertex(v.x, v.y, v.z, 0, 0));
    }
    
    // Generate vertex normals
    for(std::size_t faceIndex = 0; faceIndex < faceCount; ++faceIndex)
    {
        const sr::ModelInfo::Face& face = info.GetFace(faceIndex);

        sr::Vertex& a = vertices[face.vertidx[0]];
        sr::Vertex& b = vertices[face.vertidx[1]];
        sr::Vertex& c = vertices[face.vertidx[2]];    
             
        const glm::vec3 edgeA = b.position - a.position;
        const glm::vec3 edgeB = c.position - a.position;
        const glm::vec3 faceNormal = glm::normalize(glm::cross(edgeA, edgeB));
        a.normal += faceNormal;
        b.normal += faceNormal;
        c.normal += faceNormal;
    }

    // Normalize vertex normals    
    for(std::size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
    {
        sr::Vertex& v = vertices[vertexIndex];
        v.normal = glm::normalize(v.normal);
    }

    // Generate output vertices
    std::vector<std::vector<sr::Vertex>> outputVerticesByTexture;
    outputVerticesByTexture.resize(textureCount);
    
    for(std::size_t faceIndex = 0; faceIndex < faceCount; ++faceIndex)
    {
        const sr::ModelInfo::Face& face = info.GetFace(faceIndex);
        sr::Vertex& a = vertices[face.vertidx[0]];
        sr::Vertex& b = vertices[face.vertidx[1]];
        sr::Vertex& c = vertices[face.vertidx[2]];

        const sr::ModelInfo::TVertex ta = info.GetTextureVertex(face.tvertidx[0]);
        const sr::ModelInfo::TVertex tb = info.GetTextureVertex(face.tvertidx[1]);
        const sr::ModelInfo::TVertex tc = info.GetTextureVertex(face.tvertidx[2]);
        a.uv = glm::vec2(ta.u, ta.v);
        b.uv = glm::vec2(tb.u, tb.v);
        c.uv = glm::vec2(tc.u, tc.v);
        
        std::vector<sr::Vertex>& faceVertices = outputVerticesByTexture[face.texid];
        faceVertices.push_back(a);
        faceVertices.push_back(b);
        faceVertices.push_back(c);
    }

    // Generate buffers
    buffers.reserve(textureCount);

    for(std::size_t bufferIndex = 0; bufferIndex < textureCount; ++bufferIndex)
    {    
        std::vector<sr::Vertex>& outputVertices = outputVerticesByTexture[bufferIndex];

        if(outputVertices.size() > 0)
        {
            GLuint vboId;
	         glGenBuffers(1, &vboId);

	         const GLuint vertexStride = sizeof(Vertex); // 3 floats for the pos, 2 for the UVs, 3 for normal
    
	         // Bind the VBO
	         glBindBuffer(GL_ARRAY_BUFFER, vboId);

	         // Set the buffer's data
	         glBufferData(GL_ARRAY_BUFFER, outputVertices.size() * sizeof(Vertex), &outputVertices[0], GL_STATIC_DRAW);

	         // Unbind the VBO
	         glBindBuffer(GL_ARRAY_BUFFER, 0);

            // Load Texture
            const sr::TexturePtr texture(parent.GetTexture(bufferIndex));
    
            // Create Buffer
            buffers.push_back(new sr::TexturedTriangleBuffer(vboId, vertexStride, outputVertices.size(), texture));
        }
    }
}

void sr::ModelNode::Render(sr::TextureShader& shader)
{    
    for(auto itr = std::begin(buffers); itr != std::end(buffers); ++itr)
    {
        auto buffer = *itr;
        shader.SetTexture(buffer->Texture());
        buffer->Render(false);
    }
}
