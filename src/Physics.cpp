#include "Physics.h"

static physx::PxFilterFlags WorldFilterShader(
    physx::PxFilterObjectAttributes attributes0,
    physx::PxFilterData filterData0,
    physx::PxFilterObjectAttributes attributes1,
    physx::PxFilterData filterData1,
    physx::PxPairFlags &pairFlags,
    const void *constantBlock,
    physx::PxU32 constantBlockSize)
{
    // Check if the objects should interact based on the filter data
    if ((filterData0.word0 & filterData1.word1) == 0 || (filterData1.word0 & filterData0.word1) == 0)
        return physx::PxFilterFlag::eKILL;

    // Enable both trigger events and collision events for the pair
    pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT | physx::PxPairFlag::eTRIGGER_DEFAULT; // Allow both

    std::cout << "filterData0.word0: " << filterData0.word0 << ", filterData0.word1: " << filterData0.word1 << std::endl;
    std::cout << "filterData1.word0: " << filterData1.word0 << ", filterData1.word1: " << filterData1.word1 << std::endl;
    return physx::PxFilterFlag::eDEFAULT;
}

Physics::Physics()
{
    gFoundation = nullptr;
    gPhysics = nullptr;
    gScene = nullptr;
    gDispatcher = nullptr;
    gravity = PxVec3(0.0f, -9.81f, 0.0f);
    defaultMaterial = nullptr;
}

void Physics::initPhysX()
{

    std::cout << "PhysX Version: " << PX_PHYSICS_VERSION << std::endl;
    // create foundation
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    if (!gFoundation)
    {
        std::cerr << "PhysX Foundation initialization failed!" << std::endl;
        return;
    }

    // create PhysX-Engine
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true);
    if (!gPhysics)
    {
        std::cerr << "PhysX Physics initialization failed!" << std::endl;
        return;
    }

    // create default material
    defaultMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.0f); // set default material
    if (!defaultMaterial)
    {
        std::cerr << "Error: Material could not be created!" << std::endl;
        return;
    }

    // scene setup
    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = gravity;
    gDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = gDispatcher;
    if (!sceneDesc.cpuDispatcher)
    {
        std::cerr << "Failed to create CPU Dispatcher!" << std::endl;
        return;
    }

    sceneDesc.filterShader = WorldFilterShader;

    sceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVE_ACTORS;

    gScene = gPhysics->createScene(sceneDesc);
    if (!gScene)
    {
        std::cerr << "PhysX Scene creation failed!" << std::endl;
        return;
    }

    gControllerManager = PxCreateControllerManager(*gScene);
    if (!gControllerManager)
    {
        std::cerr << "Failed to create Controller Manager!" << std::endl;
        return;
    }

    triggerListener = new PressurePlateTriggerListener();
    gScene->setSimulationEventCallback(triggerListener);

    std::cout << "PhysX initialized successfully!" << std::endl;
}

PxRigidStatic *Physics::createPlane()
{
    if (!defaultMaterial)
    {
        std::cerr << "Error: Default material not created!" << std::endl;
        return nullptr;
    }

    PxRigidStatic *groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0.0f), *defaultMaterial);

    if (groundPlane)
    {
        gScene->addActor(*groundPlane);
    }
    else
    {
        std::cerr << "Error: Plane could not be created!" << std::endl;
    }

    return groundPlane;
}

physx::PxRigidDynamic *Physics::createSphere(float radius, const glm::vec3 &position)
{

    physx::PxVec3 physxPosition(position.x, position.y, position.z);
    physx::PxTransform transform(physxPosition, physx::PxQuat(0.0f, 0.0f, 0.0f, 1.0f));

    physx::PxRigidDynamic *dynamicBody = gPhysics->createRigidDynamic(transform);
    if (!dynamicBody)
    {
        std::cerr << "Error: Rigid dynamic body could not be created!" << std::endl;
        return nullptr;
    }

    physx::PxSphereGeometry sphereGeometry(radius);

    physx::PxShape *shape = gPhysics->createShape(sphereGeometry, *defaultMaterial);
    if (!shape)
    {
        std::cerr << "Error: Shape could not be created for the sphere!" << std::endl;
        return nullptr;
    }

    dynamicBody->attachShape(*shape);
    dynamicBody->setMass(1.0f);
    gScene->addActor(*dynamicBody);

    return dynamicBody;
}

