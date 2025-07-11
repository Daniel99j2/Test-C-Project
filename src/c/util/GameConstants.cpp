//
// Created by dj on 22/06/2025.
//

#include "GameConstants.h"

#include "../objects/type/Player.h"
#include "Keybinds.h"
#include "../world/World.h"

bool GameConstants::wireframe = false;
bool GameConstants::debug = false;
bool GameConstants::debugging = false;
int GameConstants::targetFPS = 60;
int GameConstants::window_height = 720;
int GameConstants::window_width = 1280;
GLFWwindow* GameConstants::window;
bool GameConstants::fullscreen = false;

Shader GameConstants::defaultShader;
Shader GameConstants::skyboxShader;
Shader GameConstants::lightEmitterShader;

std::shared_ptr<Player> GameConstants::player;
World GameConstants::world;

Keybinds GameConstants::keybindsManager = Keybinds();