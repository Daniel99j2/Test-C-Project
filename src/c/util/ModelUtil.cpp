//
// Created by dj on 20/06/2025.
//

#include "ModelUtil.h"
#include <iostream>
#include <chrono>
#include <random>
#include "../../../libs/json.hpp"
// ReSharper disable once CppUnusedIncludeDirective
#include "../../../libs/glew/include/GL/glew.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include "../util/Model.h"

using namespace std;

map<std::string, Model> models = {};

Model invalidModel = Model();

Model ModelUtil::genModel(string filePath) {
    ifstream file(("src/resources/models/" + filePath + ".bbmodel"));
    if (!file.is_open()) {
        cerr << "Failed to open model file: " << filePath << endl;
        throw std::runtime_error("Failed to open model file");
    }

    stringstream buffer;
    buffer << file.rdbuf();
    nlohmann::json data = nlohmann::json::parse(buffer.str());

    vector<Mesh> meshes;
    // for (auto &i: data["textures"]) {
    //     if (i["pbr_channel"] == "color") {
    //         std::string key = "textures\\";
    //         std::string path = i["path"];
    //         std::string out = "error";
    //
    //         size_t pos = path.rfind(key);
    //         if (pos != std::string::npos) {
    //             out = path.substr(pos + key.length());
    //
    //             const std::string extension = ".png";
    //             if (out.size() >= extension.size() &&
    //                 out.compare(out.size() - extension.size(), extension.size(), extension) == 0) {
    //                 out = out.substr(0, out.size() - extension.size());
    //                 }
    //         }
    //         break;
    //     }
    // }

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

                std::string key = "textures\\";
                std::string path = data["textures"][data1]["path"];
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
                uvs[vname] = RenderUtil::getUV((out + ".png"), uvs[vname]);
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

        meshes.push_back(Mesh(vertices, indices));
    }

    auto m = Model(meshes);
    models.insert({filePath, m});
    saveCBModel(("output/compiled_models/" + filePath + ".cbmodel"), m);
    return m;
}

void ModelUtil::loadModels(bool forceRegen) {
    //INSANE peformace increase... 200,000 faces goes from 20-30 secs to >2.5 secs (does not apply to startup/generation)
    if (!forceRegen) {
        std::filesystem::create_directories(std::filesystem::path("output/compiled_models/"));
        for (auto &entry: std::filesystem::recursive_directory_iterator("output/compiled_models/")) {
            if (entry.path().extension() == ".cbmodel") {
                auto model = loadCBModel(entry.path().generic_string());
                if (model.meshes.size() == 0) {
                    cerr << "Failed to load CB model: " << entry.path().generic_string() << endl;
                } else {
                    std::string key = "compiled_models/";
                    std::string path = entry.path().generic_string();
                    std::string out = "error";

                    size_t pos = path.rfind(key);
                    if (pos != std::string::npos) {
                        out = path.substr(pos + key.length());

                        const std::string extension = ".cbmodel";
                        if (out.size() >= extension.size() &&
                            out.compare(out.size() - extension.size(), extension.size(), extension) == 0) {
                            out = out.substr(0, out.size() - extension.size());
                            }
                    }

                    if (model.meshes.size() != 0) {
                        models.insert({out, model});
                        saveCBModel((entry.path().generic_string()), model);
                    }
                }
            }
        }
    }

    for (auto &entry: std::filesystem::recursive_directory_iterator("src/resources/models/")) {
        if (entry.path().extension() == ".bbmodel") {
            std::string path = entry.path().string();
            std::string key = "models/";
            std::string out = "error";

            size_t pos = path.rfind(key);
            if (pos != std::string::npos) {
                out = path.substr(pos + key.length());

                const std::string extension = ".bbmodel";
                if (out.size() >= extension.size() &&
                    out.compare(out.size() - extension.size(), extension.size(), extension) == 0) {
                    out = out.substr(0, out.size() - extension.size());
                }
            }
            if (!models.contains(out)) genModel(out);
        }
    }
}

Model ModelUtil::getModel(string name) {
    return models[name];
}

void ModelUtil::saveCBModel(const std::string &filepath, const Model &model) {
    std::filesystem::create_directories(std::filesystem::path(filepath).parent_path());

    std::ofstream out(filepath, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Failed to open file for writing: " << filepath << std::endl;
        return;
    }

    uint32_t verifier = 0x43424D44; //this can be anything, it is CBMD in binary, it checks if the file type is correct
    out.write(reinterpret_cast<char *>(&verifier), sizeof(verifier));

    uint32_t meshCount = model.meshes.size();
    out.write(reinterpret_cast<char *>(&meshCount), sizeof(meshCount));

    for (const Mesh &mesh: model.meshes) {
        // we write the vertices
        uint32_t vertexCount = mesh.vertices.size();
        out.write(reinterpret_cast<char *>(&vertexCount), sizeof(vertexCount));
        out.write(reinterpret_cast<const char *>(mesh.vertices.data()), vertexCount * sizeof(Vertex));

        // we write the indices
        uint32_t indexCount = mesh.indices.size();
        out.write(reinterpret_cast<char *>(&indexCount), sizeof(indexCount));
        out.write(reinterpret_cast<const char *>(mesh.indices.data()), indexCount * sizeof(uint32_t));
    }

    out.close();
}

Model ModelUtil::loadCBModel(const std::string &filepath) {
    try {
        std::ifstream in(filepath, std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "Failed to open file for reading: " << filepath << std::endl;
            return invalidModel;
        }

        uint32_t verifier;
        //we check if it has the verifier text
        in.read(reinterpret_cast<char *>(&verifier), sizeof(verifier));
        if (verifier != 0x43424D44) {
            std::cerr << "Invalid .cbmodel file format." << std::endl;
            return invalidModel;
        }

        uint32_t meshCount;
        in.read(reinterpret_cast<char *>(&meshCount), sizeof(meshCount));
        std::vector<Mesh> meshes;

        for (uint32_t i = 0; i < meshCount; ++i) {
            uint32_t vertexCount;
            in.read(reinterpret_cast<char *>(&vertexCount), sizeof(vertexCount));
            std::vector<Vertex> vertices(vertexCount);
            in.read(reinterpret_cast<char *>(vertices.data()), vertexCount * sizeof(Vertex));

            uint32_t indexCount;
            in.read(reinterpret_cast<char *>(&indexCount), sizeof(indexCount));
            std::vector<unsigned int> indices(indexCount);
            in.read(reinterpret_cast<char *>(indices.data()), indexCount * sizeof(uint32_t));

            meshes.emplace_back(vertices, indices);
        }

        in.close();
        return Model(meshes);
    } catch (const std::exception &e) {
        std::cerr << "Failed to read CBModel " << filepath << ": " << e.what() << std::endl;
    };
    return Model();
}