#pragma once

#include <PxPhysicsAPI.h>
#include <PxCooking.h>
#include <glm/glm.hpp>
#include "Geometry.h"
#include "RenderObject.h"
#include <iostream>
#include "PressurePlateTriggerListener.h"
#include <PxControllerManager.h>
#include <PxCapsuleController.h>
#include <PxController.h>
#include <array>

using namespace physx;

class Physics
{
private:
    /* data */
    float controllerDescRadius = 0.15f;
    float controllerDescHeight = 0.05f;

public:
    PxFoundation *gFoundation;
    PxPhysics *gPhysics;
    PxDefaultAllocator gAllocator;
    PxDefaultErrorCallback gErrorCallback;
    PxScene *gScene;
    PxDefaultCpuDispatcher *gDispatcher = nullptr;
    PxVec3 gravity;
    PxRigidStatic *plane;
    PxRigidDynamic *cameraBody;
    PxMaterial *defaultMaterial;
    PressurePlateTriggerListener *triggerListener;
    PxControllerManager *gControllerManager = nullptr;
    PxController *characterController = nullptr;
    Physics();
    void initPhysX();
    physx::PxRigidActor *Physics::createMeshFromGeometry(const GeometryData &geometryData, RigidBodyType bodyType);
    PxRigidStatic *createPlane();
    float getCharacterSize();
    PxRigidDynamic *createCameraBody(glm::vec3 startPosition);
    PxRigidDynamic *Physics::createSphere(float radius, const glm::vec3 &position);
    PxRigidDynamic *Physics::createBox(float width, float height, float depth, const glm::vec3 &position);
    void Physics::createBoundingWalls(float minX, float maxX, float minZ, float maxZ, float wallHeight, float wallThickness);
    physx::PxRigidActor *createPressurePlate(const glm::vec3 &position, const glm::vec3 &size);
    void shutdownPhysX();
    physx::PxCapsuleController *Physics::createCharacterController(const glm::vec3 &startPosition);
};

class CustomFilterCallback : public physx::PxQueryFilterCallback
{
public:
    // This is called before the exact intersection test if the ePREFILTER flag was set.
    virtual physx::PxQueryHitType::Enum preFilter(
        const physx::PxFilterData &filterData,
        const physx::PxShape *shape,
        const physx::PxRigidActor *actor,
        physx::PxHitFlags &queryFlags) override
    {

        std::cout << "Pre-filtering: " << filterData.word0 << ", " << filterData.word1 << std::endl;

        // Example filtering logic based on world mask (filterData.word0 and word1)
        if ((filterData.word0 & filterData.word1) == 0)
        {
            // If no intersection in the world, ignore this object (no collision)
            return physx::PxQueryHitType::eNONE;
        }

        // If conditions are met, let it block or touch
        queryFlags = physx::PxHitFlag::eMODIFIABLE_FLAGS;

        return physx::PxQueryHitType::eBLOCK; // Allow blocking the object (default behavior)
    }

    // This is called after the intersection test if the ePOSTFILTER flag was set.
    virtual physx::PxQueryHitType::Enum postFilter(
        const physx::PxFilterData &filterData,
        const physx::PxQueryHit &hit,
        const physx::PxShape *shape,
        const physx::PxRigidActor *actor) override
    {

        // After a hit, you can further refine your filtering logic here.
        // This is just an example; you can customize it as per your requirements.

        // If you're using multiple world filters, you can implement custom logic based on hit data.
        return physx::PxQueryHitType::eBLOCK;
    }
};