#include "Debug.h"

#include <iostream>
#include <string>

#include "Logger.h"

GLenum Debug::glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		LOG(Level::ERROR) << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}

void APIENTRY Debug::glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar *message, const void *userParam) {
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	LOG(Level::ERROR) << "---------------" << std::endl;
	LOG(Level::ERROR) << "Debug message (" << id << "): " << message << std::endl;

	switch (source) {
	case GL_DEBUG_SOURCE_API:             LOG(Level::ERROR) << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   LOG(Level::ERROR) << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: LOG(Level::ERROR) << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     LOG(Level::ERROR) << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     LOG(Level::ERROR) << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           LOG(Level::ERROR) << "Source: Other"; break;
	} LOG << std::endl;

	switch (type) {
	case GL_DEBUG_TYPE_ERROR:               LOG(Level::ERROR) << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: LOG(Level::ERROR) << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  LOG(Level::ERROR) << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         LOG(Level::ERROR) << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         LOG(Level::ERROR) << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              LOG(Level::ERROR) << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          LOG(Level::ERROR) << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           LOG(Level::ERROR) << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               LOG(Level::ERROR) << "Type: Other"; break;
	} LOG << std::endl;

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:         LOG(Level::ERROR) << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       LOG(Level::ERROR) << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          LOG(Level::ERROR) << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: LOG(Level::ERROR) << "Severity: notification"; break;
	} LOG << std::endl;
	LOG << std::endl;
}