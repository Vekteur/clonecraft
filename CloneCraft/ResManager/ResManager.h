#pragma once

#include <map>
#include <string>

#include <glad\glad.h>

#include "Texture2D.h"
#include "Shader.h"
#include "Block/BlockDatas.h"
#include "TextureArray.h"

class ResManager
{
public:
	ResManager() = delete;

	static void setShader(Shader& shader, std::string name);
	static Shader& getShader(std::string name);
	static void setTexture(Texture2D& texture, std::string name);
	static Texture2D& getTexture(std::string name);
	static std::string readFile(std::string fileName);

	static void initBlockDatas(const std::vector<TextureArray*>& texArrays);
	static const BlockDatas& blockDatas();

private:
	static std::map<std::string, Shader*> m_shaders;
	static std::map<std::string, Texture2D*> m_textures;
	static BlockDatas m_blockDatas;
};