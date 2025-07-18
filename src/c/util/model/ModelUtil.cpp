//
// Created by dj on 20/06/2025.
//

#include "ModelUtil.h"
#include <iostream>
#include <chrono>
#include <random>
#include "../../../../libs/json.hpp"
// ReSharper disable once CppUnusedIncludeDirective
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <future>
#include <sstream>

#include "Animation.h"
#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../../objects/GameObject.h"
#include "../../world/World.h"

using namespace std;

map<std::string, Model> models = {};

Model invalidModel = Model();

static glm::vec3 aiVecToGlm(const aiVector3D& v) {
    return glm::vec3(v.x, v.y, v.z);
}

static glm::vec2 aiVec2ToGlm(const aiVector3D& v) {
    return glm::vec2(v.x, v.y);
}

static glm::quat aiQuatToGlm(const aiQuaternion& q) {
    return glm::quat(q.w, q.x, q.y, q.z);
}

static glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& m) {
    return glm::mat4(
        m.a1, m.b1, m.c1, m.d1,
        m.a2, m.b2, m.c2, m.d2,
        m.a3, m.b3, m.c3, m.d3,
        m.a4, m.b4, m.c4, m.d4
    );
}

static std::vector<Vertex> processVertices(aiMesh* mesh) {
    std::vector<Vertex> vertices(mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex& v = vertices[i];
        v.Position = aiVecToGlm(mesh->mVertices[i]);
        v.Normal = mesh->HasNormals() ? aiVecToGlm(mesh->mNormals[i]) : glm::vec3(0);
        v.TexCoords = mesh->HasTextureCoords(0) ? aiVec2ToGlm(mesh->mTextureCoords[0][i]) : glm::vec2(0);
        v.Tangent = mesh->HasTangentsAndBitangents() ? aiVecToGlm(mesh->mTangents[i]) : glm::vec3(0);
        v.Bitangent = mesh->HasTangentsAndBitangents() ? aiVecToGlm(mesh->mBitangents[i]) : glm::vec3(0);
    }

    return vertices;
}

static std::vector<unsigned int> processIndices(aiMesh* mesh) {
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    return indices;
}

