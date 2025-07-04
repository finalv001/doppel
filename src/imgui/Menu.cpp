#include "Menu.h"
#include <cmath>
Menu::Menu(int windowWidth, int windowHeight) : windowWidth(windowWidth), windowHeight(windowHeight) {}

void Menu::Render()
{
    switch (g_GameState)
    {
    case GameState::MainMenu:
        RenderMainMenu();
        break;
    case GameState::Paused:
        RenderPauseMenu();
        break;
    case GameState::GameOver:
        RenderGameOverMenu();
        break;
    case GameState::Won:
        RenderWinMenu();
        break;
    default:
        break;
    }
}
void Menu::RenderWinMenu()
{
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    float t = ImGui::GetTime();
    float scale = 1.0f + 0.1f * sin(t * 3.0f);
    static std::vector<ImVec2> textPositions;
    static std::vector<ImU32> textColors;
    static std::vector<float> textAngles;
    static bool initialized = false;
    float windowWidth = io.DisplaySize.x;
    float windowHeight = io.DisplaySize.y;

    if (!initialized)
    {
        initialized = true;
        textPositions.clear();
        textColors.clear();
        textAngles.clear();

        for (int i = 0; i < 4; ++i)
        {
            float x = (rand() / (float)RAND_MAX) * (windowWidth - 200.0f);
            float y = (rand() / (float)RAND_MAX) * (windowHeight - 30.0f);
            textPositions.push_back(ImVec2(x, y));

            ImU32 color = IM_COL32(rand() % 256, rand() % 256, rand() % 256, 255);
            textColors.push_back(color);

            float angle = ((rand() % 100) / 100.0f - 0.5f) * 1.0f;
            textAngles.push_back(angle);
        }
    }

    auto RotateAround = [](ImVec2 p, ImVec2 center, float a) -> ImVec2
    {
        float s = sin(a), c = cos(a);
        p.x -= center.x;
        p.y -= center.y;
        float xnew = p.x * c - p.y * s;
        float ynew = p.x * s + p.y * c;
        return ImVec2(xnew + center.x, ynew + center.y);
    };

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("Win Menu", nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoBackground);

    ImDrawList *drawList = ImGui::GetWindowDrawList();
    ImFont *font = ImGui::GetFont();
    float fontSize = ImGui::GetFontSize() * 4.0f;

    drawList->AddRectFilled(ImVec2(0, 0), ImVec2(windowWidth, windowHeight), IM_COL32(255, 255, 255, 255));

    DrawDitheredOverlay(ImVec2(0, 0), ImVec2(windowWidth, windowHeight), ImColor(20, 20, 20, 90));

    const char *texts[] = {"wow", "pro gamer", "so skilled", "look at you"};
    float appearTimes[] = {0.0f, 2.0f, 4.0f, 6.0f};

    for (int i = 0; i < 4; ++i)
    {
        if (t > appearTimes[i])
        {
            const char *text = texts[i];
            ImVec2 size = ImGui::CalcTextSize(text);
            ImVec2 textCenter = ImVec2(
                textPositions[i].x + size.x * 0.5f,
                textPositions[i].y + size.y * 0.5f);
            ImVec2 rotated = RotateAround(textPositions[i], textCenter, textAngles[i] + sin(t + i) * 0.2f);
            drawList->AddText(font, fontSize, rotated, textColors[i], text);
        }
    }

    float buttonWidth = windowWidth / 3.0f;
    float buttonHeight = windowWidth / 20.0f;
    float buttonSpacing = 20.0f;
    float blockHeight = buttonHeight * 2 + buttonSpacing;

    float blockStartY = center.y - blockHeight * 0.5f;
    float titleSpacing = 40.0f;
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));

    const char *titleText = "YOU WON";
    ImVec2 titleSize = ImGui::CalcTextSize(titleText);
    ImVec2 scaledSize = ImVec2(titleSize.x * scale, titleSize.y * scale);
    ImVec2 titlePos(center.x - scaledSize.x * 0.5f, blockStartY - scaledSize.y - titleSpacing);

    drawList->AddText(
        font,
        fontSize * scale,
        titlePos,
        IM_COL32(0, 0, 0, 255),
        titleText);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));

    ImGui::SetCursorPos(ImVec2(center.x - buttonWidth * 0.5f, blockStartY));
    if (ImGui::Button("New Game", ImVec2(buttonWidth, buttonHeight)))
    {
        g_GameState = GameState::Restarting;
    }

    ImGui::SetCursorPos(ImVec2(center.x - buttonWidth * 0.5f, blockStartY + buttonHeight + buttonSpacing));

    if (ImGui::Button("Quit", ImVec2(buttonWidth, buttonHeight)))
    {
        g_GameState = GameState::Quitting;
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleColor(3);
    ImGui::End();
}

