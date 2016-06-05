#include "TexturedTriangleBuffer.hpp"

const int VERTEX_ARRAY  = 0;
const int ATTRIB_UV		= 2;
const int ATTRIB_NORMAL = 3;

sr::TexturedTriangleBuffer::TexturedTriangleBuffer(GLuint _vboId, GLuint _vertexStride, GLuint _vertexCount, sr::TexturePtr _texture)
    : vboId(_vboId), vertexStride(_vertexStride), vertexCount(_vertexCount), texture(_texture)
{
}

sr::TexturedTriangleBuffer::~TexturedTriangleBuffer()
{
    glDeleteBuffers(1, &vboId);
}

void sr::TexturedTriangleBuffer::Render(bool asWireframe)
{    		
	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, vboId);

	// Pass the vertex data
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, vertexStride, 0);

	// Pass the texture coordinates data
	glEnableVertexAttribArray(ATTRIB_UV);
	glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, vertexStride, (void*)(3 * sizeof(GLfloat)));
        
	// Pass the normal data
	glEnableVertexAttribArray(ATTRIB_NORMAL);
	glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, vertexStride, (void*)(5 * sizeof(GLfloat)));

	// Draws a non-indexed triangle array
	glDrawArrays(asWireframe ? GL_LINES : GL_TRIANGLES, 0, vertexCount);

	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
