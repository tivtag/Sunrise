#pragma once

#include <string>
#include "API.hpp"

namespace sr
{	
	/*
	   A shader is a program that is executed on the Graphics Processing Unit (GPU).
	*/
    class Shader
    {
    public:
        virtual ~Shader() {}
        
    protected:
        GLuint CompileFromSource(const std::string& source, const GLenum type);        
        std::string GetShaderInfoLog(const GLuint shaderId);
    };
}
