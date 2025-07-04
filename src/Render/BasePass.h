#pragma once

#include "RenderPass.h"
#include "../Skybox.h"
#include "../GameLogic/Player.h"
class BasePass : public RenderPass
{
public:
    BasePass(int width, int height, std::vector<std::shared_ptr<RenderObject>> &renderObjects, Player *player, bool &inBloomyWorld, bool &underwater);

    void Init() override;
    void Execute();

private:
    GLuint colorBuffers[2], rboDepth, pingpongFBO[2], pingpongBuffers[2], brightFBO, brightBuffer, hdrBuffer, hdrFBO;
    std::shared_ptr<Shader> blurShader = std::make_shared<Shader>("assets/shaders/gaussianBlur.vert", "assets/shaders/gaussianBlur.frag");
    std::shared_ptr<Shader> compositeShader = std::make_shared<Shader>("assets/shaders/composite.vert", "assets/shaders/composite.frag");
    Player *player;
    Skybox skybox;
    bool &inBloomyWorld;
    bool &underwater;
    void drawFullScreenQuad();
};