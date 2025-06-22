//
// Created by dj on 20/06/2025.
//

#include "ModelUtil.h"
#include <iostream>
#include <chrono>
#include <random>
#include "../../../libs/glew/include/GL/glew.h"
#include "../../../libs/json.hpp"
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>

#include "Model.h"

using namespace std;


Model ModelUtil::getModel(const char *filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Failed to open model file: " << filePath << endl;
        throw;
    }

    stringstream buffer;
    buffer << file.rdbuf();
    nlohmann::json data = nlohmann::json::parse(buffer.str());

    vector<Mesh> meshes;
    int mer;

    for (auto &i: data["textures"]) {
        if (i["pbr_channel"] == "color") {
            std::string key = "textures\\";
            std::string path = i["path"];
            std::string out = "error";

            size_t pos = path.rfind(key);
            if (pos != std::string::npos) {
                out = path.substr(pos + key.length());

                const std::string extension = ".png";
                if (out.size() >= extension.size() &&
                    out.compare(out.size() - extension.size(), extension.size(), extension) == 0) {
                    out = out.substr(0, out.size() - extension.size());
                    }
            }

            texture = RenderUtil::genTexture(("src/resources/textures/" + out));
            mer = RenderUtil::genPBR(("src/resources/textures/" + out));
            break;
        }
    }

    for (auto &i: data["elements"]) {
        map<Vertex, int> vertexMap;
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        int currentIndex = 0;

        glm::vec3 origin(
            static_cast<float>(i["origin"][0]) / 16.0f,
            static_cast<float>(i["origin"][1]) / 16.0f,
            static_cast<float>(i["origin"][2]) / 16.0f
        );

        auto vertexMapJson = i["vertices"];

        for (auto &[faceId, faceData]: i["faces"].items()) {
            auto vnames = faceData["vertices"];

            glm::vec3 vpos[3];
            for (int j = 0; j < 3; ++j) {
                auto arr = vertexMapJson[vnames[j]];
                vpos[j] = glm::vec3(
                              float(arr[0]) / 16.0f,
                              float(arr[1]) / 16.0f,
                              float(arr[2]) / 16.0f
                          ) + origin;
            }

            glm::vec3 edge1 = vpos[1] - vpos[0];
            glm::vec3 edge2 = vpos[2] - vpos[0];
            glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

            std::map<std::string, glm::vec2> uvs;
            for (auto &[vname, uvArr]: faceData["uv"].items()) {
                int data1 = faceData["texture"];
                uvs[vname] = glm::vec2(
                    static_cast<float>(uvArr[0]) / static_cast<int>(data["textures"][data1]["uv_width"]),
                    static_cast<float>(uvArr[1]) / static_cast<int>(data["textures"][data1]["uv_height"])
                );
            }

            //loop xyz
            for (int j = 0; j < 3; ++j) {
                Vertex v;
                v.Position = vpos[j];
                v.Normal = normal;
                v.TexCoords = uvs.contains(vnames[j]) ? uvs[vnames[j]] : glm::vec2(0.0f);

                if (!vertexMap.contains(v)) {
                    vertexMap[v] = currentIndex++;
                    vertices.push_back(v);
                }

                indices.push_back(vertexMap[v]);
            }
        }

        meshes.push_back(Mesh(vertices, indices, texture, mer));
    }

    return Model(meshes);
}
