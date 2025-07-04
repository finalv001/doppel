#pragma once
#include <SFML/Audio.hpp>

class PlayerState
{
public:
    PlayerState(bool &inBloomyWorld);

    void Update(float deltaTime);

    void SpendStamina(float deltaTime);
    void RegisterDamage(float deltaTime, float amount);
    void DrainRemote(float deltaTime);

    void Heal(float amount);
    void RegenerateStamina(float deltaTime);
    void ChargeRemote(float deltaTime);

    bool CanSprint() const;
    bool IsExhausted() const;

    float GetHealth() const { return health; }
    float GetStamina() const { return stamina; }
    float GetRemoteCharge() const { return remoteCharge; }
    bool IsInBloomyWorld() const { return inBloomyWorld; }
    bool IsRemoteInInventory() const { return remoteInInventory; }
    bool IsNoteInInventory() const { return noteInInventory; }

    void setNoteInInventory(bool isNoteInInventory) { noteInInventory = isNoteInInventory; };
    void setRemoteInInventory(bool isRemoteInInventory) { remoteInInventory = isRemoteInInventory; };
    void fullRemoteCharge() { remoteCharge = maxRemoteCharge; };

private:
    float timeSinceLastDamage = 0.0f;
    bool remoteInInventory = false;
    bool noteInInventory = false;
    bool &inBloomyWorld;
    float health;
    float stamina;
    float remoteCharge;
    float maxHealth;
    float maxStamina;
    float maxRemoteCharge;

    float remoteChargeRegenRate = 50.0f;
    float staminaRegenRate = 20.0f;
    float staminaDrainRate = 30.0f;

    bool exhausted = false;
};
