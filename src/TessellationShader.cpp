#include "TessellationShader.h"

TessellationShader::TessellationShader(
    const std::string &vs,
    const std::string &fs,
    const std::string &tcs,
    const std::string &tes)
{
    GLuint program = glCreateProgram();

    if (program == 0)
    {
        std::cerr << "Failed to create shader program." << std::endl;
        return;
    }

    GLuint vShader, fShader, tcsShader, tesShader;

    if (!loadShader(vs, GL_VERTEX_SHADER, vShader))
    {
        std::cerr << "Failed to load vertex shader: " << vs << std::endl;
        return;
    }
    if (!loadShader(fs, GL_FRAGMENT_SHADER, fShader))
    {
        std::cerr << "Failed to load fragment shader: " << vs << std::endl;
        return;
    }
    if (!loadShader(tcs, GL_TESS_CONTROL_SHADER, tcsShader))
    {
        std::cerr << "Failed to load tessellation control shader: " << vs << std::endl;
        return;
    }
    if (!loadShader(tes, GL_TESS_EVALUATION_SHADER, tesShader))
    {
        std::cerr << "Failed to load tessellation evalutation shader: " << vs << std::endl;
        return;
    }

    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glAttachShader(program, tcsShader);
    glAttachShader(program, tesShader);

    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char log[1024];
        glGetProgramInfoLog(program, 1024, NULL, log);
        std::cerr << "Shader program linking failed:\n"
                  << log << std::endl;
        return;
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);
    glDeleteShader(tcsShader);
    glDeleteShader(tesShader);

    _handle = program;
    usesTessellation = true;
}

bool TessellationShader::loadShader(const std::string &file, GLenum shaderType, GLuint &handle)
{
    namespace fs = std::filesystem;

    std::vector<std::string> searchPaths = {
        "",
        "../",
        "../../",
        "../../../",
        "assets/",
        "../assets/",
        "../../assets/"};

    std::ifstream in;
    std::string fullPath;

    for (const auto &prefix : searchPaths)
    {
        fs::path tryPath = prefix + file;
        in.open(tryPath, std::ios::in | std::ios::binary);
        if (in)
        {
            fullPath = tryPath.string();
            break;
        }
    }

    if (!in)
    {
        std::cerr << "Cannot open shader file: " << file << std::endl;
        return false;
    }

    std::string source;
    in.seekg(0, std::ios::end);
    source.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&source[0], source.size());
    in.close();

    // Remove BOM if present (EF BB BF)
    if (source.size() >= 3 &&
        (unsigned char)source[0] == 0xEF &&
        (unsigned char)source[1] == 0xBB &&
        (unsigned char)source[2] == 0xBF)
    {
        source = source.substr(3);
    }

    const char *src = source.c_str();
    handle = glCreateShader(shaderType);
    glShaderSource(handle, 1, &src, nullptr);
    glCompileShader(handle);

    GLint compiled;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        char log[1024];
        glGetShaderInfoLog(handle, 1024, nullptr, log);
        std::cerr << "Failed to compile shader: " << fullPath << "\n"
                  << log << std::endl;
        return false;
    }

    std::cerr << "Read shader: " << fullPath << std::endl;
    return true;
}