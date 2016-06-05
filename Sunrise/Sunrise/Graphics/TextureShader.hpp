#pragma once

#include <glm\glm.hpp>

#include "Shader.hpp"
#include "Texture.hpp"

namespace sr
{	
	/*
	   A shader that has a bound texture
	*/
    class TextureShader : public Shader
    {
    public:
        virtual ~TextureShader() {}

        virtual void SetTexture(sr::TexturePtr texture) = 0;

        virtual void SetModel(const glm::mat4& model) = 0;
        virtual void SetViewProj(const glm::mat4& view, const glm::mat4& viewProj) = 0;
    };
}