void Menu::RenderGameOverMenu()
{
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("Game over Menu", nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoBackground);

    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(windowWidth, windowHeight), IM_COL32(255, 255, 255, 255));

    DrawDitheredOverlay(ImVec2(0, 0), ImVec2(windowWidth, windowHeight), ImColor(20, 20, 20, 90));

    float buttonWidth = windowWidth / 3.0f;
    float buttonHeight = windowWidth / 20.0f;
    float buttonSpacing = 20.0f;
    float blockHeight = buttonHeight * 2 + buttonSpacing;

    float blockStartY = center.y - blockHeight * 0.5f;
    float titleSpacing = 40.0f;
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));

    const char *titleText = "Game Over";
    ImVec2 titleSize = ImGui::CalcTextSize(titleText);
    ImGui::SetCursorPos(ImVec2(center.x - titleSize.x * 0.5f, blockStartY - titleSize.y - titleSpacing));
    ImGui::Text("%s", titleText);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));

    ImGui::SetCursorPos(ImVec2(center.x - buttonWidth * 0.5f, blockStartY));
    if (ImGui::Button("New Game", ImVec2(buttonWidth, buttonHeight)))
    {
        g_GameState = GameState::Restarting;
    }

    ImGui::SetCursorPos(ImVec2(center.x - buttonWidth * 0.5f, blockStartY + buttonHeight + buttonSpacing));

    if (ImGui::Button("Quit", ImVec2(buttonWidth, buttonHeight)))
    {
        g_GameState = GameState::Quitting;
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleColor(3);
    ImGui::End();
}

void Menu::RenderPauseMenu()
{
    ImGuiIO &io = ImGui::GetIO();
    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        g_GameState = GameState::Playing;
    }

    ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("Pause Menu", nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoBackground);

    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(windowWidth, windowHeight), IM_COL32(255, 255, 255, 255));

    DrawDitheredOverlay(ImVec2(0, 0), ImVec2(windowWidth, windowHeight), ImColor(20, 20, 20, 90));

    float buttonWidth = windowWidth / 3.0f;
    float buttonHeight = windowWidth / 20.0f;
    float buttonSpacing = 20.0f;
    float blockHeight = buttonHeight * 2 + buttonSpacing;

    float blockStartY = center.y - blockHeight * 0.5f;
    float titleSpacing = 40.0f;
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));

    const char *titleText = "DOPPEL";
    ImVec2 titleSize = ImGui::CalcTextSize(titleText);
    ImGui::SetCursorPos(ImVec2(center.x - titleSize.x * 0.5f, blockStartY - titleSize.y - titleSpacing));
    ImGui::Text("%s", titleText);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));

    ImGui::SetCursorPos(ImVec2(center.x - buttonWidth * 0.5f, blockStartY));
    if (ImGui::Button("Continue", ImVec2(buttonWidth, buttonHeight)))
    {
        g_GameState = GameState::Playing;
    }

    ImGui::SetCursorPos(ImVec2(center.x - buttonWidth * 0.5f, blockStartY + buttonHeight + buttonSpacing));

    if (ImGui::Button("Restart", ImVec2(buttonWidth, buttonHeight)))
    {
        g_GameState = GameState::Restarting;
    }

    ImGui::SetCursorPos(ImVec2(center.x - buttonWidth * 0.5f, blockStartY + buttonHeight * 2 + buttonSpacing * 2));
    if (ImGui::Button("Quit", ImVec2(buttonWidth, buttonHeight)))
    {
        g_GameState = GameState::Quitting;
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleColor(3);
    ImGui::End();
}

void Menu::RenderMainMenu()
{
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("Main Menu", nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoBackground);

    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(windowWidth, windowHeight), IM_COL32(255, 255, 255, 255));

    DrawDitheredOverlay(ImVec2(0, 0), ImVec2(windowWidth, windowHeight), ImColor(20, 20, 20, 90));

    float buttonWidth = windowWidth / 3.0f;
    float buttonHeight = windowWidth / 20.0f;
    float buttonSpacing = 20.0f;
    float blockHeight = buttonHeight * 2 + buttonSpacing;

    float blockStartY = center.y - blockHeight * 0.5f;
    float titleSpacing = 40.0f;
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));

    const char *titleText = "DOPPEL";
    ImVec2 titleSize = ImGui::CalcTextSize(titleText);
    ImGui::SetCursorPos(ImVec2(center.x - titleSize.x * 0.5f, blockStartY - titleSize.y - titleSpacing));
    ImGui::Text("%s", titleText);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));

    ImGui::SetCursorPos(ImVec2(center.x - buttonWidth * 0.5f, blockStartY));
    if (ImGui::Button("Start", ImVec2(buttonWidth, buttonHeight)))
    {
        g_GameState = GameState::Playing;
    }

    ImGui::SetCursorPos(ImVec2(center.x - buttonWidth * 0.5f, blockStartY + buttonHeight + buttonSpacing));
    if (ImGui::Button("Quit", ImVec2(buttonWidth, buttonHeight)))
        g_GameState = GameState::Quitting;
    ImGui::PopStyleColor();
    ImGui::PopStyleColor(3);
    ImGui::End();
}

void Menu::DrawDitheredOverlay(ImVec2 min, ImVec2 max, ImColor color)
{
    static const int bayer4[4][4] = {
        {0, 8, 2, 10},
        {12, 4, 14, 6},
        {3, 11, 1, 9},
        {15, 7, 13, 5}};

    ImDrawList *drawList = ImGui::GetWindowDrawList();
    const float pixelSize = 2.0f;
    float width = max.x - min.x;
    float height = max.y - min.y;
    for (float y = min.y; y < max.y; y += pixelSize)
    {
        for (float x = min.x; x < max.x; x += pixelSize)
        {
            int bx = (int(x / pixelSize)) % 4;
            int by = (int(y / pixelSize)) % 4;
            float ditherThreshold = bayer4[by][bx] / 16.0f;

            float gradient = 1.0f - ((y - min.y) / height);

            gradient = pow(gradient, 1.5f);

            if (gradient >= ditherThreshold)
            {
                drawList->AddRectFilled(
                    ImVec2(x, y),
                    ImVec2(x + pixelSize, y + pixelSize),
                    color);
            }
        }
    }
}