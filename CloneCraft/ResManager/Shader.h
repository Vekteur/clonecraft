#pragma once

#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
	Shader() { }
	// Sets the current shader as active
	Shader  &use();
	// Compiles the shader from given source code
	void    compile(const GLchar *vertexSource, const GLchar *fragmentSource, const GLchar *geometrySource = nullptr);

	void    set(const GLchar *name, GLfloat value);
	void    set(const GLchar *name, GLint value);
	void    set(const GLchar *name, const glm::vec2 &value);
	void    set(const GLchar *name, const glm::vec3 &value);
	void    set(const GLchar *name, const glm::vec4 &value);
	void	set(const GLchar *name, const glm::ivec2 &value);
	void    set(const GLchar *name, const glm::ivec3 &value);
	void    set(const GLchar *name, const glm::ivec4 &value);
	void    set(const GLchar *name, const glm::mat4 &matrix);

	GLuint	getId();
private:
	// Checks if compilation or linking failed and if so, print the error logs
	GLuint	m_id;
	void    checkCompileErrors(GLuint object, std::string type);
};