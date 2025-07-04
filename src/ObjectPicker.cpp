#include "ObjectPicker.h"

namespace ObjectPicker {

glm::vec3 calculateRayDirection(float mouseX, float mouseY, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix)
{
    float x = (2.0f * mouseX) - 1.0f;
    float y = 1.0f - (2.0f * mouseY);
    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

    glm::mat4 invProj = glm::inverse(projectionMatrix);
    glm::mat4 invView = glm::inverse(viewMatrix);

    glm::vec4 rayEye = invProj * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f); // Richtung im Viewspace

    glm::vec4 rayWorld = invView * rayEye;
    return glm::normalize(glm::vec3(rayWorld));
}

RenderObject* pickObject(double mouseX, double mouseY, int width, int height, const POVCamera& camera, physx::PxScene* scene) {

    // Kamera-Position und -Richtung
    glm::vec3 rayOrigin = camera.getPosition();
    glm::vec3 rayDir = glm::normalize(camera.forward);  

    rayOrigin += rayDir * 0.21f;

    // Umwandlung in PhysX-Vektoren
    physx::PxVec3 originPx(rayOrigin.x, rayOrigin.y, rayOrigin.z);
    physx::PxVec3 dirPx(rayDir.x, rayDir.y, rayDir.z);

    // Raycast durchführen
    physx::PxRaycastBuffer hit;
    physx::PxHitFlags hitFlags = physx::PxHitFlag::ePOSITION | physx::PxHitFlag::eNORMAL;
    bool hitSomething = scene->raycast(originPx, dirPx, 5.0f, hit, hitFlags);  // kurze Reichweite!

    if (hitSomething && hit.hasBlock) {
        return reinterpret_cast<RenderObject*>(hit.block.actor->userData);
    }

    return nullptr;

}

}