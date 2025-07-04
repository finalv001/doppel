#pragma once

#include "PlayerState.h"
#include "../POVCamera.h"
#include "../ObjectPicker.h"
#include "GameState.h"
#include <SFML/Audio.hpp>

class Player
{
public:
    Player(glm::vec3 startPosition, float width, float height, Physics &physics, bool &inBloomyWorld);
    void update(float deltaTime, physx::PxScene *scene);
    void handleInput(char key, float deltaTime);
    void handleMouse(float xoffset, float yoffset);
    void updateHeadBob(float deltaTime, bool isMoving);
    void setSprinting(bool sprinting);
    void showInventory() const;
    bool isInInventory(RenderObject *obj) const;
    void pickObject(double mouseX, double mouseY, int width, int height, physx::PxScene *scene);
    void tryPickupNearbyObject(RenderObject *obj, physx::PxScene *scene, const sf::SoundBuffer sound);
    glm::vec3 getPosition() const;
    float getFootPosition() const;
    glm::mat4 getViewProjectionMatrix() const;
    POVCamera &getCamera();
    physx::PxRigidDynamic *getBody() { return camera.cameraBody; };
    PlayerState &getState();
    float getHealth() const { return state.GetHealth(); }
    float getStamina() const { return state.GetStamina(); }
    bool canSprint() const { return state.CanSprint(); }
    bool isExhausted() const { return state.IsExhausted(); }
    void spendStamina(float deltaTime) { state.SpendStamina(deltaTime); }
    void registerDamage(float deltaTime, float amount, sf::SoundBuffer sound)
    {
        static float damageCooldown = 0.0f;
        const float damageInterval = 2.0f;

        damageCooldown += deltaTime;

        if (damageCooldown >= damageInterval)
        {
            damageBuffer = sound;
            activeSounds.emplace_back(damageBuffer);
            sf::Sound &s = activeSounds.back();
            s.setVolume(05.f);
            s.setRelativeToListener(true);
            s.setMinDistance(1.f);
            s.setAttenuation(0.f);
            s.play();

            damageCooldown = 0.0f;
        }

        state.RegisterDamage(deltaTime, amount);
    }
    void regenerateStamina(float deltaTime) { state.RegenerateStamina(deltaTime); }
    void setWorldCollisionMask(uint32_t worldMask, physx::PxScene *scene);
    void throwRemote();
    void setRemoteThrowable(std::shared_ptr<RenderObject> throwable);
    std::shared_ptr<RenderObject> getRemoteThrowable() { return remoteThrowable; };
    void setInBloomyWorld(bool inBloomyWorld) { camera.inBloomyWorld = inBloomyWorld; };
    float getSize();
    void cleanupFinishedSounds();

private:
    PlayerState state;
    POVCamera camera;
    std::vector<RenderObject *> inventory;
    std::shared_ptr<RenderObject> remoteThrowable;
    bool remoteAlreadyInScene = false;
    Physics *physicsRef = nullptr;
    sf::SoundBuffer buffer;
    sf::SoundBuffer damageBuffer;
    std::vector<sf::Sound> activeSounds;
};