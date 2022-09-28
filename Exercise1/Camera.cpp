#include "Camera.h"

#include "AVTmathlib.h"

#include <iostream>

using namespace std;


Camera::Camera() {};

Camera::Camera(CameraType camType, float x, float y, float z, float r_val, float alpha_val, float beta_val, float targetX_val, float targetY_val, float targetZ_val) {

	r = r_val;
	alpha = alpha_val;
	beta = beta_val;
	fixPosition();

	target[0] = targetX_val;
	target[1] = targetY_val;
	target[2] = targetZ_val;

	type = camType;

	setProjection((float)1024, (float)768);
}

void Camera::rotateCamera(float dAlpha, float dBeta) {
	this->alpha += dAlpha / 10.0;
	this->beta += dBeta / 10.0;

	if (this->alpha > 360.0)
		this->alpha -= 360.0;
	else if (this->alpha < 0.0)
		this->alpha += 360.0;

	if (this->beta > 85.0f)
		this->beta = 85.0f;
	else if (this->beta < -85.0f)
		this->beta = -85.0f;

	cout << "alpha = " << alpha << ", beta = " << beta << endl;

	this->fixPosition();
}

void Camera::setProjection(float w, float h) {
	switch (this->type) {
	case MOVING:
	case PERSPECTIVE:
		loadIdentity(PROJECTION);
		perspective(45.0f, w / h, 0.0001f, 1000000.0f);
		break;
	case ORTHOGONAL:
		loadIdentity(PROJECTION);
		ortho(-w/2, w/2, -w/2, w/2, 0.0001f, 1000000.0f);
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

