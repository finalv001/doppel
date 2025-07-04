/*
 * Copyright 2023 Vienna University of Technology.
 * Institute of Computer Graphics and Algorithms.
 * This file is part of the GCG Lab Framework and must not be redistributed.
 */

#include "Utils.h"
#include <sstream>
#include <windows.h>
#include "./imgui/Menu.h"
#include "./imgui/GUIManager.h"
#include "imgui.h"
#include "GameLogic/Game.h"
#include "GameLogic/GameState.h"

using namespace physx;
#undef min
#undef max

/* --------------------------------------------- */
// Prototypes
/* --------------------------------------------- */

void GLAPIENTRY DebugCallbackDefault(GLenum source,
                                     GLenum type,
                                     GLuint id,
                                     GLenum severity,
                                     GLsizei length,
                                     const GLchar *message,
                                     const void *userParam);
static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char *msg);

/* --------------------------------------------- */
// Global variables
/* --------------------------------------------- */

static bool _fullscreen = true;

/* --------------------------------------------- */
// Main
/* --------------------------------------------- */

int main(int argc, char **argv)
{
    std::cout << ":::::: Running Doppel... ::::::" << std::endl;

    /* --------------------------------------------- */
    // Load settings.ini
    /* --------------------------------------------- */

    INIReader window_reader("assets/settings/window.ini");
    std::string window_title = window_reader.Get("window", "title", "doppel");
    int refresh_rate = window_reader.GetInteger("window", "refresh_rate", 60);
    _fullscreen = window_reader.GetBoolean("window", "fullscreen", true);

    int window_width, window_height;
    if (_fullscreen)
    {
        window_width = GetSystemMetrics(SM_CXSCREEN);
        window_height = GetSystemMetrics(SM_CYSCREEN);
    }
    else
    {
        window_width = window_reader.GetInteger("window", "width", 800);
        window_height = window_reader.GetInteger("window", "height", 800);
    }

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // get the displays refresh-rate instead of hard coding it
    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode))
    {
        int refresh_rate = devMode.dmDisplayFrequency;
    }

    std::cout << "Working directory: " << std::filesystem::current_path() << std::endl;

    std::cout << "Resolution:       " << window_width << " X " << window_height << std::endl;
    std::cout << "Refresh Rate:     " << refresh_rate << "Hz" << std::endl;

    /* --------------------------------------------- */
    // Create context
    /* --------------------------------------------- */

    glfwSetErrorCallback([](int error, const char *description)
                         { std::cout << "GLFW error " << error << ": " << description << std::endl; });

    if (!glfwInit())
    {
        EXIT_WITH_ERROR("Failed to init GLFW");
    }
    std::cout << "GLFW was initialized." << std::endl;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // Request OpenGL version 4.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Request core profile
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);            // Create an OpenGL debug context
    glfwWindowHint(GLFW_REFRESH_RATE, refresh_rate);               // Set refresh rate
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Enable antialiasing (4xMSAA)
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Open window
    GLFWmonitor *monitor = nullptr;

    if (_fullscreen)
        monitor = glfwGetPrimaryMonitor();

    GLFWwindow *window = glfwCreateWindow(window_width, window_height, window_title.c_str(), monitor, nullptr);
    if (!window)
        EXIT_WITH_ERROR("Failed to create window");

    // This function makes the context of the specified window current on the calling thread.
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true;
    GLenum err = glewInit();

    // If GLEW wasn't initialized
    if (err != GLEW_OK)
    {
        EXIT_WITH_ERROR("Failed to init GLEW: " << glewGetErrorString(err));
    }
    std::cout << "GLEW was initialized." << std::endl;

    // Debug callback
    if (glDebugMessageCallback != NULL)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        // Register your callback function.

        glDebugMessageCallback(DebugCallbackDefault, NULL);
        // Enable synchronous callback. This ensures that your callback function is called
        // right after an error has occurred. This capability is not defined in the AMD
        // version.
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    }

    /* --------------------------------------------- */
    // Init framework
    /* --------------------------------------------- */

    if (!initFramework())
    {
        EXIT_WITH_ERROR("Failed to init framework");
    }
    std::cout << "Framework was initialized." << std::endl;

    glEnable(GL_DEPTH_TEST);

    const char *version = (const char *)glGetString(GL_VERSION);
    std::cout << "OpenGL Version: " << version << std::endl;

    /* --------------------------------------------- */
    // Initialize scene and render loop
    /* --------------------------------------------- */
    {
        GUIManager guiManager;
        guiManager.Init(window);
        Menu menu(window_width, window_height);
        std::unique_ptr<Game> game = std::make_unique<Game>(window);

        while (!glfwWindowShouldClose(window))
        {
            guiManager.BeginFrame();

            switch (g_GameState)
            {
            case GameState::MainMenu:
            case GameState::Paused:
            case GameState::Won:
            case GameState::GameOver:
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                menu.Render();
                break;
            case GameState::Quitting:
                glfwSetWindowShouldClose(window, true);
                break;
            case GameState::Restarting:
                std::cerr << ">>> SHUTDOWN START\n";
                game->Shutdown();
                std::cerr << ">>> SHUTDOWN DONE\n";

                g_GameState = GameState::Playing;

                std::cerr << ">>> GAME CONSTRUCTOR START\n";
                game = std::make_unique<Game>(window);
                std::cerr << ">>> GAME CONSTRUCTOR DONE\n";
                break;
            case GameState::Playing:
                game->Run();
                break;
            default:
                break;
            }

            guiManager.EndFrame();
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    /* --------------------------------------------- */
    // Destroy framework
    /* --------------------------------------------- */

    destroyFramework();

    /* --------------------------------------------- */
    // Destroy context and exit
    /* --------------------------------------------- */

    glfwTerminate();

    return EXIT_SUCCESS;
}

void GLAPIENTRY DebugCallbackDefault(GLenum source,
                                     GLenum type,
                                     GLuint id,
                                     GLenum severity,
                                     GLsizei length,
                                     const GLchar *message,
                                     const void *userParam)
{
    fprintf(stderr, "\n-- OpenGL Debug Message --\n");
    fprintf(stderr, "Message: %s\n", message);
    fprintf(stderr, "Type: 0x%x\n", type);
    fprintf(stderr, "ID: %u\n", id);
    fprintf(stderr, "Severity: 0x%x\n", severity);

    const char *sourceStr;
    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        sourceStr = "API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        sourceStr = "Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        sourceStr = "Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        sourceStr = "Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        sourceStr = "Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        sourceStr = "Other";
        break;
    default:
        sourceStr = "Unknown";
        break;
    }

    fprintf(stderr, "Source: %s\n", sourceStr);
}

