#pragma once

#include <map>
#include <string>

#include <GL/glew.h>

#include "Texture.h"
#include "Shader.h"

class ResManager
{
public:
	ResManager() = delete;

	static std::map<std::string, Shader>    shaders;
	static std::map<std::string, Texture2D> textures;

	static Shader		loadShader(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile, std::string name);
	static Shader		getShader(std::string name);
	static Texture2D	loadTexture(const GLchar *file, GLboolean alpha, std::string name);
	static Texture2D	getTexture(std::string name);
	static void			clear();

private:
	static Shader		loadShaderFromFile(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile = nullptr);
	static Texture2D	loadTextureFromFile(const GLchar *file, GLboolean alpha);
};