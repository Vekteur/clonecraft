#pragma once

#include <glad/glad.h>
#include "Maths/GlmCommon.h"
#include "Frustum.h"

class Camera
{
public:
	// Default values
	static const float SENSIVITY, SCROLLSPEED, ZOOM, YAW, PITCH;
	static const float NEARPLANE, FARPLANE;
	static const vec3 POSITION, WORLDUP;

	Camera(vec3 position = vec3{ 0.0f, 0.0f, 0.0f }, float yaw = YAW, float pitch = PITCH);

	void update(ivec2 screenDim);

	mat4 getViewMatrix() const;
	mat4 getProjMatrix() const;
	mat4 getProjViewMatrix() const;
	Frustum getFrustum() const;

	vec3 getPosition() const;
	vec3 getFront() const;
	vec3 getRight() const;
	float getYaw() const;
	float getPitch() const;
	void invertPitch();
	void move(vec3 offset);
	void setPosition(vec3 position);
	void processMouse(vec2 offset);
	void processMouseScroll(float yOffset);

private:
	vec3 m_position{ POSITION };
	vec3 m_front;
	vec3 m_up;
	vec3 m_right;

	float m_yaw{ YAW };
	float m_pitch{ PITCH };

	float m_sensitivity{ SENSIVITY };
	float m_zoom{ ZOOM };

	mat4 m_viewMatrix;
	mat4 m_projMatrix;
	mat4 m_projViewMatrix;
	Frustum m_frustum;

	void updateFromEuler();
};