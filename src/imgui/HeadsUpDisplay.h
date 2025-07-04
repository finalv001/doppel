#pragma once
#include <imgui.h>
#include <iostream>

#include <GLFW/glfw3.h>
#include "../GameLogic/Playerstate.h"

class HeadsUpDisplay
{
public:
    HeadsUpDisplay(PlayerState *state, bool *showControlsGuide, int windowWidth, int windowHeight);
    void SetInstructionText(const std::string& text);
    void SetShowInstruction(bool in){showInstruction = in;};
    void Render();

private:
    void RenderPlayerState();
    void RenderControlsGuide();
    void RenderInstructionText();
    void DrawBar(ImColor color, float &percentage);
    void ShowInstructions(bool visible, const std::string& message);
    PlayerState *playerState;
    bool *showControlsGuide;
    int windowWidth, windowHeight;
    bool showInstruction = false;
    std::string instructionText = "";
};
