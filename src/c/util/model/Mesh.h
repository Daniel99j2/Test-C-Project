#pragma once
#include "Shader.h"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <GL/glew.h>

#include "Material.h"

class GameConstants;
class ModelUtil;
class RenderUtil;
class Shader;

using namespace std;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
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
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    string name;
    Material* material;

    Mesh(const vector<Vertex> &vertices, const vector<unsigned int> &indices, const string &name, Material* material)
        : vertices(vertices), indices(indices), name(name), material(material) {
        setupMesh();
    };

    void draw(const Shader &shader, const glm::mat4 &transform) const {
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, material->base);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, material->metal);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, material->emissive);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, material->rough);
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, material->normal);

        shader.setInt("material.base", 5);
        shader.setInt("material.metal", 6);
        shader.setInt("material.emissive", 7);
        shader.setInt("material.rough", 8);
        shader.setInt("material.normal", 9);

        shader.setMat4("model", transform);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

private:
    unsigned int VAO, VBO, EBO;

    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

        // normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        // texcoords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        // tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

        // bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

        glBindVertexArray(0);
    }
};
