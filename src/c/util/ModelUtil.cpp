//
// Created by dj on 20/06/2025.
//

#include "ModelUtil.h"
#include <iostream>
#include <chrono>
#include <random>
#include "../../../libs/json.hpp"
// ReSharper disable once CppUnusedIncludeDirective
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>

#include "Animation.h"
#include "../util/Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../objects/GameObject.h"
#include "../world/World.h"

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

static void extractBoneData(aiMesh* mesh, std::unordered_map<std::string, std::vector<Mesh>>& boneMeshes, const Mesh& meshData) {
    for (unsigned int i = 0; i < mesh->mNumBones; ++i) {
        aiBone* bone = mesh->mBones[i];
        boneMeshes[bone->mName.C_Str()].push_back(meshData);
    }
}

static std::unordered_map<std::string, std::string> buildBoneHierarchy(const aiNode* node, const std::string& parent = "") {
    std::unordered_map<std::string, std::string> result;
    std::string nodeName = node->mName.C_Str();

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        std::string childName = node->mChildren[i]->mName.C_Str();
        result[childName] = nodeName;

        auto childMap = buildBoneHierarchy(node->mChildren[i], nodeName);
        result.insert(childMap.begin(), childMap.end());
    }

    return result;
}

void extractCollisionPartsFromCollection(
    const aiScene* scene,
    const aiNode* node,
    const glm::mat4& parentTransform,
    const std::string& collisionCollectionName,
    std::vector<CollisionPart>& outCollisionParts
) {
    glm::mat4 nodeTransform = parentTransform * aiMatrix4x4ToGlm(node->mTransformation);

    // Check if node belongs to the collision collection
    // Assimp does NOT import Blender Collections as collections directly.
    // But Blender exports collections as nodes named "CollectionName"
    // So check if node's name matches your collision collection name
    if (node->mName.C_Str() == collisionCollectionName) {
        // This node is the collision collection root,
        // so its children are collision shapes
        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            const aiNode* colNode = node->mChildren[i];

            for (unsigned int meshIdx = 0; meshIdx < colNode->mNumMeshes; ++meshIdx) {
                aiMesh* mesh = scene->mMeshes[colNode->mMeshes[meshIdx]];

                // Compute bounding box for the mesh
                aiVector3D bbMin = mesh->mVertices[0];
                aiVector3D bbMax = mesh->mVertices[0];
                for (unsigned int v = 1; v < mesh->mNumVertices; ++v) {
                    bbMin.x = std::min(bbMin.x, mesh->mVertices[v].x);
                    bbMin.y = std::min(bbMin.y, mesh->mVertices[v].y);
                    bbMin.z = std::min(bbMin.z, mesh->mVertices[v].z);

                    bbMax.x = std::max(bbMax.x, mesh->mVertices[v].x);
                    bbMax.y = std::max(bbMax.y, mesh->mVertices[v].y);
                    bbMax.z = std::max(bbMax.z, mesh->mVertices[v].z);
                }

                glm::vec3 min = glm::vec3(bbMin.x, bbMin.y, bbMin.z);
                glm::vec3 max = glm::vec3(bbMax.x, bbMax.y, bbMax.z);
                glm::vec3 size = max - min;

                // Use node transform for position and scale
                glm::vec3 position = glm::vec3(nodeTransform[3]); // Translation part of matrix
                glm::vec3 scale = glm::vec3(
                    glm::length(glm::vec3(nodeTransform[0])),
                    glm::length(glm::vec3(nodeTransform[1])),
                    glm::length(glm::vec3(nodeTransform[2]))
                );

                size *= scale;

                // Guess shape type based on name prefix of node (or mesh)
                ShapeType shape = ShapeType::Rectangle; // default box
                std::string nodeName = colNode->mName.C_Str();
                if (nodeName.find("sphere") != std::string::npos) {
                    shape = ShapeType::Sphere;
                } else if (nodeName.find("cylinder") != std::string::npos) {
                    shape = ShapeType::Cylinder;
                }

                outCollisionParts.push_back({ shape, size, position });
            }
        }
    } else {
        // Recurse children
        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            extractCollisionPartsFromCollection(scene, node->mChildren[i], nodeTransform, collisionCollectionName, outCollisionParts);
        }
    }
}

glm::vec3 getFinalOffset(
    const std::string& name,
    const std::unordered_map<std::string, glm::vec3>& offsets,
    const std::unordered_map<std::string, std::string>& parents)
{
    // Base offset for this object
    glm::vec3 offset = glm::vec3(0.0f);

    // Get own offset if present
    auto it = offsets.find(name);
    if (it != offsets.end()) {
        offset = it->second;
    }

    // Find parent and if exists, add parent's final offset recursively
    auto pIt = parents.find(name);
    if (pIt != parents.end()) {
        const std::string& parentName = pIt->second;
        offset += getFinalOffset(parentName, offsets, parents);
    }

    return offset;
}

