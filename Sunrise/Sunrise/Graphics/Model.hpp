#pragma once

#include <vector>
#include <memory>
#include <boost\noncopyable.hpp>

#include "Texture.hpp"
#include "ModelInfo.hpp"

namespace sr
{
    class TextureManager;
    class TexturedTriangleBuffer;
    class ModelNode;
    class TextureShader;

	/*
	   A renderable 3D-Model based on the Gravity ModelInfo
	*/
    class Model : public boost::noncopyable
    {
    public:    
        Model(const sr::ModelInfoPtr, sr::TextureManager&);        
        std::size_t TextureCount() const { return textures.size(); }
        sr::TexturePtr GetTexture(const std::size_t index) const { return textures[index]; }
        const sr::ModelInfoPtr Info() const { return info; }

        void Render(sr::TextureShader&);
    private:
        void Create();
        void LoadTextures(sr::TextureManager&);

    private:

        std::vector<std::shared_ptr<ModelNode>> nodes;
        std::vector<sr::TexturePtr> textures;
        const sr::ModelInfoPtr info;
    };

    typedef std::shared_ptr<sr::Model> ModelPtr;
}
