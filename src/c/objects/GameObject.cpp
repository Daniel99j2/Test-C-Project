//
// Created by dj on 22/06/2025.
//

#include "GameObject.h"

#include "PhysicsEngine.h"

#include "src/c/util/GameConstants.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <../../../libs/glm/gtx/string_cast.hpp>

#include "src/c/util/Model.h"
#include "src/c/util/ModelUtil.h"

GameObject::GameObject(const glm::vec3 vec)
    : PhysicsObject(ShapeType::Rectangle, vec, glm::vec3(2, 2, 2), 1.0f, gravity),
      model(ModelUtil::getModel("unknown")),
      shader(GameConstants::defaultShader) {};

GameObject::GameObject() {
    throw std::runtime_error("Cannot create empty GameObject");
}

void GameObject::draw(float deltaTime) {
    shader.use();
    animator.tick(1.0f/60.0f);
    model.draw(shader, this->transform, deltaTime, animator);
}

void GameObject::baseTick() {
    this->transform = glm::mat4(1.0f);
    this->transform = glm::translate(this->transform, this->position);
    this->transform = glm::rotate(this->transform, glm::radians(this->yaw), glm::vec3(0, 1, 0));
    this->transform = glm::rotate(this->transform, glm::radians(this->pitch), glm::vec3(1, 0, 0));
}
