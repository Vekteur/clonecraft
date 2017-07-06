#include <iostream>

#include "texture.h"


Texture2D::Texture2D(GLuint internalFormat, GLuint imageFormat, GLuint wrapS, GLuint wrapT, GLuint filterMin, GLuint filterMax)
	:m_internalFormat(internalFormat), m_imageFormat(imageFormat), m_wrapS(wrapS), m_wrapT(wrapT), m_filterMin(filterMin), m_filterMax(filterMax)
{
	glGenTextures(1, &this->m_id);
}


void Texture2D::generate(GLuint width, GLuint height, unsigned char* data)
{
	// Save dimension
	this->m_width = width;
	this->m_height = height;
	// Create Texture
	glBindTexture(GL_TEXTURE_2D, this->m_id);
	glTexImage2D(GL_TEXTURE_2D, 0, this->m_internalFormat, width, height, 0, this->m_imageFormat, GL_UNSIGNED_BYTE, data);
	// Set Texture wrap and filter modes
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->m_wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->m_wrapT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->m_filterMin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->m_filterMax);
	// Unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::bind() const
{
	glBindTexture(GL_TEXTURE_2D, this->m_id);
}

GLuint Texture2D::getId() const
{
	return m_id;
}

GLuint Texture2D::getInternalFormat() const
{
	return m_internalFormat;
}

GLuint Texture2D::getImageFormat() const
{
	return m_imageFormat;
}

void Texture2D::setInternalFormat(GLuint internalFormat)
{
	m_internalFormat = internalFormat;
}

void Texture2D::setImageFormat(GLuint imageFormat)
{
	m_imageFormat = imageFormat;
}
