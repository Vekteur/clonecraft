#include "TextureArray.h"

#include "stb_image/stb_image.h"
#include "Util/Logger.h"
#include "ResManager.h"

namespace fs = std::filesystem;

TextureArray::TextureArray(const std::vector<std::filesystem::path>& paths, ivec2 size, int imageFormat) {
	glGenTextures(1, &textureArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, imageFormat, size.x, size.y, paths.size(), 0, imageFormat, GL_UNSIGNED_BYTE, nullptr);

	for (int i = 0; i < int(paths.size()); ++i) {
		const std::filesystem::path& path = paths[i];
		loadTexture(path.string(), i, size, imageFormat);
		textureMap[path.stem().string()] = i;
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

TextureArray::~TextureArray() {
	glDeleteTextures(1, &textureArray);
}

void TextureArray::loadTexture(const std::string& name, int layerNumber, ivec2 size, int imageFormat) const {
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *imageData = stbi_load(name.c_str(), &width, &height, &nrChannels, 0);
	if (imageData) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layerNumber, size.x, size.y, 1, imageFormat, GL_UNSIGNED_BYTE, imageData);
	} else {
		LOG(Level::ERROR) << "Failed to load texture" << std::endl;
	}
	stbi_image_free(imageData);
}

void TextureArray::bind() const {
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);
}

void TextureArray::unbind() const {
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

bool TextureArray::containsTexture(const std::string & name) const {
	return textureMap.find(name) != textureMap.end();
}

int TextureArray::getTextureID(const std::string& name) const {
	return textureMap.at(name);
}