static std::vector<Animation> loadAnimations(const aiScene* scene) {
    std::vector<Animation> animations;

    for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
        aiAnimation* a = scene->mAnimations[i];

        Animation anim;
        anim.name = a->mName.C_Str();
        anim.length = static_cast<float>(a->mDuration / a->mTicksPerSecond);
        anim.loopMode = Loop;

        for (unsigned int j = 0; j < a->mNumChannels; ++j) {
            aiNodeAnim* channel = a->mChannels[j];
            Animator animator;
            animator.name = channel->mNodeName.C_Str();

            for (unsigned int k = 0; k < channel->mNumPositionKeys; ++k) {
                Keyframe key;
                key.time = channel->mPositionKeys[k].mTime / a->mTicksPerSecond;
                key.channel = Channel::Position;
                key.interpolation = Linear;
                key.value = aiVecToGlm(channel->mPositionKeys[k].mValue);
                animator.keyframes.push_back(key);
            }

            for (unsigned int k = 0; k < channel->mNumRotationKeys; ++k) {
                Keyframe key;
                key.time = channel->mRotationKeys[k].mTime / a->mTicksPerSecond;
                key.channel = Channel::Rotation;
                key.interpolation = Linear;
                glm::vec3 euler = glm::degrees(glm::eulerAngles(aiQuatToGlm(channel->mRotationKeys[k].mValue)));
                key.value = euler;
                animator.keyframes.push_back(key);
            }

            for (unsigned int k = 0; k < channel->mNumScalingKeys; ++k) {
                Keyframe key;
                key.time = channel->mScalingKeys[k].mTime / a->mTicksPerSecond;
                key.channel = Channel::Scale;
                key.interpolation = Linear;
                key.value = aiVecToGlm(channel->mScalingKeys[k].mValue);
                animator.keyframes.push_back(key);
            }

            anim.animators.push_back(animator);
            anim.allowedBones.insert(animator.name);
        }

        animations.push_back(anim);
    }

    return animations;
}

Model ModelUtil::loadModelFromFile(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(("src/resources/models/" + path),
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_GenSmoothNormals |
        aiProcess_OptimizeMeshes |
        aiProcess_PreTransformVertices |
        aiProcess_ValidateDataStructure |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_OptimizeGraph |
        aiProcess_GenBoundingBoxes
        );

    cout << scene->mMeshes[0]->mAABB.mMax.x << endl;

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "[ERROR] [ModelUtil] ASSIMP: " << importer.GetErrorString() << std::endl;
        return {};
    }

    std::unordered_map<std::string, std::vector<Mesh>> boneMeshes;
    std::vector<Mesh> unboundMeshes;
    std::unordered_map<std::string, glm::vec3> origins;

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* aiMesh = scene->mMeshes[i];
        auto vertices = processVertices(aiMesh);
        auto indices = processIndices(aiMesh);
        Mesh mesh(vertices, indices);

        cout << vertices.size() << " vertices, " << indices.size() << " indices" << endl;
        for (unsigned int i = 0; i < std::min(aiMesh->mNumVertices, 5u); ++i) {
            const aiVector3D& p = aiMesh->mVertices[i];
            std::cout << "Vertex " << i << ": (" << p.x << ", " << p.y << ", " << p.z << ")\n";
        }

        // CollisionPart shape = fitPhysicsShapeForMesh(vertices);
        //
        // std::cout << "Mesh " << i << " best fit: ";
        // switch (shape.shape) {
        //     case ShapeType::Sphere:
        //         std::cout << "Sphere with radius " << shape.size.x << "\n";
        //         break;
        //     case ShapeType::Rectangle:
        //         std::cout << "Cube with size " << glm::to_string(shape.size) << "\n";
        //         break;
        //     case ShapeType::Cylinder:
        //         std::cout << "Cylinder with radius " << shape.size.x << ", height " << shape.size.y << "\n";
        //         break;
        // }


        if (aiMesh->HasBones()) {
            extractBoneData(aiMesh, boneMeshes, mesh);
            for (unsigned int b = 0; b < aiMesh->mNumBones; ++b)
                origins[aiMesh->mBones[b]->mName.C_Str()] = glm::vec3(0); // optional: use offset matrix
        } else {
            unboundMeshes.push_back(mesh);
        }
    }

    std::unordered_map<std::string, std::string> boneParents = buildBoneHierarchy(scene->mRootNode);
    std::vector<Animation> animations = loadAnimations(scene);

    std::vector<CollisionPart> collisionParts;
    extractCollisionPartsFromCollection(scene, scene->mRootNode, glm::mat4(1.0f), "Collision", collisionParts);

    return Model(boneMeshes, unboundMeshes, animations, boneParents, origins);
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
    if ((models[name].boneMeshes.empty()) && name != "unknown") {
        cerr << "[ERROR] [ModelUtil] Unknown model: " << name << endl;
        return getModel("unknown");
    }
    return &models[name];
}
