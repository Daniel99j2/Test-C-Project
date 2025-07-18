#pragma once

#include <random>

#include "AnimatorInstance.h"
#include "Mesh.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "Animation.h"
#include "Shader.h"
#include "RenderUtil.h"

class CollisionPart;
using namespace std;

class Model {
public:
    std::vector<Mesh> meshes = {};
    std::vector<Animation> animations = {};
    std::vector<CollisionPart> collisions = {};
    map<std::string, Material> materials = {};

    Model() = default;

    void draw(Shader &shader, const glm::mat4 &transform, AnimatorInstance &animator) const {
        for (const auto& mesh : meshes) {
            glm::mat4 boneTransform = transform * animator.getTransform(mesh.name);
            mesh.draw(shader, boneTransform);
        }
    }

    void drawBasic(Shader &shader) const {
        for (const auto& mesh : meshes) {
            mesh.draw(shader, glm::mat4(1));
        }
    }
};