physx::PxRigidDynamic *Physics::createBox(float width, float height, float depth, const glm::vec3 &position)
{
    physx::PxVec3 physxPosition(position.x, position.y, position.z);
    physx::PxTransform transform(physxPosition, physx::PxQuat(0.0f, 0.0f, 0.0f, 1.0f));

    physx::PxRigidDynamic *dynamicBody = gPhysics->createRigidDynamic(transform);
    if (!dynamicBody)
    {
        std::cerr << "Error: Rigid dynamic body could not be created!" << std::endl;
        return nullptr;
    }

    // PhysX expects half extents for PxBoxGeometry
    physx::PxBoxGeometry boxGeometry(width * 0.5f, height * 0.5f, depth * 0.5f);

    physx::PxShape *shape = gPhysics->createShape(boxGeometry, *defaultMaterial);
    if (!shape)
    {
        std::cerr << "Error: Shape could not be created for the box!" << std::endl;
        return nullptr;
    }

    dynamicBody->attachShape(*shape);
    dynamicBody->setMass(1.0f);
    gScene->addActor(*dynamicBody);

    return dynamicBody;
}

physx::PxRigidDynamic *Physics::createCameraBody(glm::vec3 startPosition)
{

    PxMaterial *material = gPhysics->createMaterial(0.5f, 0.5f, 0.f);
    cameraBody = gPhysics->createRigidDynamic(PxTransform(PxVec3(startPosition.x, startPosition.y, startPosition.z)));
    PxShape *shape = gPhysics->createShape(PxCapsuleGeometry(0.2f, 0.6f), *material);

    PxFilterData filterData;
    filterData.word0 = WORLD_BOTH;
    filterData.word1 = WORLD_BLOOM;
    shape->setSimulationFilterData(filterData);

    cameraBody->attachShape(*shape);
    cameraBody->setMass(80.0f);
    cameraBody->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
    cameraBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
    cameraBody->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, true);
    cameraBody->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, true);
    PxRigidBodyExt::updateMassAndInertia(*cameraBody, 10.0f);

    std::cout << "cameraBody Actor Pointer: " << cameraBody << std::endl;
    gScene->addActor(*cameraBody);
    return cameraBody;
}
float Physics::getCharacterSize()
{
    return 2 * controllerDescRadius + controllerDescHeight;
}

physx::PxCapsuleController *Physics::createCharacterController(const glm::vec3 &startPosition)
{
    // Create a controller description
    physx::PxCapsuleControllerDesc controllerDesc;
    controllerDesc.setToDefault();
    controllerDesc.radius = controllerDescRadius; // Adjust based on your character size
    controllerDesc.height = controllerDescHeight; // Height of the character controller
    controllerDesc.position = physx::PxExtendedVec3(startPosition.x, startPosition.y, startPosition.z);
    controllerDesc.material = defaultMaterial;
    controllerDesc.stepOffset = 0.05f;              // How much height the controller can step up
    controllerDesc.slopeLimit = physx::PxPi / 4.0f; // Maximum slope angle the character can walk up
    controllerDesc.contactOffset = 0.1f;          // Contact offset for collision handling (generally small value)

    // Create the controller using PxControllerManager
    physx::PxController *controllerBase = gControllerManager->createController(controllerDesc);

    // Check if the creation was successful
    if (!controllerBase)
    {
        std::cerr << "Failed to create character controller!" << std::endl;
        return nullptr;
    }

    // Cast to PxCapsuleController and return
    physx::PxCapsuleController *capsuleController = static_cast<physx::PxCapsuleController *>(controllerBase);

    // Return the capsule controller
    return capsuleController;
}