Model ModelUtil::loadModelFromFile(const std::string& path) {
    Assimp::Importer importer;
    //importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS | aiComponent_TANGENTS_AND_BITANGENTS);
    const aiScene* scene = importer.ReadFile(("src/resources/models/" + path), aiProcess_Triangulate | aiProcess_PreTransformVertices);

    importer.ApplyPostProcessing(aiProcess_OptimizeMeshes | aiProcess_RemoveRedundantMaterials | aiProcess_EmbedTextures);

    importer.ApplyPostProcessing(aiProcess_GenSmoothNormals | aiProcess_GenUVCoords);
    importer.ApplyPostProcessing(aiProcess_CalcTangentSpace | aiProcess_FlipUVs);

    importer.ApplyPostProcessing(
        aiProcess_ValidateDataStructure |
        aiProcess_GenBoundingBoxes |
        aiProcess_FindInvalidData
        );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "[ERROR] [ModelUtil] ASSIMP: " << importer.GetErrorString() << std::endl;
        return {};
    }

    Model modelOutput;

    struct TextureData {
        int width, height, channels;
        unsigned char *data;
        std::string name;
    };

    std::vector<std::future<void>> textureDataFutures;

    std::vector<TextureData> textureData;

    std::map<std::string, GLuint> textures;

    textureData.resize(scene->mNumTextures);

    for (int i = 0; i < scene->mNumTextures; ++i) {
        textureDataFutures.push_back(std::async(std::launch::async, [i, &scene, &textureData]() {
            int w, h, c;
            unsigned char *data = stbi_load_from_memory(
                reinterpret_cast<stbi_uc*>(scene->mTextures[i]->pcData),
                scene->mTextures[i]->mWidth,
                &w, &h, &c,
                STBI_rgb_alpha);

            textureData[i] = TextureData(w, h, c, data, ("*" + std::to_string(i)));
            //scene->mTextures[i]->mFilename.C_Str()
        }));
    }

    for (auto& f : textureDataFutures) f.get();

    for (auto& t : textureData) {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, t.width, t.height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, t.data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(t.data);

        textures[t.name] = textureID;
    }

    for (int i = 0; i < scene->mNumMaterials; ++i) {
        modelOutput.materials[scene->mMaterials[i]->GetName().C_Str()] = Material(scene->mMaterials[i], textures);
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* aiMesh = scene->mMeshes[i];
        auto vertices = processVertices(aiMesh);
        auto indices = processIndices(aiMesh);
        Mesh mesh(vertices, indices, aiMesh->mName.C_Str(), &modelOutput.materials[scene->mMaterials[aiMesh->mMaterialIndex]->GetName().C_Str()]);
        modelOutput.meshes.push_back(mesh);

        CollisionPart collision;

        collision.start = glm::vec3(aiMesh->mAABB.mMin.x, aiMesh->mAABB.mMin.y, aiMesh->mAABB.mMin.z);
        collision.end = glm::vec3(aiMesh->mAABB.mMax.x, aiMesh->mAABB.mMax.y, aiMesh->mAABB.mMax.z);

        modelOutput.collisions.push_back(collision);
    }

    if (scene->HasAnimations())
    for (int i = 0; i < scene->mNumAnimations; ++i) {
        aiAnimation* animation = scene->mAnimations[i];
        Animation out = Animation();
        out.name = animation->mName.C_Str();
        out.length = animation->mDuration * animation->mTicksPerSecond;
        out.loopMode = Loop;
        map<string, vector<Keyframe>> keyframes;

        for (int i2 = 0; i2 < animation->mNumChannels; ++i2) {
            aiNodeAnim* kfs = animation->mChannels[i2];

            for (int i3 = 0; i3 < kfs->mNumPositionKeys; ++i3) {
                aiVectorKey kf = kfs->mPositionKeys[i3];
                Keyframe newKf = Keyframe();
                newKf.channel = Position;
                newKf.interpolation = Linear;
                newKf.time = kf.mTime * animation->mTicksPerSecond;
                newKf.value = glm::vec3(kf.mValue.x, kf.mValue.y, kf.mValue.z);

                keyframes[kfs->mNodeName.C_Str()].push_back(newKf);

                out.allowedBones.insert(kfs->mNodeName.C_Str());
            }

            for (int i3 = 0; i3 < kfs->mNumRotationKeys; ++i3) {
                aiQuatKey kf = kfs->mRotationKeys[i3];
                Keyframe newKf = Keyframe();
                newKf.channel = Rotation;
                newKf.interpolation = Linear;
                newKf.time = kf.mTime * animation->mTicksPerSecond;
                newKf.value = glm::vec3(kf.mValue.x, kf.mValue.y, kf.mValue.z);

                keyframes[kfs->mNodeName.C_Str()].push_back(newKf);

                out.allowedBones.insert(kfs->mNodeName.C_Str());
            }

            for (int i3 = 0; i3 < kfs->mNumScalingKeys; ++i3) {
                aiVectorKey kf = kfs->mScalingKeys[i3];
                Keyframe newKf = Keyframe();
                newKf.channel = Rotation;
                newKf.interpolation = Linear;
                newKf.time = kf.mTime * animation->mTicksPerSecond;
                newKf.value = glm::vec3(kf.mValue.x, kf.mValue.y, kf.mValue.z);

                keyframes[kfs->mNodeName.C_Str()].push_back(newKf);

                out.allowedBones.insert(kfs->mNodeName.C_Str());
            }
        }
        out.keyframes = keyframes;

        modelOutput.animations.push_back(out);
    }

    return modelOutput;
}

void ModelUtil::loadModels() {
    for (auto &entry: std::filesystem::recursive_directory_iterator("src/resources/models/")) {
        std::string extension = entry.path().extension().string();
        if (extension == ".gltf" || extension == ".glb") {
            std::string path = entry.path().string();
            std::string key = "models/";
            std::string out = "error";

            size_t pos = path.rfind(key);
            if (pos != std::string::npos) {
                out = path.substr(pos + key.length());

                if (out.size() >= extension.size() &&
                    out.compare(out.size() - extension.size(), extension.size(), extension) == 0) {
                    out = out.substr(0, out.size() - extension.size());
                }
            }

            models[out] = loadModelFromFile(out + extension);
        }
    }
}

Model* ModelUtil::getModel(const string &name) {
    if ((models[name].meshes.empty()) && name != "unknown") {
        cerr << "[ERROR] [ModelUtil] Unknown model: " << name << endl;
        return getModel("unknown");
    }
    return &models[name];
}
