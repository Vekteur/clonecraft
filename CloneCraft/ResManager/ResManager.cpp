#include "ResManager.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include "stb_image.h"

// Instantiate static variables
std::map<std::string, Texture2D>    ResManager::m_textures;
std::map<std::string, Shader>       ResManager::m_shaders;

BlockDatas ResManager::m_blockDatas;

const BlockDatas& ResManager::blockDatas() {
	return m_blockDatas;
}

void ResManager::setShader(Shader shader, std::string name) {
	m_shaders[name] = shader;
}

Shader ResManager::getShader(std::string name) {
	return m_shaders[name];
}

void ResManager::setTexture(Texture2D texture, std::string name) {
	m_textures[name] = texture;
}

Texture2D ResManager::getTexture(std::string name) {
	return m_textures[name];
}

void ResManager::clear() {
	// Properly delete all shaders
	for (auto iter : m_shaders)
		glDeleteProgram(iter.second.getId());
	// Properly delete all textures
	for (auto iter : m_textures) {
		GLuint id{ iter.second.getId() };
		glDeleteTextures(1, &id);
	}
}

std::string ResManager::readFile(std::string fileName) {
	std::ifstream ifs(fileName);
	std::stringstream sstr;
	sstr << ifs.rdbuf();
	return sstr.str();
}

Shader ResManager::loadShaderFromFile(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile) {
	// Retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	try {
		vertexCode = readFile(vShaderFile);
		fragmentCode = readFile(fShaderFile);
		if (gShaderFile != nullptr)
			geometryCode = readFile(gShaderFile);
	}
	catch (std::exception e) {
		std::cout << "Error : failed to read shader files" << std::endl;
	}
	// Create shader object from source code
	Shader shader;
	shader.compile(vertexCode.c_str(), fragmentCode.c_str(), gShaderFile != nullptr ? geometryCode.c_str() : nullptr);
	return shader;
}

Texture2D ResManager::loadTextureFromFile(const GLchar *file, GLboolean alpha) {
	// Create Texture object
	Texture2D texture;
	if (alpha) {
		texture.setInternalFormat(GL_RGBA);
		texture.setImageFormat(GL_RGBA);
	}
	// Load image
	int width, height, nbChannels;
	unsigned char* image = stbi_load(file, &width, &height, &nbChannels, STBI_rgb);

	// Generate texture
	if (image) {
		texture.generate(width, height, image);
	} else {
		std::cout << "Failed to load texture" << std::endl;
	}

	// Free image
	stbi_image_free(image);
	return texture;
}

void ResManager::initBlockDatas(const std::vector<TextureArray>& texArrays) {
	m_blockDatas = BlockDatas(texArrays);
}
