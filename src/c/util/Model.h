#pragma once

#include <random>

#include "AnimatorInstance.h"
#include "Mesh.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "Animation.h"
#include "Shader.h"
#include "RenderUtil.h"

using namespace std;

class Model {
public:
    // Bone name to list of meshes
    std::unordered_map<std::string, std::vector<Mesh>> boneMeshes;
    std::vector<Animation> animations;
    std::unordered_map<std::string, std::string> boneParents;

    Model(const std::unordered_map<std::string, std::vector<Mesh>> &boneMeshes,
          const std::vector<Mesh> &unboundMeshes,
          const std::vector<Animation> &animations,
          std::unordered_map<std::string, std::string> boneParents,
          std::unordered_map<std::string, glm::vec3> origins)
            : boneMeshes(boneMeshes),
              animations(animations), boneParents(boneParents) {}

    Model() = default;

    void draw(Shader &shader, const glm::mat4 &transform, AnimatorInstance &animator) const {
        shader.setInt("material.diffuse", 0);
        shader.setInt("material.mer", 1);

        for (const auto& [bone, meshes] : boneMeshes) {
            glm::mat4 boneTransform = transform * animator.getFinalTransform(bone);
            for (const auto& mesh : meshes) {
                mesh.draw(shader, boneTransform);
            }
        }
    }

    void drawBasic(Shader &shader) const {
        AnimatorInstance fakeAnimator;
        for (const auto& [_, meshes] : boneMeshes) {
            for (const auto& mesh : meshes) {
                mesh.draw(shader, glm::mat4(1.0f));
            }
        }
    }
};
