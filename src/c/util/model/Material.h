//
// Created by dj on 11/07/2025.
//

#pragma once
#include <iostream>
#include <stb_image.h>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <GL/glew.h>
#include "RenderUtil.h"

class Material {
public:
	GLuint base;
	GLuint metal;
	GLuint rough;
	GLuint emissive;
	GLuint normal;
	std::string name;

	explicit Material(const aiMaterial* other, std::map<std::string, GLuint> textures) {
		base = get(*other, aiTextureType_DIFFUSE, AI_MATKEY_COLOR_DIFFUSE, textures);
		metal = get(*other, aiTextureType_METALNESS, AI_MATKEY_METALLIC_FACTOR, textures);
		rough = get(*other, aiTextureType_DIFFUSE_ROUGHNESS, AI_MATKEY_ROUGHNESS_FACTOR, textures);
		emissive = get(*other, aiTextureType_EMISSIVE, AI_MATKEY_EMISSIVE_INTENSITY, textures);
		normal = get(*other, aiTextureType_NORMALS, "", 0, 0, textures);
		name = other->GetName().C_Str();
	}

	Material() {};

private:

	static GLuint get(const aiMaterial &material, aiTextureType textureType, const char * str, int scene1, int i, std::map<std::string, GLuint> textures) {
		aiString texturePath;
		auto format = aiTextureType_DIFFUSE ? GL_SRGB : GL_RGB;

		if (material.GetTexture(textureType, 0, &texturePath) == AI_SUCCESS) {
			return textures[texturePath.C_Str()];
		}

		aiColor4D fallback = {0xAA/255.0f,0xFF/255.0f,0xb7/255.0f,1};
		if (str != "") aiGetMaterialColor(&material, str, scene1, i, &fallback);
		return RenderUtil::genFromSolidColour(glm::vec4(fallback.r, fallback.g, fallback.b, fallback.a), format);
	}
};
