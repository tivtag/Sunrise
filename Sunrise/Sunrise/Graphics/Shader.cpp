#include "Shader.hpp"
#include "..\Log.hpp"

GLuint sr::Shader::CompileFromSource(const std::string& source, const GLenum type)
{
    // Create the shader object
 	GLuint shaderId = glCreateShader(type);

	// Load the source code into it
    const char* fragShaderSourcePtr = source.c_str();
	glShaderSource(shaderId, 1, (const char**)&fragShaderSourcePtr, NULL);

	// Compile the source code
	glCompileShader(shaderId);

	// Check if compilation succeeded
	GLint shaderCompiled;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &shaderCompiled);

	if( shaderCompiled )
    {
        return shaderId;
    }
    else
	{
        glDeleteShader(shaderId);
		return 0;
	}
}

std::string sr::Shader::GetShaderInfoLog(const GLuint shaderId)
{
    std::string log;

    GLsizei logSize;
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logSize);
        
    if(logSize)
    {
        GLchar* buffer = new GLchar[logSize];

        GLsizei actualLength;
        glGetShaderInfoLog(shaderId, logSize, &actualLength, buffer);
        
        log = std::string(buffer);
        delete buffer; // ToDo really delete?
    }
    else
    {
        log = "";
    }

    return log;
}
