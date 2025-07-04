#pragma once

#include "Shader.h"

class TessellationShader : public Shader {
    public:
        TessellationShader(
            const std::string& vs,
            const std::string& fs,
            const std::string& tcs,
            const std::string& tes
        );
    
    private:
        bool loadShader(const std::string& file, GLenum shaderType, GLuint& handle);
    };