//
// Created by dj on 13/06/2025.
//

#ifndef RENDERUTIL_H
#define RENDERUTIL_H
#include <chrono>
#include "../../../libs/glew/include/GL/glew.h"

class RenderUtil {
public:
    static GLuint loadShader(const char *filePath, GLenum type);

    static std::string loadShaderSource(const char *filePath);

    static GLuint createShaderProgram(const char * str, const char * text);
};



#endif //RENDERUTIL_H
