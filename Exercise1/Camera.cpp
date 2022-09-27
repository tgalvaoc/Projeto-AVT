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

void Camera::setProjection(float w, float h) {
	switch (this->type) {
	case MOVING:
	case PERSPECTIVE:
		perspective(45.0f, w/h, 0.0001, 1000000);
		break;
	case ORTHOGONAL:
		ortho(0.0f, w, w, 0.0f, 0.0f, 1.0f);
		break;
	}
		
}

void Camera::cameraLookAt() {
	lookAt(this->pos[0], this->pos[1], this->pos[2],
		this->target[0], this->target[1], this->target[2],
		0, 1, 0);
}

void Camera::fixPosition() { // seno e cosseno estavam invertidos
	// alpha eh o angulo horizontal (que normalmente chamamde phi)
	// beta eh o angulo vertical (theta)
	this->pos[0] = this->r * cosf(this->alpha * 3.14f / 180.0f) * sinf(this->beta * 3.14f / 180.0f);
	this->pos[1] = this->r * cosf(this->beta * 3.14f / 180.0f);
	this->pos[2] = this->r * sinf(this->alpha * 3.14f / 180.0f) * sinf(this->beta * 3.14f / 180.0f);
}

void Camera::fixAngles() {
	this->r = sqrtf(this->pos[0] * this->pos[0] + this->pos[0] * this->pos[0] + this->pos[0] * this->pos[0]);
}


void Camera::translateCamera(float dx, float dy) {
	//this->pos[0] += sin(this->alpha * 3.14f / 180.0f) * sin(this->beta * 3.14f / 180.0f)
}

vector<Camera> Camera::buildCameras() {
	vector<Camera> cameras;

	cameras.push_back(Camera::newMoving());
	cameras.push_back(Camera::newPerspective());
	cameras.push_back(Camera::newOrthogonal());
	return cameras;
}

Camera Camera::newMoving() {
	Camera camera = Camera();

	camera.r = 10.0f;
	camera.alpha = -180.0f;
	camera.beta = -60.0f;

	camera.fixPosition();

	camera.target[0] = 0.0f;
	camera.target[1] = 0.0f;
	camera.target[2] = 0.0f;

	camera.type = MOVING;

	return camera;
};

Camera Camera::newPerspective() {
	Camera camera = Camera();

	camera.r = 100.0f;
	camera.alpha = 0.0f;
	camera.beta = 0.00f;

	camera.fixPosition();

	camera.target[0] = 0.0f;
	camera.target[1] = 0.0f;
	camera.target[2] = 0.0f;

	camera.type = PERSPECTIVE;

	return camera;
};

Camera Camera::newOrthogonal() {
	Camera camera = Camera();

	camera.r = 100.0f;
	camera.alpha = 0.0f;
	camera.beta = 0.0f;

	camera.fixPosition();

	camera.target[0] = 0.0f;
	camera.target[1] = 0.0f;
	camera.target[2] = 0.0f;

	camera.type = ORTHOGONAL;

	return camera;
};
