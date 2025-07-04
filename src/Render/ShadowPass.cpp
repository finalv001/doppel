#include "ShadowPass.h"

ShadowPass::ShadowPass(Shader *shader, int width, int height, std::vector<std::shared_ptr<RenderObject>> &renderObjects, bool &inBloomyWorld)
    : RenderPass(width, height, renderObjects), shader(shader), inBloomyWorld(inBloomyWorld)
{
    textureUnit = 5;
    Init();
}

void ShadowPass::Init()
{
    glGenFramebuffers(1, &fbo);

    glGenTextures(1, &map);
    glBindTexture(GL_TEXTURE_2D, map);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, map, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowPass::Execute()
{
    BindFramebuffer();
    shader->use();

    for (const auto &renderObject : *renderObjects)
    {
        uint32_t mask = renderObject->geometry->getWorldMask();
        bool doRenderObj = ((mask & WORLD_BLOOM) && inBloomyWorld) || ((mask & WORLD_DITHER) && !inBloomyWorld);
        if (renderObject->isRendered == true && doRenderObj && renderObject->id != "floor")
        {
            renderObject->geometry->draw(shader);
        }
    }

    UnbindFramebuffer();
    BindTexture();

    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
