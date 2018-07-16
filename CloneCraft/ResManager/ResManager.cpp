#include "ResManager.h"

#include <iostream>
#include <sstream>
#include <fstream>

// Instantiate static variables
std::map<std::string, Texture2D*>    ResManager::m_textures;
std::map<std::string, Shader*>       ResManager::m_shaders;

BlockDatas ResManager::m_blockDatas;

const BlockDatas& ResManager::blockDatas() {
	return m_blockDatas;
}

void ResManager::setShader(Shader& shader, std::string name) {
	m_shaders[name] = &shader;
}

Shader& ResManager::getShader(std::string name) {
	return *m_shaders[name];
}

void ResManager::setTexture(Texture2D& texture, std::string name) {
	m_textures[name] = &texture;
}

Texture2D& ResManager::getTexture(std::string name) {
	return *m_textures[name];
}

std::string ResManager::readFile(std::string fileName) {
	std::ifstream ifs(fileName);
	std::stringstream sstr;
	sstr << ifs.rdbuf();
	return sstr.str();
}

void ResManager::initBlockDatas(const std::vector<TextureArray*>& texArrays) {
	m_blockDatas = BlockDatas(texArrays);
}
