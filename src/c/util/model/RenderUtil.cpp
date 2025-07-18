#include "../../PreImports.h"
#include "RenderUtil.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <future>
#include <stb_image_write.h>

#include "Animation.h"
#include "Animation.h"

using namespace std;
using namespace rectpack2D;

struct Vec4Hash {
    std::size_t operator()(const glm::vec4& v) const {
        std::size_t seed = 0;
        auto hasher = std::hash<float>();
        seed ^= hasher(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hasher(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hasher(v.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hasher(v.w) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

struct Vec4Equal {
    bool operator()(const glm::vec4& a, const glm::vec4& b) const {
        return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
    }
};

std::unordered_map<std::string, RenderUtil::Atlas> atlases;

std::unordered_map<glm::vec4, GLuint, Vec4Hash, Vec4Equal> solidColours;

bool isInvalidUV(glm::vec2 uv) {
    return uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f;
}

string RenderUtil::loadShaderSource(const char *filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "[ERROR] [Shader] Failed to open shader file: " << filePath << endl;
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
        cerr << "[ERROR] [Shader] Shader compile ERROR:" << endl << log << endl;
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
        std::cerr << "[WARN] [Texture] Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

GLuint RenderUtil::genFromData(const stbi_uc* buffer, int bufferSize, GLenum format) {
    int width, height, channels;

    unsigned char* data = stbi_load_from_memory(
        buffer,
        bufferSize,
        &width, &height, &channels,
        STBI_rgb_alpha
    );

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return textureID;
}

GLuint RenderUtil::genFromSolidColour(glm::vec4 colour, GLenum format) {
    if (solidColours.contains(colour)) return solidColours[colour];
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    unsigned char pixel[4] = {
        static_cast<unsigned char>(glm::clamp(colour.r, 0.0f, 1.0f) * 255.0f),
        static_cast<unsigned char>(glm::clamp(colour.g, 0.0f, 1.0f) * 255.0f),
        static_cast<unsigned char>(glm::clamp(colour.b, 0.0f, 1.0f) * 255.0f),
        static_cast<unsigned char>(glm::clamp(colour.a, 0.0f, 1.0f) * 255.0f)
    };

    glTexImage2D(GL_TEXTURE_2D, 0, format, 1, 1, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    solidColours[colour] = textureID;

    if (!stbi_write_png("test.png", 1, 1, 4, pixel, 4))
        std::cerr << "ERROR\n";

    return textureID;
}

//need to clear data once done!
GLFWimage *RenderUtil::getImageData(string path) {
    path = path + ".png";

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    return new GLFWimage(width, height, data);
}

std::string RenderUtil::getAtlasName(const std::string &path) {
    std::string folderName = "textures/";
    std::string out = "ERROR";

    std::string path2 = path;
    ranges::replace(path2, '\\', '/');

    size_t pos = path2.find(folderName);
    if (pos != std::string::npos) {
        pos += folderName.length();
        size_t end = path2.find('/', pos);
        if (end == std::string::npos) end = path2.length();

        out =  path2.substr(pos, end - pos);
    }
    
    return out;
}

RenderUtil::Atlas* getAtlas(const std::string &path) {
    if (atlases.empty()) throw std::runtime_error("Cannot access atlases before creation!!!");
    
    std::string out = RenderUtil::getAtlasName(path);
    
    if (!atlases.contains(out)) throw std::runtime_error("Cannot access atlas " + out + " before creation!!!");

    return &atlases[out];
}

glm::vec2 RenderUtil::getUV(const std::string &path, const glm::vec2 &originalUV) {
    if (path.find("_") != std::string::npos) return originalUV;

    RenderUtil::Atlas *atlas = getAtlas(path);

    std::string key = std::filesystem::path(path).lexically_normal().generic_string();
    
    auto it = atlas->regions.find(key);
    if (it == atlas->regions.end()) {
        std::cerr << "[ERROR] [Atlas] Texture not found in atlas: " << key << endl;
        return originalUV;
    }
    const auto &r = it->second;
    glm::vec2 finalUV = r.uvMin + (r.uvMax - r.uvMin) * originalUV;

    if (isInvalidUV(r.uvMax) || isInvalidUV(r.uvMin)) {
        std::cerr << "[ERROR] [Atlas] Atlas region UV out of bounds for " << path << std::endl;
    }

    if (isInvalidUV(finalUV) || isInvalidUV(finalUV)) {
        std::cerr << "[ERROR] [Atlas] Calculated UV out of bounds for " << path << std::endl;
    }

    return finalUV;
}

RenderUtil::AtlasRegion *getRegion(const std::string &path) {
    return &getAtlas(path)->regions.find(path)->second;
}

void RenderUtil::genOrLoadAtlas(bool forceRegenerate) {
    struct ImageData {
        std::string path;
        int w, h;
        unsigned char *data;
    };

    std::unordered_map<std::string, std::vector<ImageData>> requiredAtlases;

    for (auto &entry: std::filesystem::recursive_directory_iterator("src/resources/textures/")) {
        if (entry.path().extension() == ".png" && entry.path().generic_string().find('_') == std::string::npos) {
            std::string path = entry.path().string();
            int w, h, c;
            unsigned char *data = stbi_load(path.c_str(), &w, &h, &c, 4);
            if (data) {
                requiredAtlases[getAtlasName(path)].push_back({std::filesystem::path(path).generic_string(), w, h, data});
            }
        }
    }

    std::string folder = "output/atlases/";

    std::vector<std::future<void>> futures;
    std::mutex atlasMutex;

    for (auto &[atlasName, images]: requiredAtlases) {
        futures.push_back(std::async(std::launch::async, [&, atlasName, images]() {
            std::string atlasPng = (folder + "atlas_" + atlasName + ".png");
            std::string atlasMeta = (folder + "atlas_" + atlasName + ".json");

            int atlasWidth = 0, atlasHeight = 0;
            std::unordered_map<std::string, AtlasRegion> atlasRegions;
            GLuint atlasID;

            constexpr bool allow_flip = false;
            using spaces_type = empty_spaces<allow_flip, default_empty_spaces>;

            std::vector<rect_xywh> rects;
            rects.reserve(images.size());
            for (size_t i = 0; i < images.size(); ++i) {
                rects.push_back({0, 0, images[i].w, images[i].h});
            }

            constexpr int max_side = 16000;
            constexpr int discard_step = -4;
            auto finder_input = make_finder_input(
                max_side,
                discard_step,
                [](rect_xywh &) {
                    return rectpack2D::callback_result::CONTINUE_PACKING;
                },
                [](rect_xywh &) {
                    cerr << "[ERROR] ERROR packing!" << endl;
                    return rectpack2D::callback_result::ABORT_PACKING;
                },
                flipping_option::DISABLED
            );

            using spaces_type = empty_spaces<false, default_empty_spaces>;
            auto result_size = find_best_packing<spaces_type>(
                rects,
                finder_input
            );

            if (result_size.w == 0 || result_size.h == 0) {
                std::cerr << "Atlas generation failed!" << std::endl;
                return;
            }

            atlasWidth = result_size.w;
            atlasHeight = result_size.h;

            atlasRegions.clear();

            auto *atlasData = new unsigned char[atlasWidth * atlasHeight * 4]();

            // now we make the pngs
            //this grabs the texure back from the list - as the indexes are the same
            for (size_t i = 0; i < rects.size(); ++i) {
                const auto &r = rects[i];
                auto &img = images[i];
                AtlasRegion region;
                region.width = static_cast<int>(r.w);
                region.height = static_cast<int>(r.h);
                region.uvMin = glm::vec2(static_cast<float>(r.x), static_cast<float>(r.y));
                region.uvMax = glm::vec2(static_cast<float>(r.x + r.w), static_cast<float>(r.y + r.h));

                std::string textureRoot = "src/resources/textures/";
                std::string relativeKey = std::filesystem::relative(img.path, textureRoot).generic_string();
                atlasRegions[relativeKey] = region;

                for (int y = 0; y < img.h; ++y) {
                    int dstRowStart = ((r.y + y) * atlasWidth + r.x) * 4;
                    int srcRowStart = (y * img.w) * 4;
                    memcpy(&atlasData[dstRowStart], &img.data[srcRowStart], img.w * 4);
                }

                stbi_image_free(img.data);
            }

            auto uploadTex = [atlasWidth, atlasHeight](GLuint &id, unsigned char *data) {
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

            // now the uv
            for (auto &[_, region]: atlasRegions) {
                region.uvMin /= glm::vec2(atlasWidth, atlasHeight);
                region.uvMax /= glm::vec2(atlasWidth, atlasHeight);
            }

            // save it!
            std::filesystem::create_directories(std::filesystem::path(atlasPng).parent_path());

            if (!stbi_write_png(atlasPng.c_str(), atlasWidth, atlasHeight, 4, atlasData, atlasWidth * 4))
                std::cerr << "Failed to write main atlas PNG: " << atlasPng << '\n';

            auto writeMeta = [atlasWidth, atlasHeight](const std::string &path,
                                                       const std::unordered_map<std::string, RenderUtil::AtlasRegion> &
                                                       regions) {
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

            delete[] atlasData;

            Atlas a;
            a.regions = atlasRegions;
            a.width = atlasWidth;
            a.height = atlasHeight;
            a.id = atlasID;

            std::scoped_lock lock(atlasMutex);
            atlases.emplace(atlasName, a);
        }));
    }

    for (auto &f: futures) {
        f.get();
    }
}
