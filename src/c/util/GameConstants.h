//
// Created by dj on 22/06/2025.
//

#pragma once
#include <memory>

#include "../../../libs/glew/include/GL/glew.h"
#include <GLFW/glfw3.h>

class World;
class Player;
class Shader;
class Keybinds;

class GameConstants {
public:
    static bool wireframe;
    static bool debug;
    static bool debugging;
    static int targetFPS;
    static int window_width;
    static int window_height;
    static bool fullscreen;

    static Shader defaultShader;
    static Shader skyboxShader;
    static Shader lightEmitterShader;

    static std::shared_ptr<Player> player;
    static World world;
    static Keybinds keybindsManager;
    static GLFWwindow* window;
};
