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

#include "Animation.h"
#include "../util/Model.h"

using namespace std;

map<std::string, Model> models = {};

Model invalidModel = Model();

void writeString(std::ofstream &out, const std::string &str) {
    uint32_t len = str.size();
    out.write(reinterpret_cast<char*>(&len), sizeof(len));
    out.write(str.c_str(), len);
}

std::string readString(std::ifstream &in) {
    uint32_t len;
    in.read(reinterpret_cast<char*>(&len), sizeof(len));
    std::string str(len, '\0');
    in.read(&str[0], len);
    return str;
}

Model ModelUtil::genModel(const string& filePath) {
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

    //we load the elements
    for (auto &i: data["elements"]) {
        map<Vertex, int> vertexMap;
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        int currentIndex = 0;

        //blockbench's units are *16 of ours
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
                //again, scale down
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

                //we change uv scale based on texture size
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

        string boneName = "no_bone";

        for (const auto& item : data["outliner"]) {
            if (item.is_object()) {
                for (const auto& child : item["children"]) {
                    if (child == i["uuid"]) {
                        boneName = item["name"];
                    }
                }
            }
        }
        meshes.push_back(Mesh(vertices, indices, boneName));
    }

    //animations!!!
    std::unordered_map<std::string, std::string> boneParents;
    vector<Animation> animations;
    for (const auto& animData : data["animations"]) {
        Animation anim;
        anim.name = animData["name"];
        anim.length = animData["length"];
        std::string loopStr = animData["loop"];

        if (loopStr == "loop") anim.loopMode = Loop;
        else if (loopStr == "once") anim.loopMode = Once;
        else {cout << "Unknown loop type: " << loopStr << endl; anim.loopMode = Once;};

        for (auto& [uuid, animatorData] : animData["animators"].items()) {
            Animator animator;
            animator.name = animatorData["name"];

            anim.allowedBones.insert(animator.name);

            for (const auto& kfData : animatorData["keyframes"]) {
                Keyframe kf;
                kf.time = kfData["time"];

                std::string chStr = kfData["channel"];
                if (chStr == "position") kf.channel = Channel::Position;
                else if (chStr == "rotation") kf.channel = Channel::Rotation;
                else if (chStr == "scale") kf.channel = Channel::Scale;
                else {cout << "Unknown animation channel: " << chStr << endl; continue;};

                kf.interpolation = Interpolation::Linear;
                //TODO: Fix interpolation

                auto dp = kfData["data_points"][0];
                kf.value = {
                    std::stof(static_cast<std::string>(dp["x"])),
                    std::stof(static_cast<std::string>(dp["y"])),
                    std::stof(static_cast<std::string>(dp["z"]))
                };

                if (kf.channel == Channel::Position) {kf.value /= 16.0f;}

                animator.keyframes.push_back(kf);
            }

            anim.animators.push_back(animator);
        }

        animations.push_back(anim);
    }

    if (data.contains("outliner")) {
        for (const auto& entry : data["outliner"]) {
            if (entry.is_object()) {
                std::string parentName = entry["name"];
                if (entry.contains("children")) {
                    for (const auto& child : entry["children"]) {
                        if (child.is_object() && child.contains("name")) {
                            boneParents[child["name"]] = parentName;
                        }
                    }
                }
            }
        }
    }

    auto m = Model(meshes, animations, boneParents);
    models.insert({filePath, m});
    saveCBModel(("output/compiled_models/" + filePath + ".cbmodel"), m);
    return m;
}

