#include "Game.h"
#include "../Render/ShadowPass.h"
#include "../Render/BasePass.h"

Game::Game(GLFWwindow *window)
    : interaction(false),
      show_controls_guide(false),
      firstMouse(true),
      physics(), window(window),
      transitionActive(false), transitionTimer(0.0f), should_transition(false)
{
    glfwGetFramebufferSize(window, &window_width, &window_height);

    // Initialize physX Scene
    physics.initPhysX();

    lastX = window_width / 2.0f;
    lastY = window_height / 2.0f;

    // Create shaders
    std::shared_ptr<Shader> textureShader = std::make_shared<Shader>("assets/shaders/texture.vert", "assets/shaders/texture.frag");
    std::shared_ptr<Shader> simpleColorShader = std::make_shared<Shader>("assets/shaders/simpleColor.vert", "assets/shaders/simpleColor.frag");
    ditherShader = std::make_shared<Shader>("assets/shaders/orderedDither.vert", "assets/shaders/orderedDither.frag");
    std::shared_ptr<Shader> waterShader = std::make_shared<Shader>("assets/shaders/water.vert", "assets/shaders/water.frag");
    std::shared_ptr<Shader> planeShader = std::make_shared<Shader>("assets/shaders/infPlane.vert", "assets/shaders/infPlane.frag");
    std::shared_ptr<Shader> shadowShader = std::make_shared<Shader>("assets/shaders/shadow.vert", "assets/shaders/shadow.frag");
    std::shared_ptr<Shader> textureNormalShader = std::make_shared<Shader>("assets/shaders/textureNormal.vert", "assets/shaders/textureNormal.frag");
    std::shared_ptr<Shader> textureNormalBloomShader = std::make_shared<Shader>("assets/shaders/textureNormalBloom.vert", "assets/shaders/textureNormalBloom.frag");

    transitionShader = std::make_shared<Shader>("assets/shaders/transition.vert", "assets/shaders/transition.frag");

    shaders.push_back(shadowShader);
    shaders.push_back(simpleColorShader);
    shaders.push_back(textureNormalShader);
    shaders.push_back(textureNormalBloomShader);
    shaders.push_back(waterShader);
    shaders.push_back(ditherShader);
    shaders.push_back(planeShader);

    // Create textures
    std::shared_ptr<Texture> remoteTexture = std::make_shared<Texture>("assets/textures/remote_diffuse.dds");
    std::shared_ptr<Texture> remoteNormal = std::make_shared<Texture>("assets/textures/remote_normal.dds");
    std::shared_ptr<Texture> concreteTexture = std::make_shared<Texture>("assets/textures/Concrete044B_4K-PNG_Color.dds");
    std::shared_ptr<Texture> concreteNormalMapTexture = std::make_shared<Texture>("assets/textures/Concrete044B_4K-PNG_NormalGL.dds");
    std::shared_ptr<Texture> noteDiffuse = std::make_shared<Texture>("assets/textures/note_diffuse.dds");
    // Colors
    glm::vec3 redColor = glm::vec3(181.0f / 255.0f, 27.0f / 255.0f, 0 / 255.0f);
    glm::vec3 whiteColor = glm::vec3(0.0f);
    glm::vec3 blackColor = glm::vec3(1.0f);
    glm::vec3 pinkColor = glm::vec3(162.0f / 255.0f, 129.0f / 255.0f, 160 / 255.0f);
    glm::vec3 blueColor = glm::vec3(151.0f / 255.0f, 154.0f / 255.0f, 187 / 255.0f);

    // Create materials
    std::shared_ptr<Material> remoteTextureMaterial = std::make_shared<TextureMaterial>(textureNormalBloomShader, glm::vec3(0.1f, 0.7f, 1.0f), 5.0f, remoteTexture, remoteNormal);
    std::shared_ptr<Material> ditherMaterial = std::make_shared<Material>(ditherShader, glm::vec3(0.2f, 0.7f, 0.9f), glm::vec3(0.3f, 0.8f, 1.0f), 4.0f);
    std::shared_ptr<Material> ditherFloorMaterial = std::make_shared<Material>(planeShader, glm::vec3(0.95f, 0.95, 0.95f), glm::vec3(0.0f, 0.5f, 0.0f), 1.0f);
    std::shared_ptr<Material> simpleGreyColorMaterial = std::make_shared<Material>(simpleColorShader, glm::vec3(0.95f, 0.95, 0.95f), glm::vec3(0.3f, 0.9f, 0.3f), 2.0f);
    std::shared_ptr<Material> planeMaterial = std::make_shared<Material>(planeShader, glm::vec3(0.95f, 0.95, 0.95f), glm::vec3(0.5f, 0.5f, 0.0f), 1.0f);
    std::shared_ptr<Material> simpleRedColorMaterial = std::make_shared<Material>(simpleColorShader, redColor, glm::vec3(0.5f, 0.5f, 0.0f), 1.0f);
    std::shared_ptr<Material> waterMaterial = std::make_shared<Material>(waterShader, glm::vec3(0.95f, 0.95, 0.95f), glm::vec3(0.5f, 0.5f, 0.0f), 1.0f);
    std::shared_ptr<Material> concreteMaterial = std::make_shared<TextureMaterial>(textureNormalShader, glm::vec3(1.0f, 0.9f, 0.6f), 8.0f, concreteTexture, concreteNormalMapTexture);
    std::shared_ptr<Material> noteMaterialBloomy = std::make_shared<TextureMaterial>(textureNormalBloomShader, glm::vec3(1.0f, 0.1f, 0.0f), 20.0f, noteDiffuse);
    std::shared_ptr<Material> noteMaterial = std::make_shared<TextureMaterial>(textureNormalShader, glm::vec3(1.0f, 0.2f, 0.0f), 20.0f, noteDiffuse);

    // Create geometries
    GLTFLoader loader(physics);

    auto envStructure = loader.loadModel("assets/models/env.glb", simpleGreyColorMaterial, ditherMaterial, WORLD_BOTH, RigidBodyType::STATIC);
    renderObjects.push_back(envStructure);

    ditherWaterFloor = loader.loadModel("assets/models/dither_water.glb", nullptr, waterMaterial, WORLD_DITHER, RigidBodyType::NONE);
    renderObjects.push_back(ditherWaterFloor);

    auto ditherFloor = loader.loadModel("assets/models/floor_dither.glb", nullptr, ditherFloorMaterial, WORLD_DITHER, RigidBodyType::STATIC);
    ditherFloor->id = "floor";
    renderObjects.push_back(ditherFloor);

    auto bloomyFloor = loader.loadModel("assets/models/floor_bloomy.glb", planeMaterial, ditherMaterial, WORLD_BLOOM, RigidBodyType::STATIC);
    bloomyFloor->id = "floor";
    renderObjects.push_back(bloomyFloor);

    bloomyWaterFloor = loader.loadModel("assets/models/bloomy_water.glb", ditherMaterial, ditherMaterial, WORLD_BLOOM, RigidBodyType::NONE);
    renderObjects.push_back(bloomyWaterFloor);

    auto jumpnrun_dither = loader.loadModel("assets/models/jumpnrun_dither.glb", nullptr, ditherMaterial, WORLD_DITHER, RigidBodyType::STATIC);
    renderObjects.push_back(jumpnrun_dither);

    auto jumpnrun_bloomy = loader.loadModel("assets/models/jumpnrun_bloomy.glb", concreteMaterial, ditherMaterial, WORLD_BLOOM, RigidBodyType::STATIC);
    renderObjects.push_back(jumpnrun_bloomy);

    note = loader.loadModel("assets/models/note.glb", noteMaterialBloomy, noteMaterialBloomy, WORLD_BLOOM, RigidBodyType::STATIC);
    note->setPosition(notePosition);
    note->setAsPickable();
    note->id = "note";
    renderObjects.push_back(note);

    closeUpNote = loader.loadModel("assets/models/note_closeup.glb", noteMaterial, noteMaterial, WORLD_BLOOM, RigidBodyType::NONE);
    closeUpNote->isRendered = false;

    renderObjects.push_back(closeUpNote);

    auto stoneAltar = loader.loadModel("assets/models/altar.glb", ditherMaterial, ditherMaterial, WORLD_BLOOM, RigidBodyType::STATIC);
    stoneAltar->setPosition(glm::vec3(remotePosition.x, 0.0f, remotePosition.y));
    renderObjects.push_back(stoneAltar);

    remote = loader.loadModel("assets/models/remote_static.glb", remoteTextureMaterial, simpleGreyColorMaterial, WORLD_BLOOM, RigidBodyType::NONE);
    remote->setPosition(remotePosition);
    remote->setAsPickable();

    remote->id = "remote";
    renderObjects.push_back(remote);

    // Initialize Player
    player = std::make_unique<Player>(
        glm::vec3(3.0f, 1.0f, -1.0f), // camera postion inititalization
        // notePosition + glm::vec3(0.0f, 0.0f, -1.0f),
        static_cast<float>(window_width),
        static_cast<float>(window_height),
        physics, in_bloomy_world);

    // Create Pressure Plates
    physics.createPressurePlate(glm::vec3(29.0, -10.0, 17.0), glm::vec3(10.0, 0.5, 10.0));
    physics.createBoundingWalls(
        /*minX=*/-8.0f, /*maxX=*/35.0f,
        /*minZ=*/-15.0f, /*maxZ=*/35.0f,
        /*wallHeight=*/20.0f,
        /*wallThickness=*/0.1f);

    // Render passes
    shadowPass = std::make_unique<ShadowPass>(shadowShader.get(), 10000, 10000, renderObjects, in_bloomy_world);
    basePass = std::make_unique<BasePass>(window_width, window_height, renderObjects, player.get(), in_bloomy_world, underwater);
    // Initialize lights
    dirL = DirectionalLight(glm::vec3(0.8f), glm::vec3(0.0f, -1.0f, -1.0f));
    pointL = PointLight(glm::vec3(1), glm::vec3(0.0f, 0.1f, 0.0f), glm::vec3(1.0f, 8.0f, 8.0f));

    // Render loop setup
    t = float(glfwGetTime());
    dt = 0.0f;
    t_sum = 0.0f;
    lastTime = t;
    double mouse_x, mouse_y;

    player->setRemoteThrowable(loader.loadModel(
        "assets/models/remote_static.glb",
        remoteTextureMaterial,
        ditherMaterial,
        WORLD_BOTH,
        RigidBodyType::DYNAMIC));
    renderObjects.push_back(player->getRemoteThrowable());

    hud = std::make_unique<HeadsUpDisplay>(&player->getState(), &show_controls_guide, window_width, window_height);

    std::string rel = "assets/sound/Calmed_Sub.wav";
    std::string abs = gcgFindFileInParentDir(rel.c_str());

    if (!music.openFromFile(abs))
    {
        std::cerr << "Failed to load music “" << abs << "”\n";
    }
    else
    {
        music.setLooping(true); // keep looping forever
        music.play();           // start once; it will stream in the background
    }

    std::string relE = "assets/sound/PickUpRemote.wav";
    std::string absE = gcgFindFileInParentDir(relE.c_str());
    if (!pickUpRemoteSound.loadFromFile(absE))
    {
        std::cerr << "Failed to load “" << absE << "” into pickUpSoundBuffer\n";
    }

    relE = "assets/sound/ChangeWorld.wav";
    absE = gcgFindFileInParentDir(relE.c_str());
    if (!switchWorldSound.loadFromFile(absE))
    {
        std::cerr << "Failed to load “" << absE << "” into switchWorldSound\n";
    }
    else
    {
        switchWorldPlayer = std::make_unique<sf::Sound>(switchWorldSound);
        switchWorldPlayer->setVolume(100.f);
        switchWorldPlayer->setRelativeToListener(true);
        switchWorldPlayer->setAttenuation(0.f);
    }

    relE = "assets/sound/Damage.wav";
    absE = gcgFindFileInParentDir(relE.c_str());
    if (!damageSound.loadFromFile(absE))
    {
        std::cerr << "Failed to load “" << absE << "” into damageSoundBuffer\n";
    }
}

