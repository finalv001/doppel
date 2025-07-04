#include "PlayerState.h"

PlayerState::PlayerState(bool &inBloomyWorld)
    : inBloomyWorld{inBloomyWorld},
      health(100.0f),
      remoteCharge(100.0f),
      stamina(100.0f),
      maxHealth(100.0f),
      maxStamina(100.0f), maxRemoteCharge(100.0f)
{
}

void PlayerState::Update(float deltaTime)
{
    RegenerateStamina(deltaTime);
    exhausted = stamina <= 0.0f;
}

void PlayerState::SpendStamina(float deltaTime)
{
    stamina -= staminaDrainRate * deltaTime;
    if (stamina < 0.0f)
        stamina = 0.0f;
}

void PlayerState::DrainRemote(float deltaTime)
{
    static float remoteCooldown = 0.0f;
    const float remoteInterval = 2.0f;

    remoteCooldown += deltaTime;

    if (remoteCooldown >= remoteInterval)
    {
        remoteCharge -= 5.0f;
        if (remoteCharge < 0.0f)
            remoteCharge = 0.0f;

        remoteCooldown = 0.0f;
    }
}

void PlayerState::RegisterDamage(float deltaTime, float amount)
{
    static float damageCooldown = 0.0f;
    const float damageInterval = 2.0f;

    damageCooldown += deltaTime;

    if (damageCooldown >= damageInterval)
    {
        health -= amount;
        if (health < 0.0f)
            health = 0.0f;

        damageCooldown = 0.0f;
    }
}

void PlayerState::Heal(float amount)
{
    health += amount;
    if (health >= maxHealth)
        health = maxHealth;
}

void PlayerState::RegenerateStamina(float deltaTime)
{
    if (!exhausted)
    {
        stamina += staminaRegenRate * deltaTime;
        if (stamina > maxStamina)
            stamina = maxStamina;
    }
}

void PlayerState::ChargeRemote(float deltaTime)
{
    remoteCharge += remoteChargeRegenRate * deltaTime;
    if (remoteCharge > maxRemoteCharge)
        remoteCharge = maxRemoteCharge;
}

bool PlayerState::CanSprint() const
{
    return stamina > 5.0f;
}

bool PlayerState::IsExhausted() const
{
    return exhausted;
}
