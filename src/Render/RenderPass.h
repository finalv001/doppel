#pragma once

#include <memory>
#include <vector>
#include <GL/glew.h>
#include "../Geometry.h"
#include "../Shader.h"
#include "../TessellationShader.h"
#include "../RenderObject.h"

class RenderPass
{
public:
    RenderPass(int width, int height, std::vector<std::shared_ptr<RenderObject>> &renderObjects)
        : width(width), height(height), renderObjects(&renderObjects) {}

    RenderPass(int width, int height)
        : width(width), height(height), renderObjects(nullptr) {}

    virtual ~RenderPass()
    {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &map);
    }

    virtual void Init() = 0;

    virtual void Execute() = 0;

protected:
    GLuint fbo, map;
    int width, height;
    std::vector<std::shared_ptr<RenderObject>> *renderObjects = nullptr;

    bool useModelMatrix = false;
    GLuint textureUnit;
    void BindFramebuffer()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    void BindTexture()
    {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, map);
    }

    void UnbindFramebuffer()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void checkGlError(const char *location)
    {
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            std::cerr << "[GL ERROR] 0x" << std::hex << err << " at: " << location << std::endl;
        }
    }
};