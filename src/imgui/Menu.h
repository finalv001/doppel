#include <GLFW/glfw3.h>
#include "imgui.h"
#include "../GameLogic/GameState.h"
#include <vector>

class Menu
{
public:
    Menu(int windowWidth, int windowHeight);

    void Render();
    bool startGame = false;
    bool quitGame = false;

private:
    void RenderGameOverMenu();
    void RenderPauseMenu();
    void RenderWinMenu();
    void RenderMainMenu();
    int windowWidth;
    int windowHeight;
    void DrawDitheredOverlay(ImVec2 min, ImVec2 max, ImColor color);
};