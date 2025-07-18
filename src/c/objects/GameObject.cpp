#include "GameObject.h"
#include "../util/GameConstants.h"
#include "../util/model/ModelUtil.h"
#include "../PreImports.h"
#include <glm/gtx/quaternion.hpp>

GameObject::GameObject(const glm::vec3 vec)
    : position(vec),
      mass(1.0f),
      gravity(0.98),
      model(ModelUtil::getModel("unknown")),
      shader(GameConstants::defaultShader),
      collisionParts(&this->model->collisions) {
}

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

    if (GameConstants::debugCollision) {
        for (auto& part : *collisionParts) {
            glm::vec3 direction = part.end - part.start;
            float length = glm::length(direction);
            glm::vec3 midPoint = (part.start + part.end) * 0.5f;

            // Step 1: scale along Z to stretch from a to b
            glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, length));

            // Step 2: rotation to align Z axis to direction
            glm::vec3 dirNorm = glm::normalize(direction);
            glm::vec3 up = glm::vec3(0, 0, 1); // assume original cube points along +Z

            glm::quat rotation = glm::rotation(up, dirNorm); // or use glm::rotation(dir1, dir2) from GLM_GTX_quaternion
            auto rotate = glm::mat4(rotation);

            // Step 3: move to centre point
            glm::mat4 translate = glm::translate(glm::mat4(1.0f), midPoint);

            // Final model matrix
            glm::mat4 model = translate * rotate * scale;
            model = glm::translate(model, this->position);
            shader.setMat4("model", model);

            renderBoundingBox();
        }
    }
}

void GameObject::baseTick() {
    this->transform = glm::mat4(1.0f);
    this->transform = glm::translate(this->transform, this->position);
    this->transform = glm::rotate(this->transform, glm::radians(this->yaw), glm::vec3(0, 1, 0));
    this->transform = glm::rotate(this->transform, glm::radians(this->pitch), glm::vec3(1, 0, 0));
    this->tick();
}

unsigned int boundingBoxVAO = 0;
unsigned int boundingBoxVBO;
void GameObject::renderBoundingBox() {
    if (boundingBoxVAO == 0)
    {
        float cubeEdgeVertices[] = {
            // Bottom face
            -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,

            // Top face
            -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,

            // Vertical edges
            -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &boundingBoxVAO);
        glGenBuffers(1, &boundingBoxVBO);
        glBindVertexArray(boundingBoxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, boundingBoxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeEdgeVertices), &cubeEdgeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(boundingBoxVAO);
    glDrawArrays(GL_LINES, 0, 24);
    glBindVertexArray(0);
}