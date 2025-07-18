//
// Created by dj on 30/06/2025.
//

#pragma once
#include <functional>
#include <iostream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Keybind.h"

class Keybind {
public:
    enum Mode {
        TOGGLE,
        HOLD
    };


    Keybind(int key, Mode mode, std::string name, std::function<void()> pressed, std::function<void()> whilst,
            std::function<void()> released, std::function<bool()> canBePressed = [](){return true;});

    void update(GLFWwindow* window) {
        bool beingPressed = (glfwGetKey(window, key) == GLFW_PRESS) && canBePressed();
        if (mode == TOGGLE) {
            bool wasToggled = isOutput;
            if (beingPressed && !wasPressd) {
                wasPressd = true;
                isOutput = !isOutput;
            } else {
                wasPressd = beingPressed;
            };

            if (!wasToggled && isOutput) {
                pressed();
            };
            if (wasToggled && !isOutput) {
                released();
            };

            if (isOutput) whilst();
        } else if (mode == HOLD) {
            if (beingPressed && !wasPressd) {
                pressed();
                wasPressd = true;
            };
            if (!beingPressed && wasPressd) {
                released();
                wasPressd = false;
            };
            isOutput = wasPressd;
            if (isOutput) whilst();
        }
    }

    bool isPressed() {
        return isOutput;
    }
    
private:
    bool isOutput = false;
    bool wasPressd = false;
    int key;
    Mode mode;
    std::string name;
    std::function<void()> pressed;
    std::function<void()> whilst;
    std::function<void()> released;
    std::function<bool()> canBePressed;
};