void Game::Shutdown()
{
    renderObjects.clear();

    hud = nullptr;
    player = nullptr;
    shadowPass = nullptr;
    basePass = nullptr;
    transitionShader = nullptr;
    shaders.clear();

    std::cerr << ">>> physX shutdown START\n";
    physics.shutdownPhysX();
    std::cerr << ">>> physX shutdown END\n";
}

void Game::Run()
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    /*--FRAME TIMING--**/
    dt = t;
    t = float(glfwGetTime());
    dt = t - lastTime;
    lastTime = t;
    t_sum += dt;

    /*--ANIMATING OBJECTS--*/
    if (remote && remote->isRendered && !player->getState().IsRemoteInInventory())
    {
        float yOffset = sin(t * 2.f) * 0.04f;
        remote->setPosition(remotePosition + glm::vec3(0.0f, yOffset, 0.0f));
    }
    if (note && note->isRendered && !player->getState().IsNoteInInventory())
    {
        float yOffset = sin(t * 2.f) * 0.04f;
        note->setPosition(notePosition + glm::vec3(0.0f, yOffset, 0.0f));
    }

    float waterSpeed = 0.0002f;
    bloomyWaterFloor->setPosition(bloomyWaterFloor->geometry->getPosition() + glm::vec3(0.0f, waterSpeed, 0.0f));

    pointL.position = player->getPosition();

    /*--RENDERING--**/
    for (std::shared_ptr<Shader> shader : shaders)
    {
        setPerFrameUniforms(shader.get(), camera, dirL, pointL);
    }
    shadowPass->Execute();

    ditherShader->use();
    ditherShader->setUniform("drawBloom", (int)in_bloomy_world);
    basePass->Execute();

    /*--TRANSITION--**/
    if (transitionActive)
    {
        transitionTimer += dt;

        if (transitionTimer >= 0.25 && should_transition)
        {
            in_bloomy_world = !in_bloomy_world;
            if (in_bloomy_world)
                player->getState().Heal(30.0f);

            player->setInBloomyWorld(in_bloomy_world);
            should_transition = false;
        }

        float alpha = 0.0f;
        if (transitionTimer < 0.25f)
            alpha = transitionTimer * 4.0f;
        else
            alpha = (1.0f - transitionTimer) * 2.0f;

        drawFullScreenQuadWithAlpha(alpha);

        if (transitionTimer >= 1.0f)
            transitionActive = false;
    }

    /*--HUD--**/
    bool shouldShowInstruction = false;

    if (player->getState().IsRemoteInInventory() && !remoteCloseUpShown && !remoteTimerStarted)
    {
        remote->isRendered = true;
        remoteTimerStarted = true;
        remoteDisplayStartTime = t;
    }

    if (player->getState().IsRemoteInInventory() && remoteTimerStarted)
    {
        glm::vec3 camPos = player->getCamera().getPosition();
        glm::vec3 camFront = glm::normalize(player->getCamera().getForward());
        glm::vec3 camRight = glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 camUp = glm::normalize(glm::cross(camRight, camFront));

        float distanceInFront = 0.2f;
        float sideOffset = 0.0f;
        float verticalOffset = 0.0f;

        glm::vec3 remotePos = camPos + camFront * distanceInFront + camRight * sideOffset + camUp * verticalOffset;
        hud->SetInstructionText("Picked up remote. Press [LMB] to use");
        shouldShowInstruction = true;
        remote->setTransform(remotePos, camFront, true);
    }
    if (remoteTimerStarted && (t - remoteDisplayStartTime >= remoteDisplayDuration))
    {
        remote->isRendered = false;
        remoteCloseUpShown = true;
        remoteTimerStarted = false;
    }
    if (remote && remote->isRendered && !remoteTimerStarted)
    {
        float distance = glm::distance(remote->geometry->getPosition(), player->getPosition());
        if (distance < 0.5f)
        {
            hud->SetInstructionText("Press [E] to pick up the remote");
            shouldShowInstruction = true;
        }
    }

    // note
    if (player->getState().IsNoteInInventory() && !noteCloseUpShown && !noteTimerStarted)
    {
        closeUpNote->isRendered = true;
        noteTimerStarted = true;
        noteDisplayStartTime = t;
    }

    if (player->getState().IsNoteInInventory() && noteTimerStarted)
    {
        glm::vec3 camPos = player->getCamera().getPosition();
        glm::vec3 camFront = glm::normalize(player->getCamera().getForward());
        glm::vec3 camRight = glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 camUp = glm::normalize(glm::cross(camRight, camFront));

        float distanceInFront = 0.2f;
        float sideOffset = 0.0f;
        float verticalOffset = 0.0f;

        glm::vec3 notePos = camPos + camFront * distanceInFront + camRight * sideOffset + camUp * verticalOffset;
        closeUpNote->setTransform(notePos, camFront, false);
        hud->SetInstructionText("Press [ENTER] to hide note");
        shouldShowInstruction = true;
    }
    if (noteTimerStarted && (t - noteDisplayStartTime >= noteDisplayDuration))
    {
        closeUpNote->isRendered = false;
        noteCloseUpShown = true;
        noteTimerStarted = false;
    }

    if (note && note->isRendered && !noteTimerStarted)
    {
        float distance = glm::distance(note->geometry->getPosition(), player->getPosition());
        if (distance < 0.5f)
        {
            hud->SetInstructionText("Press [E] to pick up the note");
            shouldShowInstruction = true;
        }
    }

    glm::vec2 zoneCenter(29.0f, 17.0f);
    float zoneRadius = 5.0f;
    float zoneHeightMin = -3.0f;
    float zoneHeightMax = 3.0f;

    // Only show "drop remote" if not showing "pickup remote"
    glm::vec2 playerXZ(player->getPosition().x, player->getPosition().z);
    float distanceToCenter = glm::distance(playerXZ, zoneCenter);
    bool inCylindricalZone =
        distanceToCenter <= zoneRadius &&
        player->getPosition().y >= zoneHeightMin &&
        player->getPosition().y <= zoneHeightMax;

    if (!shouldShowInstruction && inCylindricalZone && !in_bloomy_world)
    {
        hud->SetInstructionText("Press [F] to drop the remote into the pit");
        shouldShowInstruction = true;
    }

    hud->SetShowInstruction(shouldShowInstruction);
    hud->Render();

    /*--PROCESS INPUT--*/
    glfwGetCursorPos(window, &xpos, &ypos);
    processMouseInput(xpos, ypos);
    processInput(window, dt);

    /*--PLAYER UPDATES--*/
    player->update(dt, physics.gScene);

    if (player->getRemoteThrowable() && player->getRemoteThrowable()->isRendered)
    {
        player->getRemoteThrowable()->updateTransform();
    }

    float playerSize = player->getSize();
    float playerPos = player->getPosition().y;
    float playerFeet = player->getFootPosition();

    float ditherWaterPos = ditherWaterFloor->geometry->getPosition().y;
    float bloomyWaterPos = bloomyWaterFloor->geometry->getPosition().y;

    if (playerPos > 5.3f)
    {
        player->getState().ChargeRemote(dt);
        // player->getState().Heal(10);
    }

    if (in_bloomy_world)
    {
        if ((playerFeet <= bloomyWaterPos))
        {
            player->registerDamage(dt, 10.0f, damageSound);
        }

        if ((playerPos >= bloomyWaterPos))
        {
            underwater = false;
        }
        if ((playerPos < bloomyWaterPos))
        {
            underwater = true;
        }
    }
    if (in_bloomy_world && (playerPos < bloomyWaterPos))
    {
    }

    if (!in_bloomy_world && playerFeet > ditherWaterPos)
    {
        player->registerDamage(dt, 2.0f, damageSound);
        player->getState().DrainRemote(dt);
    }

    if (in_bloomy_world || playerFeet <= ditherWaterPos)
        player->getState().ChargeRemote(dt);

    if (player->getState().GetHealth() <= 0.0f || playerPos <= -5.0f)
    {
        g_GameState = GameState::GameOver;
        return;
    }
    updatePhysics(dt);

    // FPS counter logic
    // fpsTimer += dt;
    // frameCount++;

    // if (fpsTimer >= 1.0f)
    // {
    //     float currentFPS = frameCount / fpsTimer;
    //     std::cout << "FPS: " << currentFPS << std::endl;

    //     fpsTimer = 0.0f;
    //     frameCount = 0;
    // }
}

