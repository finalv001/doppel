#pragma once

#include "RenderPass.h"

class ShadowPass : public RenderPass
{
public:
    ShadowPass(Shader *shader, int width, int height, std::vector<std::shared_ptr<RenderObject>> &renderObjects, bool &inBloomyWorld);

    void Init() override;
    void Execute() override;

private:
    Shader *shader;
    bool &inBloomyWorld;
};