void ModelUtil::loadModels(bool forceRegen) {
    //INSANE performance increase... 200,000 faces go from 20-30 secs loading to >2.5 secs (does not apply to startup/generation)
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
    if (models[name].meshes.size() == 0 && name != "unknown") {
        cerr << "Unknown model: " << name << endl;
        return getModel("unknown");
    }
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

        writeString(out, mesh.boneName);
    }

    uint32_t animCount = model.animations.size();
    out.write(reinterpret_cast<char*>(&animCount), sizeof(animCount));

    for (const Animation &anim : model.animations) {
        writeString(out, anim.name);
        out.write(reinterpret_cast<const char*>(&anim.length), sizeof(anim.length));
        out.write(reinterpret_cast<const char*>(&anim.loopMode), sizeof(uint8_t));

        uint32_t animatorCount = anim.animators.size();
        out.write(reinterpret_cast<char*>(&animatorCount), sizeof(animatorCount));

        for (const Animator &animator : anim.animators) {
            writeString(out, animator.name);

            uint32_t keyframeCount = animator.keyframes.size();
            out.write(reinterpret_cast<char*>(&keyframeCount), sizeof(keyframeCount));

            for (const Keyframe &kf : animator.keyframes) {
                out.write(reinterpret_cast<const char*>(&kf.time), sizeof(kf.time));

                uint8_t ch = static_cast<uint8_t>(kf.channel);
                uint8_t interp = static_cast<uint8_t>(kf.interpolation);
                out.write(reinterpret_cast<char*>(&ch), sizeof(ch));
                out.write(reinterpret_cast<char*>(&interp), sizeof(interp));

                out.write(reinterpret_cast<const char*>(&kf.value), sizeof(Vec3));
            }

            uint32_t allowedBoneCount = anim.allowedBones.size();
            out.write(reinterpret_cast<const char*>(&allowedBoneCount), sizeof(allowedBoneCount));
            for (const auto& bone : anim.allowedBones) {
                writeString(out, bone);
            }
        }
    }

    uint32_t boneParentCount = model.boneParents.size();
    out.write(reinterpret_cast<const char*>(&boneParentCount), sizeof(boneParentCount));
    for (const auto& [child, parent] : model.boneParents) {
        writeString(out, child);
        writeString(out, parent);
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

            std::string boneName = readString(in);

            meshes.emplace_back(vertices, indices, boneName);
        }

        vector<Animation> animations;

        uint32_t animCount = 0;
        if (!in.eof()) {  // check if theres animation data
            in.read(reinterpret_cast<char*>(&animCount), sizeof(animCount));
        }

        for (uint32_t i = 0; i < animCount; ++i) {
            Animation anim;
            anim.name = readString(in);
            in.read(reinterpret_cast<char*>(&anim.length), sizeof(anim.length));
            in.read(reinterpret_cast<char*>(&anim.loopMode), sizeof(uint8_t));

            uint32_t animatorCount;
            in.read(reinterpret_cast<char*>(&animatorCount), sizeof(animatorCount));
            anim.animators.resize(animatorCount);

            for (Animator &animator : anim.animators) {
                animator.name = readString(in);

                uint32_t keyframeCount;
                in.read(reinterpret_cast<char*>(&keyframeCount), sizeof(keyframeCount));
                animator.keyframes.resize(keyframeCount);

                for (Keyframe &kf : animator.keyframes) {
                    in.read(reinterpret_cast<char*>(&kf.time), sizeof(kf.time));

                    uint8_t ch, interp;
                    in.read(reinterpret_cast<char*>(&ch), sizeof(ch));
                    in.read(reinterpret_cast<char*>(&interp), sizeof(interp));

                    kf.channel = static_cast<Channel>(ch);
                    kf.interpolation = static_cast<Interpolation>(interp);

                    in.read(reinterpret_cast<char*>(&kf.value), sizeof(Vec3));
                }

                uint32_t allowedBoneCount;
                in.read(reinterpret_cast<char*>(&allowedBoneCount), sizeof(allowedBoneCount));
                for (uint32_t i = 0; i < allowedBoneCount; ++i) {
                    anim.allowedBones.insert(readString(in));
                }
            }

            animations.push_back(anim);
        }

        uint32_t boneParentCount = 0;
        if (!in.eof()) {  // check if theres bone parent data so no animations work
            in.read(reinterpret_cast<char*>(&boneParentCount), sizeof(boneParentCount));
        }
        std::unordered_map<std::string, std::string> boneParents;
        for (uint32_t i = 0; i < boneParentCount; ++i) {
            std::string child = readString(in);
            std::string parent = readString(in);
            boneParents[child] = parent;
        }

        in.close();
        return Model(meshes, animations, boneParents);
    } catch (const std::exception &e) {
        std::cerr << "Failed to read CBModel " << filepath << ": " << e.what() << std::endl;
    };
    return Model();
}
