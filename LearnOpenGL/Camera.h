
#pragma once

#ifndef CAMERA_H
#define CAMERA_H


#include <glm/glm.hpp>
#include <string>
#include <iostream>
#include <glad.h>
#include <glfw3.h>

#include "Shader.h"


class Camera {

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;

	unsigned int screenWidth;
	unsigned int screenHeight;
	float yaw;
	float pitch;
	float fov;
	float Xoffset;
	float Yoffset;
	float lastX;
	float lastY;
	float sensitivity;
	float minRender;
	float maxRender;

	bool firstMouse;
	bool pitchLock;
	bool yawLock;

	static Camera* instance;

public: 

	Camera(const unsigned int Width,const unsigned int Height);
	// getters
	void setYaw(float Yaw);
	void setPitch(float Pitch);
	void setFov(float Fov);
	void setSensitivity(float Sensitivity);
	void setMinRender(float R);
	void setMaxRender(float R);
	void setPitchLock(bool P);
	void setYawLock(bool P);
	void setPosition(float x,float y,float z);
	void setFrontDirection(bool DirectionForward);
	void setUpDirection(const char Up);


	// setters
	float getYaw();
	float getPitch();
	float getFov();
	float getSensitivity();
	float getMinRender();
	float getMaxRender();
	bool getPitchLock();
	bool getYawLock();
	glm::vec3& getPosition();
	glm::vec3 getFront();
	glm::vec3 getUp();

	// Member callback functions
	void _mouse_callback(GLFWwindow* window, float xpos, float ypos);
	void _scroll_callback(GLFWwindow* window, float xoffset, float yoffset);

	// Static wrapper callbacks for GLFW
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	void Use(Shader shader);


	~Camera(void);


};



#endif