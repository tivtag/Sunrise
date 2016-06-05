#pragma once

#include <boost\noncopyable.hpp>
#include <vector>
#include <glm\glm.hpp>

#include "ModelInfo.hpp"

namespace sr
{
    class Model;
    class TexturedTriangleBuffer;
    class TextureShader;

	/*
	    A ModelNode contains the triangle data of the model for a specific key frame.
	*/
    class ModelNode : public boost::noncopyable 
    {
    public:
        ModelNode(sr::Model& parent, const sr::ModelInfo::Node& );
        ~ModelNode();

        void Render(sr::TextureShader&);

    private:
        void Create();

    private:
        std::vector<TexturedTriangleBuffer*> buffers;
        glm::mat4 transform;

        const sr::Model& parent;
        const sr::ModelInfo::Node& info;
    };
}