static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char *msg)
{
    std::stringstream stringStream;
    std::string sourceString;
    std::string typeString;
    std::string severityString;

    // The AMD variant of this extension provides a less detailed classification of the error,
    // which is why some arguments might be "Unknown".
    switch (source)
    {
    case GL_DEBUG_CATEGORY_API_ERROR_AMD:
    case GL_DEBUG_SOURCE_API:
    {
        sourceString = "API";
        break;
    }
    case GL_DEBUG_CATEGORY_APPLICATION_AMD:
    case GL_DEBUG_SOURCE_APPLICATION:
    {
        sourceString = "Application";
        break;
    }
    case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
    {
        sourceString = "Window System";
        break;
    }
    case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
    {
        sourceString = "Shader Compiler";
        break;
    }
    case GL_DEBUG_SOURCE_THIRD_PARTY:
    {
        sourceString = "Third Party";
        break;
    }
    case GL_DEBUG_CATEGORY_OTHER_AMD:
    case GL_DEBUG_SOURCE_OTHER:
    {
        sourceString = "Other";
        break;
    }
    default:
    {
        sourceString = "Unknown";
        break;
    }
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
    {
        typeString = "Error";
        break;
    }
    case GL_DEBUG_CATEGORY_DEPRECATION_AMD:
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    {
        typeString = "Deprecated Behavior";
        break;
    }
    case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD:
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    {
        typeString = "Undefined Behavior";
        break;
    }
    case GL_DEBUG_TYPE_PORTABILITY_ARB:
    {
        typeString = "Portability";
        break;
    }
    case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:
    case GL_DEBUG_TYPE_PERFORMANCE:
    {
        typeString = "Performance";
        break;
    }
    case GL_DEBUG_CATEGORY_OTHER_AMD:
    case GL_DEBUG_TYPE_OTHER:
    {
        typeString = "Other";
        break;
    }
    default:
    {
        typeString = "Unknown";
        break;
    }
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
    {
        severityString = "High";
        break;
    }
    case GL_DEBUG_SEVERITY_MEDIUM:
    {
        severityString = "Medium";
        break;
    }
    case GL_DEBUG_SEVERITY_LOW:
    {
        severityString = "Low";
        break;
    }
    default:
    {
        severityString = "Unknown";
        break;
    }
    }

    stringStream << "OpenGL Error: " << msg;
    stringStream << " [Source = " << sourceString;
    stringStream << ", Type = " << typeString;
    stringStream << ", Severity = " << severityString;
    stringStream << ", ID = " << id << "]";

    return stringStream.str();
}
