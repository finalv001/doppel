#include "POVCamera.h"
#include <iostream>
#define M_PI 3.14159265358979323846

POVCamera::POVCamera() {};
// constructor, projection mat is hardcoded in here, clean up later
POVCamera::POVCamera(glm::vec3 startPosition, float width, float height, Physics &physics)
    : position(startPosition), forward(glm::vec3(0.0f, 0.0f, 0.0f)), upward(glm::vec3(0.0f, 1.0f, 0.0f)),
      yaw(-180.0f), pitch(0.0f), movementSpeed(1.5f), mouseSensitivity(0.1f), verticalVelocity(0.0f), groundedTimer(0.0f)
{
    jumpImpulse = 3.5f;
    projection = glm::perspective(glm::radians(90.0f), width / height, 0.1f, 100.0f);
    this->width = width;
    this->height = height;
    headBobActive = false;
    headBobTimer = 0.0f;
    headBobAmplitude = 0.01f;
    headBobFrequency = 13.0f;
    isJumping = false;
    isSprinting = false;
    inBloomyWorld = true;
    isGrounded = false;
    // cameraBody = physics.createCameraBody(position);
    characterController = physics.createCharacterController(position);
    updateCameraVectors();
}

void POVCamera::setSprinting(bool sprinting)
{
    isSprinting = sprinting;
}

float POVCamera::getFootPos() const
{
    return (float)characterController->getFootPosition().y;
}

// update the camera based on yaw and pitch
void POVCamera::updateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    forward = glm::normalize(front);

    // normalize upward
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    upward = glm::normalize(glm::cross(right, forward));
}

// Update camera position based on PhysX controller
void POVCamera::updateFromPhysics(physx::PxScene *gScene, float deltaTime)
{
    physx::PxExtendedVec3 extendedPos = characterController->getPosition();
    position = glm::vec3(extendedPos.x, extendedPos.y, extendedPos.z);

    const float gravity = -9.81f;

    verticalVelocity += gravity * deltaTime;

    if (isJumping)
    {
        verticalVelocity += jumpImpulse;
        isJumping = false;
    }

    if (isGrounded && verticalVelocity < 0.0f)
    {
        verticalVelocity = 0.0f;
    }

    glm::vec3 movement(0.0f, verticalVelocity * deltaTime, 0.0f);
    moveWithDynamicFilters(movement, deltaTime);
}

glm::vec3 POVCamera::getForward() const
{
    return forward;
}
void POVCamera::processKeyboard(char direction, float deltaTime)
{
    movementSpeed = isSprinting ? 3.0f : 1.5f;
    float moveStep = movementSpeed * deltaTime;

    // flat forward calc
    glm::vec3 forwardFlat = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z)); // Nur x und z
    glm::vec3 rightFlat = glm::normalize(glm::cross(upward, forwardFlat));         // Nur x und z

    glm::vec3 movement(0.0f, 0.0f, 0.0f);

    // calculate movement
    if (direction == 'W')
    {
        movement += forwardFlat * moveStep;
    }
    if (direction == 'S')
    {
        movement -= forwardFlat * moveStep;
    }
    if (direction == 'A')
    {
        movement += rightFlat * moveStep;
    }
    if (direction == 'D')
    {
        movement -= rightFlat * moveStep;
    }

    if (direction == ' ' && !isJumping && (isGrounded || groundedTimer > 0.0f))
    {
        verticalVelocity = 0.0f;
        isJumping = true;
        isGrounded = false;
        groundedTimer = 0.0f;
    }

    moveWithDynamicFilters(movement, deltaTime);
}

// mouse control (view around)
void POVCamera::processMouseMovement(float xoffset, float yoffset)
{
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // restriction to prevent camera tipping over
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    updateCameraVectors();
}

void POVCamera::updateHeadBob(float deltaTime, bool isMoving)
{
    if (isMoving)
    {
        headBobActive = true;
        headBobTimer += deltaTime;
    }
    else if (headBobActive)
    {

        headBobTimer += deltaTime;

        // stop after a full sine stop
        if (headBobTimer >= (M_PI / headBobFrequency))
        {
            headBobActive = false;
            headBobTimer = 0.0f;
        }
    }
}

glm::mat4 POVCamera::getViewProjectionMatrix() const
{
    float bobOffset = 0.0f;

    if (headBobActive || headBobTimer > 0.0f)
    {
        bobOffset = sin(headBobTimer * headBobFrequency) * headBobAmplitude;
    }

    glm::vec3 bobbedPosition = position;
    bobbedPosition.y += bobOffset;

    return projection * glm::lookAt(bobbedPosition, bobbedPosition + forward, upward);
}

glm::mat4 POVCamera::getViewMatrix() const
{
    return glm::lookAt(position, position + forward, upward);
}

glm::mat4 POVCamera::getProjectionMatrix() const
{
    return projection;
}

glm::mat4 POVCamera::getViewProjNoTransforms() const
{
    glm::mat4 view = glm::mat4(glm::mat3(glm::lookAt(position, position + forward, upward)));
    return projection * view;
}

glm::vec3 POVCamera::getPosition() const
{
    return position;
}
void POVCamera::setPosition(glm::vec3 positionVec)
{
    position = positionVec;
}

glm::mat4 POVCamera::getShadowProjectionMatrix() const
{
    return glm::perspective(glm::radians(90.0f), width / height, 0.1f, 20.0f);
}

void POVCamera::moveWithDynamicFilters(const glm::vec3 &displacement, float deltaTime)
{
    // Create the filter data based on the world the player is in
    physx::PxFilterData filterData;
    if (inBloomyWorld)
    {
        filterData.word0 = WORLD_BLOOM;                // Set the world the controller belongs to
        filterData.word1 = WORLD_BLOOM | WORLD_STATIC; // Collide with Bloomy world objects and static ones
    }
    else
    {
        filterData.word0 = WORLD_DITHER;                // Set the world the controller belongs to
        filterData.word1 = WORLD_DITHER | WORLD_STATIC; // Collide with Dither world objects and static ones
    }

    // Create PxControllerFilters with the dynamically updated filter data
    physx::PxControllerFilters controllerFilters;
    controllerFilters.mFilterData = &filterData;

    // Create the custom filter callback
    // CustomFilterCallback* filterCallback = new CustomFilterCallback();
    // controllerFilters.mFilterCallback = filterCallback;  // Use the custom filter callback
    controllerFilters.mFilterFlags = physx::PxQueryFlags(physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::ePREFILTER);

    // Convert displacement to PxVec3 for movement
    physx::PxVec3 disp(displacement.x, displacement.y, displacement.z);

    // Define a minimum distance for movement to consider
    PxF32 minDist = 0.001f;

    // Use the move function with the updated filter
    PxF32 elapsedTime = deltaTime; // Time elapsed since last frame
    PxControllerCollisionFlags flags = characterController->move(disp, minDist, elapsedTime, controllerFilters);

    // the very smart isGrounded condition
    if (flags & PxControllerCollisionFlag::eCOLLISION_DOWN)
    {
        isGrounded = true;
        groundedTimer = COYOTE_TIME;
    }
    else
    {
        isGrounded = false;
        groundedTimer -= deltaTime;
        if (groundedTimer < 0.0f)
            groundedTimer = 0.0f;
    }
}