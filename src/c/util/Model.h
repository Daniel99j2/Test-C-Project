#pragma once

#include <random>

#include "AnimatorInstance.h"
#include "Mesh.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <../../../libs/glm/gtx/string_cast.hpp>

#include "Animation.h"
#include "Shader.h"
#include "RenderUtil.h"

using namespace std;

class Model {
public:
    vector<Mesh> meshes;
    vector<Animation> animations;
    std::unordered_map<std::string, std::string> boneParents;

    explicit Model(const vector<Mesh> &meshes, const vector<Animation> &animations, std::unordered_map<std::string, std::string> boneParents) {
        this->boneParents = boneParents;
        this->meshes = meshes;
        this->animations = animations;
    }

    Model() {
    };

    void Draw(::Shader &shader, const glm::mat4 & transform, float deltaTime, AnimatorInstance &animator) const {
        glActiveTexture(GL_TEXTURE0);
        shader.setInt("material.diffuse", 0);
        glBindTexture(GL_TEXTURE_2D, RenderUtil::getAtlas());

        glActiveTexture(GL_TEXTURE1);
        shader.setInt("material.mer", 1);
        glBindTexture(GL_TEXTURE_2D, RenderUtil::getMERAtlas());

        //debugging: CHECK:
        //TICKS?
        //DRAWS?
        //UPDATED?

        for (Mesh mesh: meshes) {
            mesh.draw(shader, transform, animator);
        }
    };
};
