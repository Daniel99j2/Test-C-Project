//
// Created by dj on 13/06/2025.
//

#ifndef RENDERUTIL_H
#define RENDERUTIL_H
#include <chrono>
#include "../../../libs/glew/include/GL/glew.h"
#include "../../../libs/glm/mat4x4.hpp"
#include "../../../libs/glm/ext/matrix_transform.hpp"
#include "../../../libs/glm/ext/matrix_clip_space.hpp"

class RenderUtil {
public:
    static GLuint loadShader(const char *filePath, GLenum type);

    static std::string loadShaderSource(const char *filePath);

    static GLuint createShaderProgram(const char * str, const char * text);

    static GLuint  genTexture(int width, int height, int nrChannels, std::string path);
};



#endif //RENDERUTIL_H
