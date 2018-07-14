#pragma once

#include "glad/glad.h"
#include "GlmCommon.h"

#include <vector>
#include <unordered_map>
#include <filesystem>

class TextureArray {
public:
	TextureArray() = default;
	TextureArray(const std::vector<std::filesystem::path>& paths, ivec2 size, int imageFormat = GL_RGB);

	void bind() const;
	bool containsTexture(const std::string& name) const;
	int getTextureID(const std::string& name) const;
private:
	GLuint textureArray;
	std::unordered_map<std::string, int> textureMap;

	void loadTexture(const std::string& name, int layerNumber, ivec2 size, int imageFormat) const;
};

