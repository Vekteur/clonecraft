#include "Camera.h"

#include <iostream>
#include <cmath>

#include "Window.h"
#include "Maths/Converter.h"
#include "Util/Logger.h"

const float Camera::SPEED{ 30.f };
const float Camera::SENSIVITY{ 0.16f };
const float Camera::SCROLLSPEED{ 3.f / 2 };
const float Camera::ZOOM{ 45.f };
const float Camera::YAW{ -90.f };
const float Camera::PITCH{ 0.f };

const float Camera::NEARPLANE{ 0.1f };
const float Camera::FARPLANE{ 2000.f };

const vec3 Camera::POSITION{ vec3{0.f, 0.f, 0.f } };
const vec3 Camera::WORLDUP{ vec3{ 0.f, 1.f, 0.f } };

Camera::Camera(vec3 position, float yaw, float pitch)
	:m_position{ position }, m_yaw{ yaw }, m_pitch{ pitch } {
	// Init up and right vectors from front vector
	updateFromEuler();
}

void Camera::update(ivec2 screenDim) {
	m_viewMatrix = glm::lookAt(m_position, m_position + m_front, WORLDUP);
	m_projMatrix = glm::perspective(glm::radians(m_zoom),
		static_cast<float>(screenDim.x) / static_cast<float>(screenDim.y), NEARPLANE, FARPLANE);
	m_projViewMatrix = m_projMatrix * m_viewMatrix;
	m_frustum = Frustum(m_projViewMatrix);
}

mat4 Camera::getViewMatrix() {
	return m_viewMatrix;
}

mat4 Camera::getProjMatrix() {
	return m_projMatrix;
}

mat4 Camera::getProjViewMatrix() {
	return m_projViewMatrix;
}

Frustum Camera::getFrustum() {
	return m_frustum;
}

vec3 Camera::getPosition() {
	return m_position;
}

vec3 Camera::getFront() {
	return m_front;
}

float Camera::getYaw() {
	return m_yaw;
}

float Camera::getPitch() {
	return m_pitch;
}

void Camera::invertPitch() {
	m_pitch = -m_pitch;
	updateFromEuler();
}

vec3 toHorizontal(vec3 vec) { // There must be a horizontal movement
	return normalize(vec3{ vec.x, 0.f, vec.z });
}

void Camera::move(Direction direction, float deltaTime) {
	float velocity = m_speed * deltaTime;

	switch (direction) {
	case FORWARD:
		m_position += toHorizontal(m_front) * velocity;
		break;
	case BACKWARD:
		m_position -= toHorizontal(m_front) * velocity;
		break;
	case RIGHT:
		m_position += toHorizontal(m_right) * velocity;
		break;
	case LEFT:
		m_position -= toHorizontal(m_right) * velocity;
		break;
	case UP:
		m_position += WORLDUP * velocity;
		break;
	case DOWN:
		m_position -= WORLDUP * velocity;
		break;
	}
}

void Camera::move(vec3 offset) {
	m_position += offset;
}

void Camera::setPosition(vec3 position) {
	m_position = position;
}

void Camera::processMouse(vec2 offset) {
	offset.x *= m_sensitivity * (m_zoom / ZOOM);
	offset.y *= m_sensitivity * (m_zoom / ZOOM);

	m_yaw = static_cast<float>(fmod(m_yaw + offset.x + 360, 360));
	m_pitch -= offset.y;
	m_pitch = clamp(m_pitch, -89.9f, 89.9f);

	this->updateFromEuler();
}

void Camera::processMouseScroll(float yOffset) {
	m_zoom *= pow(1.f / SCROLLSPEED, yOffset);
	m_zoom = clamp(m_zoom, 1.f, ZOOM);
}

void Camera::updateFromEuler() {
	vec3 front;
	front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	front.y = sin(glm::radians(m_pitch));
	front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_front = normalize(front);

	m_right = normalize(cross(m_front, WORLDUP));
	m_up = normalize(cross(m_right, m_front));
}