physx::PxRigidActor *Physics::createMeshFromGeometry(const GeometryData &geometryData, RigidBodyType bodyType)
{
    std::vector<physx::PxVec3> pxVertices;
    for (const auto &vertex : geometryData.positions)
    {
        pxVertices.push_back(physx::PxVec3(vertex.x, vertex.y, vertex.z));
    }

    // === COMMON SETUP ===
    PxTolerancesScale scale;
    PxCookingParams cookingParams(scale);
    cookingParams.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
    cookingParams.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;

    PxTransform transform(PxVec3(0.0f, 0.0f, 0.0f));
    PxShape *shape = nullptr;
    PxRigidActor *actor = nullptr;

    if (bodyType == RigidBodyType::STATIC)
    {
        // === TRIANGLE MESH FOR STATIC ===
        std::vector<physx::PxU32> pxIndices;
        for (size_t i = 0; i < geometryData.indices.size(); i += 3)
        {
            pxIndices.push_back(geometryData.indices[i]);
            pxIndices.push_back(geometryData.indices[i + 1]);
            pxIndices.push_back(geometryData.indices[i + 2]);
        }

        PxTriangleMeshDesc meshDesc;
        meshDesc.points.count = static_cast<physx::PxU32>(pxVertices.size());
        meshDesc.points.stride = sizeof(physx::PxVec3);
        meshDesc.points.data = pxVertices.data();

        meshDesc.triangles.count = static_cast<physx::PxU32>(pxIndices.size() / 3);
        meshDesc.triangles.stride = 3 * sizeof(physx::PxU32);
        meshDesc.triangles.data = pxIndices.data();

        PxDefaultMemoryOutputStream writeBuffer;
        PxTriangleMeshCookingResult::Enum result;
        bool status = PxCookTriangleMesh(cookingParams, meshDesc, writeBuffer, &result);
        if (!status)
        {
            std::cerr << "Failed to cook the triangle mesh." << std::endl;
            return nullptr;
        }

        PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
        PxTriangleMesh *triangleMesh = gPhysics->createTriangleMesh(readBuffer);
        if (!triangleMesh)
        {
            std::cerr << "Failed to create the triangle mesh." << std::endl;
            return nullptr;
        }

        shape = gPhysics->createShape(PxTriangleMeshGeometry(triangleMesh), *defaultMaterial);
        if (!shape)
        {
            std::cerr << "Failed to create shape from triangle mesh." << std::endl;
            return nullptr;
        }

        PxRigidStatic *rigidStatic = gPhysics->createRigidStatic(transform);
        rigidStatic->attachShape(*shape);
        actor = rigidStatic;
    }
    else // DYNAMIC
    {
        // === CONVEX MESH FOR DYNAMIC ===
        PxConvexMeshDesc convexDesc;
        convexDesc.points.count = static_cast<physx::PxU32>(pxVertices.size());
        convexDesc.points.stride = sizeof(physx::PxVec3);
        convexDesc.points.data = pxVertices.data();
        convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

        PxDefaultMemoryOutputStream writeBuffer;
        bool status = PxCookConvexMesh(cookingParams, convexDesc, writeBuffer);
        if (!status)
        {
            std::cerr << "Failed to cook convex mesh." << std::endl;
            return nullptr;
        }

        PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
        PxConvexMesh *convexMesh = gPhysics->createConvexMesh(readBuffer);
        if (!convexMesh)
        {
            std::cerr << "Failed to create convex mesh." << std::endl;
            return nullptr;
        }

        shape = gPhysics->createShape(PxConvexMeshGeometry(convexMesh), *defaultMaterial);
        if (!shape)
        {
            std::cerr << "Failed to create shape from convex mesh." << std::endl;
            return nullptr;
        }

        PxRigidDynamic *rigidDynamic = gPhysics->createRigidDynamic(transform);
        rigidDynamic->attachShape(*shape);
        PxRigidBodyExt::updateMassAndInertia(*rigidDynamic, 10.0f);
        actor = rigidDynamic;
    }

    if (actor)
        gScene->addActor(*actor);

    return actor;
}

physx::PxRigidActor *Physics::createPressurePlate(const glm::vec3 &position, const glm::vec3 &size)
{
    physx::PxVec3 pxSize(size.x, size.y, size.z);
    physx::PxVec3 pxPosition(position.x, position.y, position.z);

    physx::PxRigidStatic *pressurePlateActor = gPhysics->createRigidStatic(physx::PxTransform(pxPosition));

    physx::PxFilterData pressurePlateFilterData;
    pressurePlateFilterData.word0 = WORLD_BOTH;
    pressurePlateFilterData.word1 = WORLD_REMOTE | WORLD_STATIC;

    physx::PxShape *shape = gPhysics->createShape(physx::PxBoxGeometry(pxSize), *defaultMaterial);
    shape->setSimulationFilterData(pressurePlateFilterData);
    shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true); // Mark the shape as a trigger

    pressurePlateActor->attachShape(*shape);

    gScene->addActor(*pressurePlateActor);

    pressurePlateActor->userData = (void *)"PressurePlate";

    return pressurePlateActor;
}


