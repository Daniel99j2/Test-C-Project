//
// Created by dj on 22/06/2025.
//

#include "GameConstants.h"

#include "../objects/type/Player.h"
#include "src/c/world/World.h"

bool GameConstants::wireframe = false;
bool GameConstants::debug = false;
int GameConstants::targetFPS = 60;

Shader GameConstants::defaultShader;
Shader GameConstants::skyboxShader;
Shader GameConstants::lightEmitterShader;

PhysicsEngine GameConstants::physicsEngine;

std::shared_ptr<Player> GameConstants::player;
World GameConstants::world;