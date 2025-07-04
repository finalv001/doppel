#pragma once

#include "RenderObject.h"
#include "POVCamera.h"
#include <vector>
#include <PxPhysicsAPI.h>

namespace ObjectPicker {
    glm::vec3 calculateRayDirection(double mouseX, double mouseY, int width, int height, const POVCamera& camera);
    RenderObject* pickObject(double mouseX, double mouseY, int width, int height, const POVCamera& camera, physx::PxScene* scene);
}