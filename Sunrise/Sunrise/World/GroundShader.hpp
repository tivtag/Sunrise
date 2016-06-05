#pragma once

#include <glm/glm.hpp>

#include "../Graphics/TextureShader.hpp"
#include "../Graphics/Texture.hpp"

namespace sr
{
    class FileSystem;

    class GroundShader : public sr::TextureShader
    {
    public:
        GroundShader(sr::FileSystem&);
        ~GroundShader();
        bool Load();

        void SetTexture(sr::TexturePtr texture);     
        void SetModel(const glm::mat4& model);
        void SetViewProj(const glm::mat4& view, const glm::mat4& viewProj);

        void SetSunLightPosition(const glm::vec3&);
        
        void Begin();
        void End();        
    private:
        glm::mat4 viewProj;

	    GLuint fragShader, vertexShader;	// Used to hold the fragment and vertex shader handles
	    GLuint programObject;				// Used to hold the program handle (made out of the two previous shaders)

        GLuint location_textureID,          // texture sampler shader input argument
               location_model,              // MVP matrix shader input argument
               location_view, 
               location_mvp,
               location_lightPosition;

        sr::FileSystem& fileSystem;
    };
}
