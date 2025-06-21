#include <random>

#include "Mesh.h"
#include "Shader.h"
#ifndef MODEL_H
#define MODEL_H

using namespace std;

class Model {
public:
    vector<Mesh> meshes;

    Model(vector<Mesh> meshes) {
        this->meshes = meshes;
    }

    void Draw(Shader &shader) {
        for (Mesh mesh : meshes) {
            mesh.Draw(shader);
        }
    };
};

#endif //MODEL_H
