//
// Created by dj on 22/06/2025.
//

#pragma once
#include <memory>

class World;
class Player;
class PhysicsEngine;
class Shader;

class GameConstants {
public:
    static bool wireframe;
    static bool debug;
    static int targetFPS;

    static Shader defaultShader;
    static Shader skyboxShader;
    static Shader lightEmitterShader;

    static PhysicsEngine physicsEngine;
    static std::shared_ptr<Player> player;
    static World world;
};
