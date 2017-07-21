#include "Camera.h"

#include <iostream>
#include <cmath>

#include "Window.h"

const GLfloat Camera::SPEED{ 30.0f };
const GLfloat Camera::SENSITIVTY{ 0.25f };
const GLfloat Camera::ZOOM{ 45.0f };
const GLfloat Camera::YAW{ -90.0f };
const GLfloat Camera::PITCH{ 0.0f };

const vec3 Camera::POSITION{ vec3{0.0f, 0.0f, 0.0f } };
const vec3 Camera::FRONT{ vec3{ 0.0f, 0.0f, -1.0f } };
const vec3 Camera::WORLDUP{ vec3{ 0.0f, 1.0f, 0.0f } };

const GLfloat Camera::NEARPLANE{ 0.1f };
const GLfloat Camera::FARPLANE{ 1000.0f };

Camera::Camera(vec3 position, GLfloat yaw, GLfloat pitch)
	:m_position{ position }, m_yaw{ yaw }, m_pitch{ pitch }
{
	// Init up and right vectors from front vector
	this->updateFromEuler();
}

mat4 Camera::getViewMatrix()
{
	return glm::lookAt(m_position, m_position + m_front, WORLDUP);
}

mat4 Camera::getProjectionMatrix()
{
	return glm::perspective(glm::radians(m_zoom), static_cast<GLfloat>(Window::SCREEN_WIDTH) / static_cast<GLfloat>(Window::SCREEN_HEIGHT), NEARPLANE, FARPLANE);
}

vec3 Camera::getPosition()
{
	return m_position;
}

GLfloat Camera::getYaw()
{
	return m_yaw;
}

GLfloat Camera::getPitch()
{
	return m_pitch;
}

void Camera::move(Direction direction, GLfloat deltaTime)
{
	GLfloat velocity = m_speed * deltaTime;
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

void Camera::move(vec3 offset)
{
	m_position += offset;
}

void Camera::setPosition(vec3 position)
{
	m_position = position;
}

void Camera::processMouse(GLfloat xOffset, GLfloat yOffset)
{
	xOffset *= m_sensitivity;
	yOffset *= m_sensitivity;

	m_yaw = fmod(m_yaw + xOffset + 360, 360);
	m_pitch += yOffset;

	if (m_pitch > 89.0f)
		m_pitch = 89.0f;
	if (m_pitch < -89.0f)
		m_pitch = -89.0f;

	this->updateFromEuler();
}

void Camera::processMouseScroll(GLfloat yOffset)
{
	m_zoom -= yOffset;
	if (m_zoom < 1.0f)
		m_zoom = 1.0f;
	else if (m_zoom > 45.0f)
		m_zoom = 45.0f;
}

void Camera::updateFromEuler()
{
	vec3 front;
	front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	front.y = sin(glm::radians(m_pitch));
	front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_front = normalize(front);

	m_right = normalize(cross(m_front, WORLDUP));
	m_up = normalize(cross(m_right, m_front));
}