#include <physx/PxPhysicsAPI.h>
#include "GameLogic/GameState.h"
#include <iostream>

class PressurePlateTriggerListener : public physx::PxSimulationEventCallback
{
public:
    PressurePlateTriggerListener() = default;
    virtual ~PressurePlateTriggerListener() = default;

    // Called when a breakable constraint breaks
    virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override
    {
        
    }

    // Called with the actors which have just been woken up
    virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override
    {
       
    }

    // Called with the actors which have just been put to sleep
    virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override
    {

    }

    // Called when certain contact events occur
    virtual void onContact(const physx::PxContactPairHeader& pairHeader,
                           const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override
    {

        // Loop through all contact pairs
        for (physx::PxU32 i = 0; i < nbPairs; ++i)
        {
            const physx::PxContactPair& pair = pairs[i];

            // Check if the pair has touch notifications (entry or exit)
            if (pair.flags.isSet(physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)) // When the objects touch (entry)
            {


                // Check if one of the colliders is the pressure plate and the other is the remote
                if ((pairHeader.actors[0]->userData == (void*)"PressurePlate" && pairHeader.actors[1]->userData == (void*)"Remote") ||
                    (pairHeader.actors[1]->userData == (void*)"PressurePlate" && pairHeader.actors[0]->userData == (void*)"Remote"))
                {
                    std::cout << "Pressure Plate and Remote are in contact!" << std::endl;
                    triggerWinScreen();  // Trigger the win screen
                } 
            }

            if (pair.flags.isSet(physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)) // When the objects stop touching (exit)
            {
                // Check if one of the colliders is the pressure plate and the other is the remote
                if ((pairHeader.actors[0]->userData == (void*)"PressurePlate" && pairHeader.actors[1]->userData == (void*)"Remote") ||
                    (pairHeader.actors[1]->userData == (void*)"PressurePlate" && pairHeader.actors[0]->userData == (void*)"Remote"))
                {
                    std::cout << "Pressure Plate and Remote are no longer in contact!" << std::endl;
                }
            }
        }
    }

    // Called with the current trigger pair events
    virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override
    {

        // Loop through all the trigger pairs
        for (physx::PxU32 i = 0; i < count; ++i)
        {
            // Get the trigger shape and the other actor from the pair
            physx::PxShape* triggerShape = pairs[i].triggerShape;
            physx::PxActor* otherActor = pairs[i].otherActor;
    
            // Check if the status indicates a touch event (entry or exit)
            if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND || pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
            {
                // Check if the trigger is the pressure plate and the other actor is the remote
                if (triggerShape->getActor()->userData == (void*)"PressurePlate" &&
                    otherActor->userData == (void*)"Remote")
                {
                    if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
                    {
                        std::cout << "Pressure Plate Triggered! Remote is on the plate." << std::endl;
                        g_GameState = GameState::Won;
                    }
                    else if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
                    {
                        std::cout << "Remote has left the pressure plate." << std::endl;
                    }
                }
            }  else {
                std::cout << "notify faulty" << std::endl;
            }
        }
    }

    // Provides early access to the new pose of moving rigid bodies
    virtual void onAdvance(const physx::PxRigidBody* const* bodies, const physx::PxTransform* transforms,
                           const physx::PxU32 count) override
    {
    }

private:
    void triggerWinScreen()
    {
        g_GameState = GameState::Won;
    }
};
