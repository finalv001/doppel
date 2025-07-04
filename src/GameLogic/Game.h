
#pragma once
#include "../POVCamera.h"
#include "../Physics.h"
#include "PlayerState.h"
#include "Player.h"
#include "../Light.h"
#include "../GLTFLoader.h"
#include "../Render/RenderPass.h"
#include "../imgui/HeadsUpDisplay.h"
#include "../ObjectPicker.h"
#include "../Skybox.h"
#include "GameState.h"
#include <SFML/Audio.hpp>

class Game
{
public:
    Game(GLFWwindow *window);
    void Run();
    void End();
    void Pause();
    void Shutdown();

private:
    GLFWwindow *window;
    int window_width, window_height;
    double xpos, ypos;
    float lastX, lastY;
    float t, dt, t_sum, lastTime;
    float transitionTimer;

    bool transitionActive;
    bool should_transition;
    bool in_bloomy_world = true;
    bool interaction;
    bool show_controls_guide;
    bool firstMouse;
    bool useNormalMap = true;
    bool underwater = false;

    float fpsTimer = 0.0f;
    int frameCount = 0;
    bool remoteCloseUpShown = false;
    bool remoteTimerStarted = false;
    float remoteDisplayStartTime = 0.0f;
    float remoteDisplayDuration = 3.0f;

    bool noteCloseUpShown = false;
    bool noteTimerStarted = false;
    float noteDisplayStartTime = 0.0f;
    float noteDisplayDuration = 20.0f;

    std::shared_ptr<RenderObject> remote, bloomyWaterFloor, ditherWaterFloor, note, closeUpNote;
    glm::vec3 remotePosition = glm::vec3(0.2f, 0.4f, 0.4f);
    glm::vec3 notePosition = glm::vec3(21.26f, 6.5f, -6.27f);
    std::unique_ptr<Player> player;
    sf::Music music;
    sf::SoundBuffer pickUpRemoteSound, switchWorldSound, damageSound;
    std::unique_ptr<sf::Sound> switchWorldPlayer;
    Skybox skybox;
    POVCamera camera;
    Physics physics;
    std::vector<std::shared_ptr<Shader>> shaders;
    std::shared_ptr<Shader> transitionShader;
    std::shared_ptr<Shader> ditherShader;
    std::unique_ptr<RenderPass> shadowPass, basePass;
    DirectionalLight dirL;
    PointLight pointL;
    std::unique_ptr<HeadsUpDisplay> hud;
    std::vector<std::shared_ptr<RenderObject>> renderObjects;

    void setPerFrameUniforms(Shader *shader, POVCamera &camera, DirectionalLight &dirL, PointLight &pointL);
    void processInput(GLFWwindow *window, float deltaTime);
    void processMouseInput(double xpos, double ypos);
    void updatePhysics(float deltaTime);
    void drawFullScreenQuadWithAlpha(float alpha);
};
