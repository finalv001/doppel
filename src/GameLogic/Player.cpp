#include "Player.h"

Player::Player(glm::vec3 startPosition, float width, float height, Physics &physics, bool &inBloomyWorld)
    : state(inBloomyWorld), physicsRef(&physics)
{
    camera = POVCamera(startPosition, width, height, physics);
}

void Player::update(float deltaTime, physx::PxScene *scene)
{
    state.Update(deltaTime);
    camera.updateFromPhysics(scene, deltaTime);
    auto currentPos = camera.getPosition();
    bool isInBloomyWorld = state.IsInBloomyWorld();

    // float borderXPos = 35.0f;
    // float borderXNeg = -8.0f;
    // float borderZPos = 35.0f;
    // float borderZNeg = -15.0f;
    // currentPos.x = std::clamp(currentPos.x, borderXNeg, borderXPos);
    // currentPos.z = std::clamp(currentPos.z, borderZNeg, borderZPos);
    camera.setPosition(currentPos);
    cleanupFinishedSounds();
}

void Player::handleInput(char key, float deltaTime)
{
    if (key == 'W' && state.CanSprint() && camera.isSprinting)
    {
        state.SpendStamina(deltaTime);
    }

    camera.processKeyboard(key, deltaTime);
}

void Player::handleMouse(float xoffset, float yoffset)
{
    camera.processMouseMovement(xoffset, yoffset);
}

void Player::updateHeadBob(float deltaTime, bool isMoving)
{
    camera.updateHeadBob(deltaTime, isMoving);
}

void Player::setSprinting(bool sprinting)
{
    camera.setSprinting(sprinting);
}

void Player::setRemoteThrowable(std::shared_ptr<RenderObject> throwable)
{
    remoteThrowable = throwable;
    remoteThrowable->setAsPickable();
    remoteThrowable->isRendered = false;
    remoteThrowable->id = "remote";
    remoteThrowable->setCollisionFilter(WORLD_REMOTE, WORLD_STATIC);
    remoteThrowable->getRigidActor()->userData = (void *)"Remote";

    if (remoteThrowable->dynamicBody)
    {

        physicsRef->gScene->removeActor(*remoteThrowable->dynamicBody);
        remoteAlreadyInScene = false;
    }
}

void Player::pickObject(double mouseX, double mouseY, int width, int height, physx::PxScene *scene)
{
    {
        RenderObject *obj = ObjectPicker::pickObject(mouseX, mouseY, width, height, camera, scene);

        std::cerr << "[PickObject] hit obj=" << obj;

        if (obj)
        {
            std::cerr
                << " isPickable=" << obj->isPickable
                << " inInventory=" << isInInventory(obj)
                << "\n";
        }
        else
        {
            std::cerr << " (no object)\n";
        }

        if (obj && obj->isPickable == true && !isInInventory(obj))
        {
            if (obj->dynamicBody)
            {
                scene->removeActor(*obj->dynamicBody);
            }
            else if (obj->staticBody)
            {
                scene->removeActor(*obj->staticBody);
            }
            inventory.push_back(obj);
            obj->isRendered = false;
            std::cerr << "Picked up object!" << std::endl;
        }
        else
        {
            std::cerr << "No pickable object found!" << std::endl;
        }
    }
}

void Player::tryPickupNearbyObject(RenderObject *obj, physx::PxScene *scene, const sf::SoundBuffer sound)
{
    if (!obj || isInInventory(obj) || !obj->isPickable || !obj->isRendered)
        return;

    float distance = glm::distance(getPosition(), obj->geometry->getPosition());
    if (distance > 0.5f)
        return;

    if (obj->dynamicBody)
        scene->removeActor(*obj->dynamicBody);
    else if (obj->staticBody)
        scene->removeActor(*obj->staticBody);

    inventory.push_back(obj);
    obj->isRendered = false;
    if (obj->id == "remote")
    {
        buffer = sound;

        activeSounds.emplace_back(buffer);
        sf::Sound &s = activeSounds.back();
        s.setVolume(30.f);
        s.setRelativeToListener(true);
        s.setMinDistance(1.f);
        s.setAttenuation(0.f);
        s.play();

        state.setRemoteInInventory(true);

        if (obj == remoteThrowable.get())
        {
            obj->isRendered = false;
            remoteAlreadyInScene = false;
        }
    }

    if (obj->id == "note")
    {
        buffer = sound;

        activeSounds.emplace_back(buffer);
        sf::Sound &s = activeSounds.back();
        s.setVolume(30.f);
        s.setRelativeToListener(true);
        s.setMinDistance(1.f);
        s.setAttenuation(0.f);
        s.play();

        state.setNoteInInventory(true);

        if (obj == remoteThrowable.get())
        {
            obj->isRendered = false;
            remoteAlreadyInScene = false;
        }
    }

    std::cerr << "Picked up nearby object!" << std::endl;
}

