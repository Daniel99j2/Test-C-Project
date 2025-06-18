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

GLuint RenderUtil::genTexture(string path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
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
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}