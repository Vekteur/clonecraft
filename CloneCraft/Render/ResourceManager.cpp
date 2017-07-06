#include "ResourceManager.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include <SOIL/SOIL.h>

// Instantiate static variables
std::map<std::string, Texture2D>    ResManager::Textures;
std::map<std::string, Shader>       ResManager::Shaders;

Shader ResManager::loadShader(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile, std::string name)
{
	Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
	return Shaders[name];
}

Shader ResManager::getShader(std::string name)
{
	return Shaders[name];
}

Texture2D ResManager::loadTexture(const GLchar *file, GLboolean alpha, std::string name)
{
	Textures[name] = loadTextureFromFile(file, alpha);
	return Textures[name];
}

Texture2D ResManager::getTexture(std::string name)
{
	return Textures[name];
}

void ResManager::clear()
{
	// Properly delete all shaders	
	for (auto iter : Shaders)
		glDeleteProgram(iter.second.getId());
	// Properly delete all textures
	for (auto iter : Textures)
	{
		GLuint id{ iter.second.getId() };
		glDeleteTextures(1, &id);
	}
}

Shader ResManager::loadShaderFromFile(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile)
{
	// Retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	try
	{
		std::ifstream vertexShaderFile{ vShaderFile };
		std::stringstream vShaderStream;
		vShaderStream << vertexShaderFile.rdbuf();
		vertexShaderFile.close();
		vertexCode = vShaderStream.str();

		std::ifstream fragmentShaderFile{ fShaderFile };
		std::stringstream fShaderStream;
		fShaderStream << fragmentShaderFile.rdbuf();
		fragmentShaderFile.close();
		fragmentCode = fShaderStream.str();

		if (gShaderFile != nullptr)
		{
			std::ifstream geometryShaderFile{ gShaderFile };
			std::stringstream gShaderStream;
			gShaderStream << geometryShaderFile.rdbuf();
			geometryShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	catch (std::exception e)
	{
		std::cout << "Error : failed to read shader files" << std::endl;
	}
	// Create shader object from source code
	Shader shader;
	shader.compile(vertexCode.c_str(), fragmentCode.c_str(), gShaderFile != nullptr ? geometryCode.c_str() : nullptr);
	return shader;
}

Texture2D ResManager::loadTextureFromFile(const GLchar *file, GLboolean alpha)
{
	// Create Texture object
	Texture2D texture;
	if (alpha)
	{
		texture.setInternalFormat(GL_RGBA);
		texture.setImageFormat(GL_RGBA);
	}
	// Load image
	int width, height;
	unsigned char* image = SOIL_load_image(file, &width, &height, 0, texture.getImageFormat() == GL_RGBA ? SOIL_LOAD_RGBA : SOIL_LOAD_RGB);
	// Generate texture
	texture.generate(width, height, image);
	// Free image
	SOIL_free_image_data(image);
	return texture;
}