void Game::setPerFrameUniforms(Shader *shader, POVCamera &camera, DirectionalLight &dirL, PointLight &pointL)
{

    // get projection around position, so shadowmap is dynamic to player position
    glm::vec3 camPos = player->getPosition();
    glm::vec3 lightDir = glm::normalize(dirL.direction);
    glm::vec3 lightPos = camPos - lightDir * 40.0f;

    glm::mat4 lightView = glm::lookAt(
        lightPos,
        camPos,
        glm::vec3(0.0f, 1.0f, 0.0f));

    float halfSizeX = 10.0f;
    float halfSizeY = 10.0f;
    glm::mat4 lightProjection = glm::ortho(
        -halfSizeX, halfSizeX,
        -halfSizeY, halfSizeY,
        1.0f, 50.0f);

    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    // Uniforms setzen
    shader->use();
    shader->setUniform("u_time", (float)glfwGetTime());
    shader->setUniform("in_bloomy_world", in_bloomy_world);
    shader->setUniform("shadowMap", 5);
    shader->setUniform("lightSpaceMatrix", lightSpaceMatrix);
    shader->setUniform("viewProjMatrix", player->getViewProjectionMatrix());
    shader->setUniform("camera_world", camPos);
    shader->setUniform("dirL.color", dirL.color);
    shader->setUniform("dirL.direction", dirL.direction);
    shader->setUniform("pointL.color", pointL.color);
    shader->setUniform("pointL.position", pointL.position);
    shader->setUniform("pointL.attenuation", pointL.attenuation);
    shader->setUniform("useNormalMap", useNormalMap ? 1 : 0);
}

