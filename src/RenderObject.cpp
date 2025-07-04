#include "RenderObject.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

// update the position of the object and rotate according to body type
void RenderObject::updateTransform()
{
    if (!geometry)
        return;

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // check if the body is static or dynamic and
    if (bodyType == RigidBodyType::DYNAMIC && dynamicBody)
    {

        physx::PxTransform transform = dynamicBody->getGlobalPose();

        modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(transform.p.x, transform.p.y, transform.p.z));

        glm::quat rotation = glm::quat(transform.q.w, transform.q.x, transform.q.y, transform.q.z);
        modelMatrix *= glm::mat4_cast(rotation);
    }
    else if (bodyType == RigidBodyType::STATIC && staticBody)
    {
        physx::PxTransform transform = staticBody->getGlobalPose();

        modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(transform.p.x, transform.p.y, transform.p.z));
    }

    geometry->setModelMatrix(modelMatrix);
}

void RenderObject::setTransform(const glm::vec3 &position, const glm::vec3 &forward, bool spin)
{
    if (!geometry)
    {
        return;
    }
    glm::vec3 f = glm::normalize(forward);
    glm::vec3 up = glm::vec3(0, 1, 0);
    glm::vec3 right = glm::normalize(glm::cross(up, f));
    up = glm::cross(f, right);

    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0] = glm::vec4(right, 0.0f);
    rotation[1] = glm::vec4(up, 0.0f);
    rotation[2] = glm::vec4(f, 0.0f);

    glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);

    glm::mat4 model = translation * rotation;
    if (spin)
    {
        float spinAngle = float(glfwGetTime()) * 2.0f;

        glm::mat4 spinMat = glm::rotate(glm::mat4(1.0f), spinAngle, glm::vec3(0, 1, 0));
        model *= spinMat;
    }

    geometry->setModelMatrix(model);
}

void RenderObject::setPosition(const glm::vec3 &position)
{
    if (geometry)
    {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
        geometry->setModelMatrix(translation);
    }

    physx::PxVec3 pxPos(position.x, position.y, position.z);

    if (dynamicBody)
    {
        physx::PxTransform t = dynamicBody->getGlobalPose();
        t.p = pxPos;
        dynamicBody->setGlobalPose(t);
    }
    else if (staticBody)
    {
        physx::PxTransform t = staticBody->getGlobalPose();
        t.p = pxPos;
        staticBody->setGlobalPose(t);
    }
}
void RenderObject::setAsPickable()
{
    isPickable = true;
}

void RenderObject::setIdentifier(std::string name)
{
    id = name;
}
