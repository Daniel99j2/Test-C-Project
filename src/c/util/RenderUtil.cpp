#include "RenderUtil.h"

#include <iostream>
#include <chrono>
#include <random>
#include "../../../libs/glew/include/GL/glew.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include "../../../libs/glm/vec3.hpp"
#include "../../../libs/glm/vec4.hpp"
#include "../../../libs/glm/ext/matrix_transform.hpp"
#include "../../../libs/glm/ext/matrix_clip_space.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "../../../libs/stb_image.h"
#include "libs/json.hpp"
#include <libs/json.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <../../../libs/stb_image_write.h>

using namespace std;

GLuint atlasID = 0;
int atlasWidth = 0, atlasHeight = 0;
std::unordered_map<std::string, RenderUtil::AtlasRegion> atlasRegions;
GLuint merAtlasID = 0;
std::unordered_map<std::string, RenderUtil::AtlasRegion> merAtlasRegions;

string RenderUtil::loadShaderSource(const char *filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Failed to open shader file: " << filePath << endl;
        return "";
    }

    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint RenderUtil::loadShader(const char *filePath, GLenum type) {
    string srcStr = loadShaderSource(filePath);
    const char *src = srcStr.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        cerr << "Shader compile error:\n" << log << endl;
    }

    return shader;
}