void Game::updatePhysics(float deltaTime)
{
    physics.gScene->simulate(deltaTime);
    physics.gScene->fetchResults(true);
}
void Game::processInput(GLFWwindow *window, float deltaTime)
{
    bool isMoving = false;

    static bool leftMouseWasDown = false;
    static bool escapeKeyWasDown = false;
    static bool qKeyWasDown = false;
    static bool eKeyWasDown = false;
    static bool nKeyWasDown = false;
    static bool fKeyWasDown = false;
    static bool spaceKeyWasDown = false;

    auto isKeyPressedThisFrame = [](int key, GLFWwindow *window, bool &wasDown)
    {
        bool isDown = glfwGetKey(window, key) == GLFW_PRESS;
        bool justPressed = isDown && !wasDown;
        wasDown = isDown;
        return justPressed;
    };

    auto isMousePressedThisFrame = [](int button, GLFWwindow *window, bool &wasDown)
    {
        bool isDown = glfwGetMouseButton(window, button) == GLFW_PRESS;
        bool justPressed = isDown && !wasDown;
        wasDown = isDown;
        return justPressed;
    };

    // Mouse button interaction (e.g., transition)
    if (isMousePressedThisFrame(GLFW_MOUSE_BUTTON_LEFT, window, leftMouseWasDown) && !transitionActive)
    {
        if (player->getState().IsRemoteInInventory() && player->getState().GetRemoteCharge() >= 10.0f)
        {
            if (switchWorldPlayer)
            {
                switchWorldPlayer->setVolume(20.0f);
                switchWorldPlayer->play();
            }
            transitionActive = true;
            transitionTimer = 0.0f;
            should_transition = true;
        }
    }

    // Escape key pressed (pause game)
    if (isKeyPressedThisFrame(GLFW_KEY_ESCAPE, window, escapeKeyWasDown))
    {
        g_GameState = GameState::Paused;
        // glfwSetWindowShouldClose(window, true);
    }

    // Toggle controls guide
    if (isKeyPressedThisFrame(GLFW_KEY_Q, window, qKeyWasDown))
    {
        show_controls_guide = !show_controls_guide;
    }

    // Toggle normal map
    if (isKeyPressedThisFrame(GLFW_KEY_N, window, nKeyWasDown))
    {
        useNormalMap = !useNormalMap;
    }

    // Toggle normal map
    if (isKeyPressedThisFrame(GLFW_KEY_ENTER, window, nKeyWasDown))
    {
        if (noteTimerStarted)
        {
            closeUpNote->isRendered = false;
            noteCloseUpShown = true;
            noteTimerStarted = false;
        }
    }

    // Toggle interaction
    if (isKeyPressedThisFrame(GLFW_KEY_E, window, eKeyWasDown))
    {

        // Try nearby pickup first
        player->tryPickupNearbyObject(remote.get(), physics.gScene, pickUpRemoteSound);
        player->tryPickupNearbyObject(note.get(), physics.gScene, pickUpRemoteSound);
        player->tryPickupNearbyObject(player->getRemoteThrowable().get(), physics.gScene, pickUpRemoteSound);

        // raycast-based interaction
        player->pickObject(xpos, ypos, window_width, window_height, physics.gScene);
        player->showInventory();
    }

    // Throw Remote
    if (isKeyPressedThisFrame(GLFW_KEY_F, window, fKeyWasDown))
    {
        if (player->getState().IsRemoteInInventory())
        {
            player->throwRemote();
        }
    }

    // Handle sprinting
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        player->spendStamina(deltaTime); // Spend stamina on sprinting
        player->setSprinting(player->canSprint());
    }
    else
    {
        player->setSprinting(false); // Stop sprinting
    }

    // Handle movement with WASD keys
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && !transitionActive)
    {
        player->handleInput('W', deltaTime);
        isMoving = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !transitionActive)
    {
        player->handleInput('S', deltaTime);
        isMoving = true;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && !transitionActive)
    {
        player->handleInput('A', deltaTime);
        isMoving = true;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && !transitionActive)
    {
        player->handleInput('D', deltaTime);
        isMoving = true;
    }

    // Handle jumping with spacebar
    if (isKeyPressedThisFrame(GLFW_KEY_SPACE, window, spaceKeyWasDown) && !transitionActive)
    {
        player->handleInput(' ', deltaTime); // Space bar for jump
    }

    // Update head bobbing based on movement
    player->updateHeadBob(deltaTime, isMoving);
}

void Game::processMouseInput(double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;
    if (!transitionActive)
    {
        player->handleMouse(xoffset, yoffset);
    }
}

void Game::drawFullScreenQuadWithAlpha(float alpha)
{
    static GLuint quadVAO = 0;
    static GLuint quadVBO = 0;

    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,

            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f};

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    }

    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // transition shader is used here
    transitionShader->use();
    transitionShader->setUniform("uAlpha", alpha);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
