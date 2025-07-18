//
// Created by dj on 13/06/2025.
//

#pragma once
#include <chrono>
#include <stb_image.h>
#include <unordered_map>

#include <GLFW/glfw3.h>

class RenderUtil {
public:
	static GLuint loadShader(const char *filePath, GLenum type);

	static std::string loadShaderSource(const char *filePath);

	static GLuint createShaderProgram(const char *str, const char *text);

	static GLuint genTexture(std::string path);

	static GLuint genFromData(const stbi_uc *buffer, int bufferSize, GLenum format);

	static GLuint genFromSolidColour(glm::vec4 colour, GLenum format);

	static GLFWimage *getImageData(std::string path);

	static std::string getAtlasName(const std::string &path);

	static void genOrLoadAtlas(bool forceRegenerate);

	static glm::vec2 getUV(const std::string &texturePath, const glm::vec2 &originalUV);

	struct AtlasRegion {
		glm::vec2 uvMin; // (u, v)
		glm::vec2 uvMax; // (u, v)
		int width, height;
	};

	struct Atlas {
		int width, height, id;
		std::unordered_map<std::string, AtlasRegion> regions;
		std::string path;
	};
};
