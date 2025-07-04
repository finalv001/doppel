#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <PxPhysicsAPI.h>
#include "Physics.h"

class POVCamera
{
private:
    void updateCameraVectors();
    float POVCamera::getBodySize();

public:
    // bool isGrounded(physx::PxScene *scene);
    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 upward;
    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;
    float width;
    float height;
    float getFootPos() const;
    glm::vec3 getForward() const;
    POVCamera(glm::vec3 startPosition, float width, float height, Physics &physics);
    POVCamera();
    void processKeyboard(char direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset);
    void POVCamera::setSprinting(bool isSprinting);
    glm::mat4 getViewProjectionMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const;
    void setPosition(glm::vec3 position);
    glm::mat4 projection;
    bool headBobActive;
    bool isJumping;
    bool isGrounded;
    bool isSprinting;
    float verticalVelocity;
    float jumpImpulse;
    float headBobTimer;
    float headBobAmplitude;
    float headBobFrequency;
    bool inBloomyWorld;
    float groundedTimer;
    static constexpr float COYOTE_TIME = 0.1f;
    void updateHeadBob(float deltaTime, bool isMoving);
    void updateFromPhysics(physx::PxScene *gScene, float deltaTime);
    glm::mat4 getViewProjNoTransforms() const;
    physx::PxRigidDynamic *cameraBody;
    physx::PxCapsuleController *characterController;
    glm::mat4 getShadowProjectionMatrix() const;
    void POVCamera::moveWithDynamicFilters(const glm::vec3 &displacement, float deltaTime);
};
