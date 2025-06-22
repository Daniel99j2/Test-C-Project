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
    std::string key = std::filesystem::path(path).generic_string(); // normalize to use forward slashes
    auto it = atlasRegions.find("src/resources/textures/"+key);
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

GLuint RenderUtil::genOrLoadAtlas(const std::string &folder, const std::string &atlasPng, const std::string &atlasMeta, bool forceRegenerate) {
    // Reuse if it already exists
    if (!forceRegenerate && filesystem::exists(atlasPng) && filesystem::exists(atlasMeta)) {
        int w, h, c;
        unsigned char *data = stbi_load(atlasPng.c_str(), &w, &h, &c, 4);
        if (!data) {
            std::cerr << "Failed to load atlas PNG.\n";
            return 0;
        }

        atlasWidth = w;
        atlasHeight = h;

        glGenTextures(1, &atlasID);
        glBindTexture(GL_TEXTURE_2D, atlasID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);

        // Parse metadata
        std::ifstream in(atlasMeta);
        nlohmann::json j;
        in >> j;
        atlasWidth = j["atlasWidth"];
        atlasHeight = j["atlasHeight"];
        atlasRegions.clear();

        for (auto &[key, val] : j["textures"].items()) {
            std::string normPath = std::filesystem::path(key).generic_string();
            RenderUtil::AtlasRegion r;
            r.uvMin = glm::vec2(val["uvMin"][0], val["uvMin"][1]);
            r.uvMax = glm::vec2(val["uvMax"][0], val["uvMax"][1]);
            r.width = int((r.uvMax.x - r.uvMin.x) * atlasWidth);
            r.height = int((r.uvMax.y - r.uvMin.y) * atlasHeight);
            atlasRegions[normPath] = r;
        }

        return atlasID;
    }

    // Rebuild atlas
    struct ImageData {
        std::string path;
        int w, h, c;
        unsigned char *data;
    };

    std::vector<ImageData> images;
    for (auto &entry : filesystem::directory_iterator(folder)) {
        if (entry.path().extension() == ".png") {
            std::string p = entry.path().string();
            int w, h, c;
            unsigned char *data = stbi_load(p.c_str(), &w, &h, &c, 4);
            if (data) {
                images.push_back({std::filesystem::path(p).generic_string(), w, h, 4, data});
            } else {
                std::cerr << "Failed to load image: " << p << '\n';
            }
        }
    }

    const int maxWidth = 2048, padding = 2;
    int x = padding, y = padding, rowH = 0;
    atlasWidth = maxWidth;

    for (auto &img : images) {
        if (x + img.w + padding > maxWidth) {
            y += rowH + padding;
            x = padding;
            rowH = 0;
        }

        atlasRegions[img.path] = {
            glm::vec2(float(x), float(y)), {}, img.w, img.h
        };

        x += img.w + padding;
        rowH = std::max(rowH, img.h);
    }

    atlasHeight = y + rowH + padding;

    auto *atlasData = new unsigned char[atlasWidth * atlasHeight * 4];
    std::fill(atlasData, atlasData + atlasWidth * atlasHeight * 4, 0);

    for (auto &img : images) {
        auto &region = atlasRegions[img.path];
        int px = int(region.uvMin.x);
        int py = int(region.uvMin.y);

        for (int j = 0; j < img.h; ++j) {
            for (int i = 0; i < img.w; ++i) {
                if (px + i < atlasWidth && py + j < atlasHeight) {
                    int dst = ((py + j) * atlasWidth + (px + i)) * 4;
                    int src = (j * img.w + i) * 4;
                    memcpy(&atlasData[dst], &img.data[src], 4);
                }
            }
        }

        region.uvMin /= glm::vec2(atlasWidth, atlasHeight);
        region.uvMax = region.uvMin + glm::vec2(float(img.w), float(img.h)) / glm::vec2(atlasWidth, atlasHeight);
        stbi_image_free(img.data);
    }

    // Create output dir
    filesystem::create_directories(filesystem::path(atlasMeta).parent_path());

    // Save image first (before freeing memory)
    if (!stbi_write_png(atlasPng.c_str(), atlasWidth, atlasHeight, 4, atlasData, atlasWidth * 4)) {
        std::cerr << "Failed to write PNG to " << atlasPng << '\n';
    }

    // Upload OpenGL
    glGenTextures(1, &atlasID);
    glBindTexture(GL_TEXTURE_2D, atlasID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlasWidth, atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlasData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    delete[] atlasData;

    // Save metadata
    nlohmann::json j;
    j["atlasWidth"] = atlasWidth;
    j["atlasHeight"] = atlasHeight;

    for (auto &[path, r] : atlasRegions) {
        j["textures"][path] = {
            {"uvMin", {r.uvMin.x, r.uvMin.y}},
            {"uvMax", {r.uvMax.x, r.uvMax.y}}
        };
    }

    std::ofstream out(atlasMeta);
    if (!out.is_open()) {
        std::cerr << "Couldn't write atlas metadata: " << atlasMeta << "\n";
    } else {
        out << j.dump(2);
    }

    return atlasID;
}
