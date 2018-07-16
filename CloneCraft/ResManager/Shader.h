#pragma once

#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
	Shader();
	~Shader();
	// Sets the current shader as active
	const Shader& use() const;
	// Compiles the shader from given source code

	void    compile(const GLchar *vertexSource, const GLchar *fragmentSource, const GLchar *geometrySource = nullptr);
	void loadFromFile(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile = nullptr);

	void    set(const GLchar *name, GLfloat value) const;
	void    set(const GLchar *name, GLint value) const;
	void    set(const GLchar *name, const glm::vec2 &value) const;
	void    set(const GLchar *name, const glm::vec3 &value) const;
	void    set(const GLchar *name, const glm::vec4 &value) const;
	void	set(const GLchar *name, const glm::ivec2 &value) const;
	void    set(const GLchar *name, const glm::ivec3 &value) const;
	void    set(const GLchar *name, const glm::ivec4 &value) const;
	void    set(const GLchar *name, const glm::mat4 &matrix) const;

	GLuint	getId() const;
private:
	// Checks if compilation or linking failed and if so, print the error logs
	GLuint	m_id = 0;
	void    checkCompileErrors(GLuint object, std::string type) const;
};