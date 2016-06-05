#include "ModelInstance.hpp"
#include "WorldInfo.hpp"
#include "..\Graphics\TextureShader.hpp"

sr::ModelInstance::ModelInstance(const sr::ModelPtr _model, const sr::WorldInfo::ModelObject& _worldModel)
    : model(_model), worldModel(_worldModel), position(_worldModel.Position()), scale(_worldModel.Scale()), rotation(_model->Info()->GetNode(0).Rotation() * _worldModel.Rotation())
{
    UpdateTransform();
}

void sr::ModelInstance::Render(sr::TextureShader& shader)
{
    shader.SetModel(transform);
    model->Render(shader);
}

void sr::ModelInstance::UpdateTransform()
{    
    const glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);
    const glm::mat4 transMat = glm::translate(glm::mat4(1.0f), position);
    transform = scaleMat * rotation * transMat * model->Info()->GetNode(0).Origin();
}
