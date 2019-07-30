#pragma once
#include "Transform.h"
#include "Camera.h"

class FirstPersonDebug
{
	

public:

	Transform* transform;
	Camera * camera;
	FirstPersonDebug(GLfloat fovX, GLfloat width, GLfloat height, GLfloat near, GLfloat far);
	~FirstPersonDebug();
	void Update(GLFWwindow* window, GLfloat width, GLfloat height, glm::vec2 mousePosition, float deltaTime);
	glm::mat4 GetCameraMatrix();
};

