//
// Created by dj on 22/06/2025.
//

#pragma once
#include <string>

#include "PhysicsEngine.h"
#include "libs/glm/glm.hpp"
#include "src/c/util/Model.h"
#include "src/c/util/Shader.h"

class GameObject : public PhysicsObject {
public:
    Model model;
    Shader shader;
    std::string type;
    glm::vec3 velocity = glm::vec3(0);
    float pitch = 0;
    float yaw = 0;
    float gravity = 0.98f;
    bool isStatic = false;
    glm::mat4 transform = glm::mat4(1.0f);
    int id = -1;

    void draw();
    void baseTick();
    virtual void tick() = 0;
    virtual ~GameObject() = default;

    explicit GameObject(glm::vec3 vec);
    GameObject(const GameObject&) = default;
    GameObject& operator=(const GameObject&) = default;

private:
    GameObject();
};
