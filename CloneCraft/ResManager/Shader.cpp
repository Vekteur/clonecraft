#include "Shader.h"

#include <iostream>

Shader &Shader::use() {
	glUseProgram(this->m_id);
	return *this;
}

void Shader::compile(const GLchar* vertexSource, const GLchar* fragmentSource, const GLchar* geometrySource) {
	m_id = glCreateProgram();

	GLuint sVertex, sFragment, sGeometry;
	// Vertex Shader
	sVertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(sVertex, 1, &vertexSource, NULL);
	glCompileShader(sVertex);
	checkCompileErrors(sVertex, "VERTEX");
	glAttachShader(m_id, sVertex);
	// Fragment Shader
	sFragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(sFragment, 1, &fragmentSource, NULL);
	glCompileShader(sFragment);
	checkCompileErrors(sFragment, "FRAGMENT");
	glAttachShader(m_id, sFragment);
	// If geometry shader source code is given, also compile geometry shader
	if (geometrySource != nullptr) {
		sGeometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(sGeometry, 1, &geometrySource, NULL);
		glCompileShader(sGeometry);
		checkCompileErrors(sGeometry, "GEOMETRY");
		glAttachShader(m_id, sGeometry);
	}
	// Shader Program
	glLinkProgram(m_id);
	checkCompileErrors(m_id, "PROGRAM");
	// Delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(sVertex);
	glDeleteShader(sFragment);
	if (geometrySource != nullptr)
		glDeleteShader(sGeometry);
}

void Shader::set(const GLchar *name, GLfloat value) {
	glUniform1f(glGetUniformLocation(this->m_id, name), value);
}

void Shader::set(const GLchar *name, GLint value) {
	glUniform1i(glGetUniformLocation(this->m_id, name), value);
}

void Shader::set(const GLchar *name, const glm::vec2 &value) {
	glUniform2f(glGetUniformLocation(this->m_id, name), value.x, value.y);
}

void Shader::set(const GLchar *name, const glm::vec3 &value) {
	glUniform3f(glGetUniformLocation(this->m_id, name), value.x, value.y, value.z);
}

void Shader::set(const GLchar *name, const glm::vec4 &value) {
	glUniform4f(glGetUniformLocation(this->m_id, name), value.x, value.y, value.z, value.w);
}

void Shader::set(const GLchar * name, const glm::ivec2 & value) {
	glUniform2i(glGetUniformLocation(this->m_id, name), value.x, value.y);
}

void Shader::set(const GLchar * name, const glm::ivec3 & value) {
	glUniform3i(glGetUniformLocation(this->m_id, name), value.x, value.y, value.z);
}

void Shader::set(const GLchar * name, const glm::ivec4 & value) {
	glUniform4i(glGetUniformLocation(this->m_id, name), value.x, value.y, value.z, value.w);
}

void Shader::set(const GLchar *name, const glm::mat4 &matrix) {
	glUniformMatrix4fv(glGetUniformLocation(this->m_id, name), 1, GL_FALSE, glm::value_ptr(matrix));
}

GLuint Shader::getId() {
	return m_id;
}

void Shader::checkCompileErrors(GLuint object, std::string type) {
	GLint success;
	GLchar infoLog[1024];
	if (type != "PROGRAM") {
		glGetShaderiv(object, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(object, 1024, NULL, infoLog);
			std::cout << "Error : shader of type " << type << " didn't load properly\n" << infoLog << "\n";
		}
	}
	else {
		glGetProgramiv(object, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(object, 1024, NULL, infoLog);
			std::cout << "Error : shader of type " << type << " didn't load properly\n" << infoLog << "\n";
		}
	}
}