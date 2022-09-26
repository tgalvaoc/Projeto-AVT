#include "Camera.h"

#include "AVTmathlib.h"

using namespace std;


void Camera::rotateCamera(float dAlpha, float dBeta) {
	this->alpha += dAlpha;
	this->beta += dBeta;

	if (this->beta > 85.0f)
		this->beta = 85.0f;
	else if (this->beta < -85.0f)
		this->beta = -85.0f;


	this->fixPosition();

}

void Camera::setProjection() {
	///switch()
}

void Camera::cameraLookAt() {
	lookAt(this->pos[0], this->pos[1], this->pos[2],
		this->target[0], this->target[1], this->target[2],
		0, 1, 0);
}

void Camera::fixPosition() {
	this->pos[0] = this->r * sin(this->alpha * 3.14f / 180.0f) * cos(this->beta * 3.14f / 180.0f);
	this->pos[1] = this->r * sin(this->beta * 3.14f / 180.0f);
	this->pos[2] = this->r * cos(this->alpha * 3.14f / 180.0f) * cos(this->beta * 3.14f / 180.0f);
}


vector<Camera> Camera::buildCameras() {
	vector<Camera> cameras;

	cameras.push_back(Camera::moving());
	cameras.push_back(Camera::perspective());
	cameras.push_back(Camera::orthogonal());
	return cameras;
}

Camera Camera::moving() {
	Camera camera = Camera();

	camera.r = 10.0f;
	camera.alpha = -180.0f;
	camera.beta = 45.0f;

	camera.fixPosition();

	camera.target[0] = 0.0f;
	camera.target[1] = 0.0f;
	camera.target[2] = 0.0f;
};

Camera Camera::perspective() {
	Camera camera = Camera();

	camera.r = 100.0f;
	camera.alpha = 0.0f;
	camera.beta = 0.00f;

	camera.fixPosition();

	camera.target[0] = 0.0f;
	camera.target[1] = 0.0f;
	camera.target[2] = 0.0f;
};

Camera Camera::orthogonal() {
	Camera camera = Camera();

	camera.r = 100.0f;
	camera.alpha = 0.0f;
	camera.beta = 0.0f;

	camera.fixPosition();

	camera.target[0] = 0.0f;
	camera.target[1] = 0.0f;
	camera.target[2] = 0.0f;
};