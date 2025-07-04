#pragma once
enum class GameState
{
    MainMenu,
    Playing,
    Paused,
    GameOver,
    Quitting,
    Restarting,
    Won
};

extern GameState g_GameState;