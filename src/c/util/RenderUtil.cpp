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
#include "../../../libs/glm/mat4x4.hpp"
#include "../../../libs/glm/ext/matrix_transform.hpp"
#include "../../../libs/glm/ext/matrix_clip_space.hpp"
#include "../../../libs/glm/ext/scalar_constants.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "../../../libs/stb_image.h"

using namespace std;

unsigned int textureId = 0;

string RenderUtil::loadShaderSource(const char* filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Failed to open shader file: " << filePath << endl;
        return "";
    }

    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint RenderUtil::loadShader(const char* filePath, GLenum type) {
    string srcStr = loadShaderSource(filePath);
    const char* src = srcStr.c_str();

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

GLuint RenderUtil::createShaderProgram(const char* vertexSrc, const char* fragmentSrc) {
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

GLuint RenderUtil::genTexture(int width, int height, int nrChannels, string path) {
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    textureId++;
    return textureId-1;
}