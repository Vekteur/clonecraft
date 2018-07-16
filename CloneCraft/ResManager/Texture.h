#pragma once

#include <glad/glad.h>

class Texture2D
{
public:
	~Texture2D();

	Texture2D(GLuint internalFormat = GL_RGB, GLuint imageFormat = GL_RGB, 
		GLuint wrapS = GL_REPEAT, GLuint wrapT = GL_REPEAT, 
		GLuint filterMin = GL_NEAREST, GLuint filterMax = GL_NEAREST);

	void	generate(GLuint width, GLuint height, unsigned char* data);
	void loadFromFile(const GLchar *file, GLboolean alpha);
	void	bind() const;

	GLuint	getId() const;
	GLuint	getInternalFormat() const;
	GLuint	getImageFormat() const;
	void	setInternalFormat(GLuint internalFormat);
	void	setImageFormat(GLuint imageFormat);

private:
	GLuint m_id = 0;
	GLuint m_width{ 0 }, m_height{ 0 };
	GLuint m_internalFormat, m_imageFormat;
	GLuint m_wrapS, m_wrapT;
	GLuint m_filterMin, m_filterMax;
};