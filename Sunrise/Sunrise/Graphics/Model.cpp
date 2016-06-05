#include "Model.hpp"
#include "ModelNode.hpp"
#include "TextureManager.hpp"
#include "TextureShader.hpp"

sr::Model::Model(const sr::ModelInfoPtr _info, sr::TextureManager& textureManager)
    : info(_info)
{
    LoadTextures(textureManager);
    Create();
}

void sr::Model::LoadTextures(sr::TextureManager& textureManager)
{
    textures.resize(info->TextureCount());

    for(std::size_t i = 0; i < textures.size(); ++i)
    {   
        std::string textureName = std::string("data\\texture\\") + info->GetTexture(i);
        textures[i] = textureManager.LoadTexture(textureName);
    }
}

void sr::Model::Create()
{
    nodes.resize(info->NodeCount());

    for(std::size_t i = 0; i < info->NodeCount(); ++i)
    {
        nodes[i] = std::make_shared<ModelNode>(*this, info->GetNode(i));
    }
}

void sr::Model::Render(sr::TextureShader& shader)
{
    nodes[0]->Render(shader);
}
