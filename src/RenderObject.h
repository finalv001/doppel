#pragma once
#include <memory>
#include <PxPhysicsAPI.h>
#include "Geometry.h"
#include <string>

// Enum für die Typen der Rigid Bodies
enum class RigidBodyType
{
    STATIC,
    DYNAMIC,
    NONE
};

struct RenderObject
{
    std::string id;
    std::shared_ptr<Geometry> geometry;
    physx::PxRigidDynamic *dynamicBody;
    physx::PxRigidStatic *staticBody;
    RigidBodyType bodyType;
    bool isPickable = false;
    bool isRendered = true;
    void RenderObject::setTransform(const glm::vec3 &position, const glm::vec3 &forward, bool spin);
    // Konstruktor für das RenderObject
    RenderObject(std::shared_ptr<Geometry> geom, physx::PxRigidDynamic *body)
        : geometry(geom), dynamicBody(body), staticBody(nullptr), bodyType(RigidBodyType::DYNAMIC), isPickable(false), isRendered(true)
    {
        if (!dynamicBody)
        {
            std::cerr << "Error: dynamicBody is nullptr!" << std::endl;
        }
        else
        {
            dynamicBody->userData = this;
            setCollisionFilter(geom->getWorldMask(), geom->getWorldMask() | WORLD_REMOTE);
        }
    }

    RenderObject(std::shared_ptr<Geometry> geom, physx::PxRigidStatic *body)
        : geometry(geom), dynamicBody(nullptr), staticBody(body), bodyType(RigidBodyType::STATIC), isPickable(false), isRendered(true)
    {
        if (!staticBody)
        {
            std::cerr << "Error: dynamicBody is nullptr!" << std::endl;
        }
        else
        {
            staticBody->userData = this;
            setCollisionFilter(geom->getWorldMask(), geom->getWorldMask() | WORLD_REMOTE);
        }
    }

    RenderObject(std::shared_ptr<Geometry> geom)
        : geometry(geom), dynamicBody(nullptr), staticBody(nullptr), bodyType(RigidBodyType::NONE), isRendered(true)
    {
    }

    RigidBodyType getBodyType() const
    {
        return bodyType;
    }

    void setBodyType(RigidBodyType type)
    {
        bodyType = type;
    }

    void updateTransform();

    void setPosition(const glm::vec3 &position);

    void setIdentifier(std::string name);

    void setAsPickable();

    physx::PxRigidActor *getRigidActor() const
    {
        return dynamicBody  ? static_cast<physx::PxRigidActor *>(dynamicBody)
               : staticBody ? static_cast<physx::PxRigidActor *>(staticBody)
                            : nullptr;
    }

    void setCollisionFilter(uint32_t group, uint32_t mask)
    {
        physx::PxFilterData filterData;
        filterData.word0 = group; // word0 is the spaces the body is in
        filterData.word1 = mask;  // word1 is the spaces the body collides with

        physx::PxShape *shapeBuffer[8];
        uint32_t shapeCount = 0;

        if (dynamicBody)
            shapeCount = dynamicBody->getShapes(shapeBuffer, 8);
        else if (staticBody)
            shapeCount = staticBody->getShapes(shapeBuffer, 8);

        for (uint32_t i = 0; i < shapeCount; ++i)
        {
            if (shapeBuffer[i])
                shapeBuffer[i]->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
            shapeBuffer[i]->setSimulationFilterData(filterData);
            shapeBuffer[i]->setQueryFilterData(filterData);
        }
    }

private:
    void setCollisionFilter(uint32_t worldMask)
    {
        physx::PxFilterData filterData;
        filterData.word0 = worldMask; // word0 is the spaces the body is in
        filterData.word1 = worldMask; // word1 is the spaces the body collides with

        if (dynamicBody)
        {
            physx::PxShape *shapeBuffer[8];
            uint32_t shapeCount = dynamicBody->getShapes(shapeBuffer, 8);
            if (shapeCount == 0)
                return;

            for (uint32_t i = 0; i < shapeCount; ++i)
            {
                if (shapeBuffer[i])
                    shapeBuffer[i]->setSimulationFilterData(filterData);
            }
        }
        else if (staticBody)
        {
            physx::PxShape *shapeBuffer[8];
            uint32_t shapeCount = staticBody->getShapes(shapeBuffer, 8);
            if (shapeCount == 0)
                return;

            for (uint32_t i = 0; i < shapeCount; ++i)
            {
                if (shapeBuffer[i])
                    shapeBuffer[i]->setSimulationFilterData(filterData);
            }
        }
    }
};
