#include "TextureArray.h"

#include "stb_image.h"
#include "Logger.h"

namespace fs = std::filesystem;

TextureArray::TextureArray(const std::vector<std::filesystem::path>& paths, ivec2 size, int imageFormat) {
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, imageFormat, size.x, size.y, paths.size(), 0, imageFormat, GL_UNSIGNED_BYTE, nullptr);
	glGenTextures(1, &textureArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);

	for (int i = 0; i < paths.size(); ++i) {
		const std::filesystem::path& path = paths[i];
		loadTexture(path.string(), i, imageFormat);
		textureMap[path.stem().string()] = i;
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void TextureArray::loadTexture(const std::string& name, int layerNumber, int imageFormat) const {
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *imageData = stbi_load(name.c_str(), &width, &height, &nrChannels, 0);
	if (imageData) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layerNumber, 512, 512, 1, imageFormat, GL_UNSIGNED_BYTE, imageData);
	} else {
		LOG(Level::ERROR) << "Failed to load texture" << std::endl;
	}
	stbi_image_free(imageData);
}

void TextureArray::bind() const {
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);
}

bool TextureArray::containsTexture(const std::string & name) const {
	return textureMap.find(name) != textureMap.end();
}

int TextureArray::getTextureID(const std::string& name) const {
	return textureMap.at(name);
}
