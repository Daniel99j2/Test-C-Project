//
// Created by dj on 13/06/2025.
//

#pragma once
#include <chrono>
#include <unordered_map>

#include "../../../libs/glew/include/GL/glew.h"
#include <GLFW/glfw3.h>
#include "../../../libs/glm/mat4x4.hpp"
#include "../../../libs/glm/ext/matrix_transform.hpp"
#include "../../../libs/glm/ext/matrix_clip_space.hpp"

class RenderUtil {
public:
	static GLuint loadShader(const char *filePath, GLenum type);

	static std::string loadShaderSource(const char *filePath);

	static GLuint createShaderProgram(const char *str, const char *text);

	static GLuint genTexture(std::string path);

	static GLFWimage *getImageData(std::string path);

	static GLuint genPBR(std::string path);

	static void genOrLoadAtlas(const std::string &folder, const std::string &atlasPng, const std::string &atlasMeta,
	                           const std::string &merPng, const std::string &merMeta, bool forceRegenerate);

	static GLuint getAtlas();

	static GLuint getMERAtlas();

	static glm::vec2 getUV(const std::string &texturePath, const glm::vec2 &originalUV);

	struct AtlasRegion {
		glm::vec2 uvMin; // (u, v)
		glm::vec2 uvMax; // (u, v)
		int width, height;
	};
};
