#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class GUIManager
{
public:
    bool Init(GLFWwindow *window);
    void BeginFrame();
    void EndFrame();
    void Cleanup();

private:
    GLFWwindow *window;
};