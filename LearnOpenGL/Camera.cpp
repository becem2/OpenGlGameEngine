
#include "Camera.h"

// Initialize static instance
Camera* Camera::instance = nullptr;

Camera::Camera(const unsigned int Width, const unsigned int Height) {
	Camera::instance = this;

	position = glm::vec3(0.0f, 0.0f, 3.0f); // camera position is set to x = 0 / y = 0 / z = -3
	front = glm::vec3(0.0f, 0.0f, -1.0f); // camera vector front is set to z = -1
	up = glm::vec3(0.0f, 1.0f, 0.0f); // camera up vector is set to y = 1

	screenHeight = Height;
	screenWidth = Width;

	Xoffset = Width /2.0f;
	Yoffset = Height /2.0f;
	lastX = Width / 2.0f;
	lastY = Height / 2.0f;

	yaw = -90.0f;
	pitch = 0.0f; 
	fov = 45.0f;
	sensitivity = 0.1;

	firstMouse = true;

	minRender = 0.1f;
	maxRender = 100.0f;

	pitchLock = false;
	yawLock = false;



}



void Camera::setYaw(float Yaw) { yaw = Yaw; }
void Camera::setPitch(float Pitch) { pitch = Pitch; }
void Camera::setFov(float Fov) {fov = Fov; }
void Camera::setSensitivity(float Sensitivity) { sensitivity = Sensitivity; }
void Camera::setMinRender(float R) { minRender = R; }
void Camera::setMaxRender(float R) { maxRender = R; }

void Camera::setPitchLock(bool P) { pitchLock = P; }
void Camera::setYawLock(bool P) { yawLock = P; }
void Camera::setPosition(float x,float y,float z){
	position = glm::vec3(x, y, z);
}
void Camera::setFrontDirection(bool DirectionForward) {

	if (DirectionForward) {
		front = glm::vec3(0.0f, 0.0f, -1.0f);
	}
	else {
		front = glm::vec3(0.0f, 0.0f, 1.0f);
	}
}
void Camera::setUpDirection(const char Up) {
	switch (Up) {
		case 'x':
			up = glm::vec3(1.0f, 0.0f, 0.0f);
			break;

		case 'y':
			up = glm::vec3(0.0f, 1.0f, 0.0f);
			break;

		case 'z':
			up = glm::vec3(0.0f, 0.0f, 1.0f);
			break;

		default :
			up = glm::vec3(1.0f, 0.0f, 0.0f);
			break;
	}

}


float Camera::getYaw() { return yaw; }
float Camera::getPitch() { return pitch; }
float Camera::getFov() { return fov; }
float Camera::getSensitivity() { return sensitivity; }
float Camera::getMinRender() { return minRender; }
float Camera::getMaxRender() { return maxRender; }
bool Camera::getPitchLock() { return pitchLock; }
bool Camera::getYawLock() { return yawLock; }
glm::vec3& Camera::getPosition() { return position; }
glm::vec3 Camera::getFront() { return front; }
glm::vec3 Camera::getUp() { return up; }



void Camera::_mouse_callback(GLFWwindow* window, float xpos, float ypos) {

	float Xoffset = (xpos - lastX);

	float Yoffset = (lastY - ypos);

	float sensitivity = 0.1f;


	lastY = ypos;
	lastX = xpos;
	Xoffset *= sensitivity;
	Yoffset *= sensitivity;
	yaw += Xoffset;
	pitch += Yoffset;

	if (pitchLock) {
		if (pitch >  89.0f) pitch = 89.0f;
		if (pitch < -89.0f) pitch = -89.0f;
	}

	if (yawLock) {
		if (yaw >  179.0f) yaw = 179.0f;
		if (yaw < -179.0f) yaw = -179.0f;
	}



	glm::vec3 direction = glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch)));

	front = glm::normalize(direction);

}

void Camera::_scroll_callback(GLFWwindow* window, float xoffset, float yoffset) {
	fov -= yoffset;
	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 45.0f)
		fov = 45.0f;
}

// Static wrapper callbacks
void Camera::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (Camera::instance != nullptr) {
		Camera::instance->_mouse_callback(window, (float)xpos, (float)ypos);
	}
}

void Camera::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (Camera::instance != nullptr) {
		Camera::instance->_scroll_callback(window, (float)xoffset, (float)yoffset);
	}
}



void Camera::Use(Shader shader) {
	glm::mat4 projection = glm::perspective(glm::radians(fov), (float)screenWidth / (float)screenHeight, minRender, maxRender);
	shader.setMat4("projection", projection);

	glm::mat4 view = glm::lookAt(position, position + front, up);
	shader.setMat4("view", view);

}






Camera::~Camera(void) {


}