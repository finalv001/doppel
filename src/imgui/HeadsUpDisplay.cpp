#include "HeadsUpDisplay.h"

HeadsUpDisplay::HeadsUpDisplay(PlayerState *state, bool *showControlsGuide, int windowWidth, int windowHeight)
    : showControlsGuide(showControlsGuide), playerState(state), windowWidth(windowWidth), windowHeight(windowHeight)
{
}

void HeadsUpDisplay::Render()
{
    if (*showControlsGuide)
    {
        RenderControlsGuide();
    }

    RenderPlayerState();

    if (showInstruction && !instructionText.empty())
    {
        RenderInstructionText();
    }
}

ImVec4 LerpColor(const ImVec4 &a, const ImVec4 &b, float t)
{
    return ImVec4(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t);
}

void HeadsUpDisplay::RenderControlsGuide()
{
    ImGuiIO &io = ImGui::GetIO();

    ImVec2 windowSize(300, 200);
    ImVec2 centerPos = ImVec2(
        (io.DisplaySize.x - windowSize.x) * 0.5f,
        (io.DisplaySize.y - windowSize.y) * 0.5f);

    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.3f);

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0.3f);

    ImGui::Begin("Controls Guide", nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove);
    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::Text("WASD - move");
    ImGui::Text("Space - jump");
    ImGui::Text("E - pick up objects");
    ImGui::Text("F - throw remote");
    ImGui::Text("LMB - use remote");
    ImGui::Text("SHIFT - sprint");
    ImGui::Text("ESC - toggle pause");
    ImGui::Text("Q - toggle controls guide");
    ImGui::Text("N - toggle normal mapping");

    ImGui::End();
}
void HeadsUpDisplay::RenderPlayerState()
{
    ImGuiIO &io = ImGui::GetIO();

    ImGui::SetNextWindowBgAlpha(0.3f);

    ImGui::Begin("Player Stats", nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoBackground);
    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;

    ImVec2 size = ImGui::GetWindowSize();
    ImGui::SetWindowPos(ImVec2(10, windowHeight - size.y - 10));

    float health = playerState->GetHealth();
    float stamina = playerState->GetStamina();
    float remoteCharge = playerState->GetRemoteCharge();

    // ImGui::Text("Health: %.2f", playerState->GetHealth());
    // ImGui::Text("Stamina: %.2f", playerState->GetStamina());

    ImVec4 green = ImVec4(94 / 255.0f, 188 / 255.0f, 0 / 255.0f, 1.0f);
    ImVec4 orange = ImVec4(222 / 255.0f, 120 / 255.0f, 11 / 255.0f, 1.0f);
    ImVec4 red = ImVec4(181 / 255.0f, 27 / 255.0f, 0 / 255.0f, 1.0f);
    ImVec4 black = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

    if (playerState->IsRemoteInInventory())
    {
        DrawBar(orange, remoteCharge);
    }

    DrawBar(red, health);

    DrawBar(green, stamina);

    ImGui::End();
}

void HeadsUpDisplay::RenderInstructionText()
{
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImVec2 textSize = ImGui::CalcTextSize(instructionText.c_str());
    ImVec2 textPos = ImVec2((windowWidth - textSize.x) * 0.5f, windowHeight * 0.9f);
    ImGui::SetNextWindowPos(textPos, ImGuiCond_Always);

    ImGui::Begin("InstructionHint", nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoBackground);

    ImGui::Text("%s", instructionText.c_str());

    ImGui::End();
}

void HeadsUpDisplay::DrawBar(ImColor color, float &percentage)
{
    float barWidth = windowWidth / 6;
    float barHeight = windowHeight / 80;
    float border = 2.0f;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    pos.x = (float)(int)pos.x;
    pos.y = (float)(int)pos.y;

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    draw_list->Flags &= ~ImDrawListFlags_AntiAliasedFill;
    draw_list->Flags &= ~ImDrawListFlags_AntiAliasedLines;

    // black border
    float borderThickness = 2.0f;
    ImVec2 outerMin = ImVec2(pos.x - border, pos.y - border);
    ImVec2 outerMax = ImVec2(pos.x + barWidth + border, pos.y + barHeight + border);
    draw_list->AddRect(outerMin, outerMax, IM_COL32(0, 0, 0, 255), 3.0f, 0, borderThickness);

    static const int bayer4[4][4] = {
        {0, 8, 2, 10},
        {12, 4, 14, 6},
        {3, 11, 1, 9},
        {15, 7, 13, 5}};

    // filled part
    float filledWidth = barWidth * percentage / 100.0f;
    // ImVec2 fillMax = ImVec2(pos.x + filledWidth, pos.y + barHeight);
    // draw_list->AddRectFilled(pos, fillMax, ImColor(color), 3.0f);

    const float pixelSize = 2.0f;
    for (float y = 0; y < barHeight; y += pixelSize)
    {
        for (float x = 0; x < filledWidth; x += pixelSize)
        {
            int mx = (int(x / pixelSize)) % 4;
            int my = (int(y / pixelSize)) % 4;
            float threshold = bayer4[my][mx] / 16.0f * 100.0f;

            if (percentage >= threshold)
            {
                ImVec2 pixelMin = ImVec2(pos.x + x, pos.y + y);
                ImVec2 pixelMax = ImVec2(pos.x + x + pixelSize, pos.y + y + pixelSize);
                draw_list->AddRectFilled(pixelMin, pixelMax, color);
            }
        }
    }
    ImGui::Dummy(ImVec2(barWidth + 2 * border, barHeight + 2 * border));
}

void HeadsUpDisplay::ShowInstructions(bool visible, const std::string &message)
{
    showInstruction = visible;
    instructionText = message;
}

void HeadsUpDisplay::SetInstructionText(const std::string &text)
{
    instructionText = text;
}