void Player::throwRemote()
{
    if (!remoteThrowable || !state.IsRemoteInInventory())
        return;

    glm::vec3 forward = glm::normalize(camera.forward);

    glm::vec3 throwPos = getPosition() + forward * 0.21f;

    glm::vec3 throwVel = forward * 2.0f + glm::vec3(0, 0.5f, 0);

    uint32_t activeWorldMask = state.IsInBloomyWorld() ? WORLD_BLOOM : WORLD_DITHER;
    remoteThrowable->setCollisionFilter(WORLD_REMOTE, WORLD_STATIC | activeWorldMask);
    remoteThrowable->dynamicBody->setGlobalPose(PxTransform(PxVec3(throwPos.x, throwPos.y, throwPos.z)));
    remoteThrowable->dynamicBody->setLinearVelocity(PxVec3(throwVel.x, throwVel.y, throwVel.z));
    remoteThrowable->isRendered = true;
    remoteThrowable->setAsPickable();

    auto it = std::find(inventory.begin(), inventory.end(), remoteThrowable.get());
    if (it != inventory.end())
        inventory.erase(it);

    state.setRemoteInInventory(false);

    if (!remoteAlreadyInScene)
    {
        physicsRef->gScene->addActor(*remoteThrowable->dynamicBody);
        remoteAlreadyInScene = true;
    }
}
glm::vec3 Player::getPosition() const
{
    return camera.getPosition();
}

float Player::getFootPosition() const
{
    return camera.getFootPos() - 0.01f;
}

float Player::getSize()
{
    return physicsRef->getCharacterSize();
}

glm::mat4 Player::getViewProjectionMatrix() const
{
    return camera.getViewProjectionMatrix();
}

POVCamera &Player::getCamera()
{
    return camera;
}

PlayerState &Player::getState()
{
    return state;
}

void Player::showInventory() const
{
    std::cout << "\n========== INVENTORY ==========\n";

    if (inventory.empty())
    {
        std::cout << "Inventory is empty.\n";
    }
    else
    {
        int index = 1;
        for (const auto &obj : inventory)
        {
            std::cout << "[" << index++ << "] "
                      << " | Object Ptr: " << obj
                      << "\n";
        }
    }

    std::cout << "===============================\n";
}

bool Player::isInInventory(RenderObject *obj) const
{
    return std::find(inventory.begin(), inventory.end(), obj) != inventory.end();
}

void Player::setWorldCollisionMask(uint32_t worldMask, physx::PxScene *scene)
{
    physx::PxRigidDynamic *body = camera.cameraBody;
    if (!body)
        return;

    physx::PxShape *shapes[8];
    uint32_t fetchedShapeCount = body->getShapes(shapes, 8);

    for (uint32_t i = 0; i < fetchedShapeCount; ++i)
    {
        physx::PxFilterData filterData;
        filterData.word0 = worldMask;
        filterData.word1 = worldMask | WORLD_STATIC;
        ;
        shapes[i]->setSimulationFilterData(filterData);
    }
    body->wakeUp();
    scene->resetFiltering(*body);
}

void Player::cleanupFinishedSounds()
{
    activeSounds.erase(
        std::remove_if(
            activeSounds.begin(),
            activeSounds.end(),
            [](const sf::Sound &s)
            {
                return s.getStatus() == sf::SoundSource::Status::Stopped;
            }),
        activeSounds.end());
}