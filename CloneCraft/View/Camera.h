#pragma once

#include <GL/glew.h>
#include "GlmCommon.h"

class Camera
{
private:

	// Default values
	static const GLfloat SPEED, SENSITIVTY, ZOOM, YAW, PITCH;
	static const GLfloat NEARPLANE, FARPLANE;
	static const vec3 POSITION, FRONT, WORLDUP;

	vec3 m_position;
	vec3 m_front{ FRONT };
	vec3 m_up;
	vec3 m_right;

	GLfloat m_yaw;
	GLfloat m_pitch;

	GLfloat m_speed{ SPEED };
	GLfloat m_sensitivity{ SENSITIVTY };
	GLfloat m_zoom{ ZOOM };

	void updateFromEuler();

public:

	enum Direction {
		FORWARD,
		BACKWARD,
		RIGHT,
		LEFT,
		UP,
		DOWN
	};

	Camera(vec3 position = vec3{ 0.0f, 0.0f, 0.0f }, GLfloat yaw = YAW, GLfloat pitch = PITCH);

	mat4 getViewMatrix();
	mat4 getProjectionMatrix();
	vec3 getPosition();
	GLfloat getYaw();
	GLfloat getPitch();

	void move(Direction direction, GLfloat deltaTime);
	void move(vec3 offset);
	void setPosition(vec3 position);
	void processMouse(GLfloat xOffset, GLfloat yOffset);
	void processMouseScroll(GLfloat yOffset);
};