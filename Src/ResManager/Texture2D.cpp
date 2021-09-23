
#include "Texture2D.h"

#include "stb_image/stb_image.h"

#include <iostream>

Texture2D::~Texture2D() {
	glDeleteTextures(1, &m_id);
}

Texture2D::Texture2D(GLuint internalFormat, GLuint imageFormat, GLuint wrapS, GLuint wrapT, GLuint filterMin, GLuint filterMax)
	:m_internalFormat(internalFormat), m_imageFormat(imageFormat), m_wrapS(wrapS), m_wrapT(wrapT), m_filterMin(filterMin), m_filterMax(filterMax) {
	glGenTextures(1, &m_id);
}

void Texture2D::generate(GLuint width, GLuint height, unsigned char* data) {
	// Save dimension
	m_width = width;
	m_height = height;
	// Create Texture
	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, width, height, 0, m_imageFormat, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	// Set Texture wrap and filter modes
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_filterMin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_filterMax);
	// Unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::loadFromFile(const GLchar *file, GLboolean alpha) {
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
}

void Texture2D::bind() const {
	glBindTexture(GL_TEXTURE_2D, m_id);
}

GLuint Texture2D::getId() const {
	return m_id;
}

GLuint Texture2D::getInternalFormat() const {
	return m_internalFormat;
}

GLuint Texture2D::getImageFormat() const {
	return m_imageFormat;
}

void Texture2D::setInternalFormat(GLuint internalFormat) {
	m_internalFormat = internalFormat;
}

void Texture2D::setImageFormat(GLuint imageFormat) {
	m_imageFormat = imageFormat;
}