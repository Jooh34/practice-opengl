/** 
 * Copyright (C) 2023 Jooh
 **/

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct GLFWwindow;

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera
{
public:
	// camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	// euler Angles
	float Yaw;
	float Pitch;

	// camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	~Camera()
	{

	};

	glm::mat4 getViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	glm::vec3 getCamPosition();
	void processInput(GLFWwindow* window, float deltaTime);
	void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

private:
	void updateCameraVectors();
};
