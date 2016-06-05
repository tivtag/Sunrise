#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "..\Graphics\Model.hpp"
#include "WorldInfo.hpp"

namespace sr
{
    class ModelInstance
    {
    public:
        ModelInstance(const sr::ModelPtr, const sr::WorldInfo::ModelObject&);
        
        void Render(sr::TextureShader&);

        glm::vec3 Position() const { return position; }
        void Position(const glm::vec3& newPosition) 
        {
            position = newPosition;
            UpdateTransform();
        }

    private:
        void UpdateTransform();

    private:
        glm::vec3 position;
        glm::vec3 scale;
        glm::mat4 rotation;

        glm::mat4 transform;
        const sr::ModelPtr model;
        const sr::WorldInfo::ModelObject& worldModel;
    };
}

