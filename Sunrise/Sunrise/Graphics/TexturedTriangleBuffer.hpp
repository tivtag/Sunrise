#pragma once
#include <boost\noncopyable.hpp>

#include "API.hpp"
#include "Texture.hpp"

namespace sr
{
	/*
	   A buffer that holds triangles textured with the same texture
	*/
    class TexturedTriangleBuffer : public boost::noncopyable
    {
    public:
        TexturedTriangleBuffer(GLuint vboId, GLuint vertexStride, GLuint vertexCount, sr::TexturePtr);
        ~TexturedTriangleBuffer();

        sr::TexturePtr Texture() const { return texture; }
        void Render(bool asWireframe = false);
    private:
        GLuint vboId;        
	    GLuint vertexStride;	
        GLuint vertexCount;

        sr::TexturePtr texture;
    };
}