GLuint RenderUtil::createShaderProgram(const char *vertexSrc, const char *fragmentSrc) {
    GLuint vertexShader = loadShader(vertexSrc, GL_VERTEX_SHADER);
    GLuint fragmentShader = loadShader(fragmentSrc, GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

//dont use unless needed
GLuint RenderUtil::genTexture(string path) {
    path = path + ".png";
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

//use an image path. Returns -1 if null
GLuint RenderUtil::genPBR(string path) {
    path = path + ".texture_set.json";
    ifstream file(path);
    if (!file.is_open()) {
        return -1;
    }

    stringstream buffer;
    buffer << file.rdbuf();
    nlohmann::json data = nlohmann::json::parse(buffer.str());

    string path1;

    size_t pos = path.find_last_of('/');

    if (pos != std::string::npos) {
        path1 = path.substr(0, pos + 1);
    }

    return genTexture((path1 + static_cast<string>(data["minecraft:texture_set"]["metalness_emissive_roughness"])));
}

glm::vec2 RenderUtil::getUV(const std::string &path, const glm::vec2 &originalUV) {
    if (atlasRegions.empty()) throw("Cannot access atlas before creation!!!");

    std::string key = std::filesystem::path(path).generic_string(); // normalize to use forward slashes
    auto it = atlasRegions.find("src/resources/textures/" + key);
    if (it == atlasRegions.end()) {
        std::cerr << "Texture not found in atlas: " << key << endl;
        return originalUV;
    }
    const auto &r = it->second;
    return r.uvMin + (r.uvMax - r.uvMin) * originalUV;
}

RenderUtil::AtlasRegion *getRegion(const std::string &path) {
    auto it = atlasRegions.find(path);
    if (it != atlasRegions.end()) return &it->second;
    return nullptr;
}

unsigned char *loadMERTextureOrFallback(const std::string &basePath, int &w, int &h, int fw, int fh) {
    std::string jsonPath = basePath + ".texture_set.json";
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        w = fw;
        h = fh;
        unsigned char *fallback = new unsigned char[w * h * 4];
        for (int i = 0; i < w * h; ++i) {
            fallback[i * 4 + 0] = 0x20; // metal
            fallback[i * 4 + 1] = 0x01; // emissive
            fallback[i * 4 + 2] = 0x80; // rough
            fallback[i * 4 + 3] = 0xFF; // transparency
        }
        return fallback;
    }

    nlohmann::json data;
    file >> data;
    std::string relMER = data["minecraft:texture_set"]["metalness_emissive_roughness"];
    std::filesystem::path baseDir = std::filesystem::path(basePath).parent_path();
    std::string fullMER = (baseDir / (relMER + ".png")).generic_string();

    int c;
    unsigned char *img = stbi_load(fullMER.c_str(), &w, &h, &c, 4);
    if (!img) {
        w = fw;
        h = fh;
        unsigned char *fallback = new unsigned char[w * h * 4];
        for (int i = 0; i < w * h; ++i) {
            fallback[i * 4 + 0] = 0x20;
            fallback[i * 4 + 1] = 0x01;
            fallback[i * 4 + 2] = 0x80;
            fallback[i * 4 + 3] = 0xFF;
        }
        return fallback;
    }

    return img;
}

void RenderUtil::genOrLoadAtlas(const std::string &folder, const std::string &atlasPng, const std::string &atlasMeta,
                                  const std::string &merPng, const std::string &merMeta, bool forceRegenerate) {
        if (!forceRegenerate) {
        int w1, h1, c1, w2, h2, c2;
        unsigned char *atlasData = stbi_load(atlasPng.c_str(), &w1, &h1, &c1, 4);
        unsigned char *merData = stbi_load(merPng.c_str(), &w2, &h2, &c2, 4);

        if (atlasData && merData && w1 == w2 && h1 == h2) {
            atlasWidth = w1;
            atlasHeight = h1;

            auto readMeta = [](const std::string &path, std::unordered_map<std::string, RenderUtil::AtlasRegion> &regions) {
                std::ifstream in(path);
                if (!in.is_open()) return false;

                nlohmann::json j;
                in >> j;

                if (!j.contains("textures")) return false;

                for (auto &[k, v] : j["textures"].items()) {
                    glm::vec2 uvMin = {v["uvMin"][0], v["uvMin"][1]};
                    glm::vec2 uvMax = {v["uvMax"][0], v["uvMax"][1]};
                    regions[k] = {
                        .uvMin = uvMin,
                        .uvMax = uvMax,
                        .width = static_cast<int>((uvMax.x - uvMin.x) * atlasWidth),
                        .height = static_cast<int>((uvMax.y - uvMin.y) * atlasHeight)
                    };
                }
                return true;
            };

            if (readMeta(atlasMeta, atlasRegions) && readMeta(merMeta, merAtlasRegions)) {
                auto uploadTex = [](GLuint &id, unsigned char *data) {
                    glGenTextures(1, &id);
                    glBindTexture(GL_TEXTURE_2D, id);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlasWidth, atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                    glGenerateMipmap(GL_TEXTURE_2D);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                };

                uploadTex(atlasID, atlasData);
                uploadTex(merAtlasID, merData);
                stbi_image_free(atlasData);
                stbi_image_free(merData);
                return;
            }

            stbi_image_free(atlasData);
            stbi_image_free(merData);
            std::cerr << "Failed to load atlas metadata, regenerating...\n";
        } else {
            std::cerr << "Failed to load existing atlases, regenerating...\n";
            if (atlasData) stbi_image_free(atlasData);
            if (merData) stbi_image_free(merData);
        }
    }

    //if we dont have a cache
    struct ImageData {
        std::string path;
        int w, h;
        unsigned char *data;
        unsigned char *merData;
    };

    std::vector<ImageData> images;
    for (auto &entry: std::filesystem::recursive_directory_iterator(folder)) {
        if (entry.path().extension() == ".png") {
            std::string path = entry.path().string();
            int w, h, c;
            unsigned char *data = stbi_load(path.c_str(), &w, &h, &c, 4);
            if (data) {
                int merW = 0, merH = 0;
                unsigned char *merData = loadMERTextureOrFallback(path.substr(0, path.size() - 4), merW, merH, w, h);
                if (merW != w || merH != h) {
                    std::cerr << "MER texture size mismatch for " << path << std::endl;
                }
                images.push_back({std::filesystem::path(path).generic_string(), w, h, data, merData});
            }
        }
    }

    const int maxWidth = 2048, padding = 2;
    int x = padding, y = padding, rowH = 0;
    atlasWidth = maxWidth;

    for (auto &img: images) {
        if (x + img.w + padding > maxWidth) {
            y += rowH + padding;
            x = padding;
            rowH = 0;
        }
        glm::vec2 uvMin = {float(x), float(y)};
        glm::vec2 uvMax = {float(x + img.w), float(y + img.h)};
        RenderUtil::AtlasRegion region;
        region.uvMin = uvMin;
        region.uvMax = uvMax;
        region.width = img.w;
        region.height = img.h;

        atlasRegions[img.path] = region;
        merAtlasRegions[img.path] = region;

        x += img.w + padding;
        rowH = std::max(rowH, img.h);
    }

    atlasHeight = y + rowH + padding;

    auto *atlasData = new unsigned char[atlasWidth * atlasHeight * 4]();
    auto *merData = new unsigned char[atlasWidth * atlasHeight * 4]();

    for (auto &img: images) {
        auto &region = atlasRegions[img.path];
        int px = static_cast<int>(region.uvMin.x);
        int py = static_cast<int>(region.uvMin.y);

        for (int j = 0; j < img.h; ++j) {
            for (int i = 0; i < img.w; ++i) {
                int dst = ((py + j) * atlasWidth + (px + i)) * 4;
                int src = (j * img.w + i) * 4;
                memcpy(&atlasData[dst], &img.data[src], 4);
                memcpy(&merData[dst], &img.merData[src], 4);
            }
        }

        stbi_image_free(img.data);
        delete[] img.merData;
    }

    auto uploadTex = [](GLuint &id, unsigned char *data) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlasWidth, atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    };

    uploadTex(atlasID, atlasData);
    uploadTex(merAtlasID, merData);

    // Normalize UVs AFTER uploading
    for (auto &[_, region]: atlasRegions) {
        region.uvMin /= glm::vec2(atlasWidth, atlasHeight);
        region.uvMax /= glm::vec2(atlasWidth, atlasHeight);
    }
    for (auto &[_, region]: merAtlasRegions) {
        region.uvMin /= glm::vec2(atlasWidth, atlasHeight);
        region.uvMax /= glm::vec2(atlasWidth, atlasHeight);
    }

    // Save to disk
    std::filesystem::create_directories(std::filesystem::path(atlasPng).parent_path());
    std::filesystem::create_directories(std::filesystem::path(merPng).parent_path());

    if (!stbi_write_png(atlasPng.c_str(), atlasWidth, atlasHeight, 4, atlasData, atlasWidth * 4))
        std::cerr << "Failed to write main atlas PNG: " << atlasPng << '\n';

    if (!stbi_write_png(merPng.c_str(), atlasWidth, atlasHeight, 4, merData, atlasWidth * 4))
        std::cerr << "Failed to write MER atlas PNG: " << merPng << '\n';

    delete[] atlasData;
    delete[] merData;

    auto writeMeta = [](const std::string &path, const std::unordered_map<std::string, RenderUtil::AtlasRegion> &regions) {
        nlohmann::json j;
        j["atlasWidth"] = atlasWidth;
        j["atlasHeight"] = atlasHeight;
        for (auto &[p, r]: regions) {
            j["textures"][p] = {
                {"uvMin", {r.uvMin.x, r.uvMin.y}},
                {"uvMax", {r.uvMax.x, r.uvMax.y}}
            };
        }
        std::ofstream out(path);
        out << j.dump(2);
    };

    writeMeta(atlasMeta, atlasRegions);
    writeMeta(merMeta, merAtlasRegions);
}


GLuint RenderUtil::getAtlas() {
    return atlasID;
}

GLuint RenderUtil::getMERAtlas() {
    return merAtlasID;
}