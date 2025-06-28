#pragma once
#include "../../../libs/glm/vec3.hpp"
#include "../../../libs/glm/vec2.hpp"

#include "libs/glew/include/GL/glew.h"

class GameConstants;
class ModelUtil;
class RenderUtil;
class Shader;

using namespace std;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

inline bool operator<(const Vertex &a, const Vertex &b) {
    if (a.Position.x != b.Position.x) return a.Position.x < b.Position.x;
    if (a.Position.y != b.Position.y) return a.Position.y < b.Position.y;
    if (a.Position.z != b.Position.z) return a.Position.z < b.Position.z;

    if (a.Normal.x != b.Normal.x) return a.Normal.x < b.Normal.x;
    if (a.Normal.y != b.Normal.y) return a.Normal.y < b.Normal.y;
    if (a.Normal.z != b.Normal.z) return a.Normal.z < b.Normal.z;

    if (a.TexCoords.x != b.TexCoords.x) return a.TexCoords.x < b.TexCoords.x;
    return a.TexCoords.y < b.TexCoords.y;
}

class Mesh {
public:
    // mesh data
    vector<Vertex> vertices;
    vector<unsigned int> indices;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices) {
        this->vertices = vertices;
        this->indices = indices;

        setupMesh();
    }

    void draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    };

private:
    //  render data
    unsigned int VAO, VBO, EBO;

    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                     &indices[0], GL_STATIC_DRAW);

        // vertex positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) 0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
    }
};