void Physics::createBoundingWalls(
    float minX, float maxX,
    float minZ, float maxZ,
    float wallHeight,
    float wallThickness)
{
    struct WallDef { glm::vec3 center; glm::vec3 halfExtents; };
    std::array<WallDef, 4> walls = {{
        { glm::vec3((minX+maxX)/2, wallHeight/2, maxZ + wallThickness/2),
          glm::vec3((maxX-minX)/2, wallHeight/2, wallThickness/2) },
        { glm::vec3((minX+maxX)/2, wallHeight/2, minZ - wallThickness/2),
          glm::vec3((maxX-minX)/2, wallHeight/2, wallThickness/2) },
        { glm::vec3(maxX + wallThickness/2, wallHeight/2, (minZ+maxZ)/2),
          glm::vec3(wallThickness/2, wallHeight/2, (maxZ-minZ)/2) },
        { glm::vec3(minX - wallThickness/2, wallHeight/2, (minZ+maxZ)/2),
          glm::vec3(wallThickness/2, wallHeight/2, (maxZ-minZ)/2) }
    }};

    for (auto &w : walls)
    {
        physx::PxTransform pose(
            physx::PxVec3(w.center.x, w.center.y, w.center.z)
        );
        physx::PxRigidStatic* wallActor = gPhysics->createRigidStatic(pose);
        if (!wallActor) continue;

        physx::PxBoxGeometry geom(
            w.halfExtents.x,
            w.halfExtents.y,
            w.halfExtents.z
        );
        physx::PxShape* shape = gPhysics->createShape(
            geom,
            *defaultMaterial,
            true
        );
        if (!shape) {
            wallActor->release();
            continue;
        }

        physx::PxFilterData fd;
        fd.word0 = WORLD_BOTH;
        fd.word1 = WORLD_BOTH;
        shape->setSimulationFilterData(fd);
        shape->setQueryFilterData(fd);
        shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE,    true);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE,     true);
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE,        false);

        wallActor->attachShape(*shape);
        shape->release();

        gScene->addActor(*wallActor);
    }
}

void Physics::shutdownPhysX()
{

    if (gControllerManager)
    {
        gControllerManager->purgeControllers();
        gControllerManager->release();
        gControllerManager = nullptr;
    }

    // 1) Remove & release all actors (static + dynamic) from the scene
    if (gScene)
    {
        // Fetch number of static + dynamic actors
        PxU32 nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC);
        if (nbActors > 0)
        {
            std::vector<PxActor *> actors(nbActors);
            gScene->getActors(PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC, actors.data(), nbActors);
            for (PxActor *actor : actors)
            {
                if (!actor)
                    continue;

                // If still in the scene, remove it first
                if (actor->getScene())
                    gScene->removeActor(*actor);

                // Now release the actor (which also cleans up any attached shapes)
                actor->release();
            }
        }

        // Finally release the scene itself
        gScene->release();
        gScene = nullptr;
    }

    // 2) Release the controller manager (destroys any remaining controllers)

    // 3) Release the default material
    if (defaultMaterial)
    {
        defaultMaterial->release();
        defaultMaterial = nullptr;
    }

    // 4) Delete our trigger listener
    if (triggerListener)
    {
        delete triggerListener;
        triggerListener = nullptr;
    }

    // 5) Release the CPU dispatcher
    if (gDispatcher)
    {
        gDispatcher->release();
        gDispatcher = nullptr;
    }

    // 8) Release the PhysX object
    if (gPhysics)
    {
        gPhysics->release();
        gPhysics = nullptr;
    }

    // 9) Release the Foundation
    if (gFoundation)
    {
        gFoundation->release();
        gFoundation = nullptr;
    }

    std::cerr << "[PhysX] Shutdown complete\n";
}