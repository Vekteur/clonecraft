#include "Camera.h"

#include <iostream>
#include <cmath>

#include "Window.h"
#include "Converter.h"

const float Camera::SPEED{ 30.0f };
const float Camera::SENSITIVTY{ 0.25f };
const float Camera::SCROLLSPEED{ 5.0f };
const float Camera::ZOOM{ 45.0f };
const float Camera::YAW{ -90.0f };
const float Camera::PITCH{ 0.0f };

const float Camera::NEARPLANE{ 0.1f };
const float Camera::FARPLANE{ 1000.0f };

const vec3 Camera::POSITION{ vec3{0.0f, 0.0f, 0.0f } };
const vec3 Camera::FRONT{ vec3{ 0.0f, 0.0f, -1.0f } };
const vec3 Camera::WORLDUP{ vec3{ 0.0f, 1.0f, 0.0f } };

Camera::Camera(vec3 position, float yaw, float pitch)
	:m_position{ position }, m_yaw{ yaw }, m_pitch{ pitch }, m_chunk{ Converter::globalToChunk(position) } {
	// Init up and right vectors from front vector
	updateFromEuler();
}

mat4 Camera::getViewMatrix() {
	return glm::lookAt(m_position, m_position + m_front, WORLDUP);
}

mat4 Camera::getProjectionMatrix() {
	return glm::perspective(glm::radians(m_zoom),
		static_cast<float>(Window::SCREEN_WIDTH) / static_cast<float>(Window::SCREEN_HEIGHT), NEARPLANE, FARPLANE);
}

vec3 Camera::getPosition() {
	return m_position;
}

float Camera::getYaw() {
	return m_yaw;
}

float Camera::getPitch() {
	return m_pitch;
}

void Camera::move(Direction direction, float deltaTime) {
	float velocity = m_speed * deltaTime;
	if (direction == FORWARD)
		m_position += m_front * velocity;
	if (direction == BACKWARD)
		m_position -= m_front * velocity;
	if (direction == RIGHT)
		m_position += m_right * velocity;
	if (direction == LEFT)
		m_position -= m_right * velocity;
	if (direction == UP)
		m_position += m_up * velocity;
	if (direction == DOWN)
		m_position -= m_up * velocity;
}

void Camera::move(vec3 offset) {
	m_position += offset;
}

void Camera::setPosition(vec3 position) {
	m_position = position;
}

void Camera::processMouse(vec2 offset) {
	offset.x *= m_sensitivity;
	offset.y *= m_sensitivity;

	m_yaw = fmod(m_yaw + offset.x + 360, 360);
	m_pitch += offset.y;

	if (m_pitch > 89.0f)
		m_pitch = 89.0f;
	if (m_pitch < -89.0f)
		m_pitch = -89.0f;

	this->updateFromEuler();
}

void Camera::processMouseScroll(float yOffset) {
	m_zoom -= yOffset * SCROLLSPEED;
	if (m_zoom < 1.0f)
		m_zoom = 1.0f;
	else if (m_zoom > 45.0f)
		m_zoom = 45.0f;
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