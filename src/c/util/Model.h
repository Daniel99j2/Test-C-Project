#pragma once

#include <random>

#include "Mesh.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <../../../libs/glm/gtx/string_cast.hpp>
#include "Shader.h"
#include "RenderUtil.h"

using namespace std;

class Model {
public:
    vector<Mesh> meshes;

    explicit Model(const vector<Mesh> &meshes) {
        this->meshes = meshes;
    }

    Model() {
    };

    void Draw(const Shader &shader, const glm::mat4 &transform) const {
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
        shader.setMat4("model", transform);

        for (Mesh mesh: meshes) {
            mesh.draw();
        }
    };
};
