#include "GameObject.h"
#include "../util/GameConstants.h"
#include "../util/ModelUtil.h"
#include "../PreImports.h"

GameObject::GameObject(const glm::vec3 vec)
    : position(vec),
      mass(1.0f),
      gravity(0.98),
      model(ModelUtil::getModel("unknown")),
      shader(GameConstants::defaultShader) {}

GameObject::GameObject() {
    throw std::runtime_error("Cannot create empty GameObject");
}

void GameObject::applySlowdown(float drag) {
    velocity *= (1.0f - drag);
}

void GameObject::update(float dt) {
    if (!isStatic) {
        velocity += glm::vec3(0.0f, -gravity, 0.0f) * dt;
        position += velocity * dt;
    }
    collisions.clear();
}

void GameObject::draw(float deltaTime) {
    shader.use();
    animator.tick(deltaTime);
    model->draw(shader, this->transform, animator);
}

void GameObject::baseTick() {
    this->transform = glm::mat4(1.0f);
    this->transform = glm::translate(this->transform, this->position);
    this->transform = glm::rotate(this->transform, glm::radians(this->yaw), glm::vec3(0, 1, 0));
    this->transform = glm::rotate(this->transform, glm::radians(this->pitch), glm::vec3(1, 0, 0));
    this->tick();
}