#include "Camera.h"

#include "AVTmathlib.h"

#include <iostream>

using namespace std;

Camera::Camera() {};

Camera::Camera(CameraType camType, float x, float y, float z, float targetX_val, float targetY_val, float targetZ_val) {

	pos[0] = x;
	pos[1] = y;
	pos[2] = z;

	target[0] = targetX_val;
	target[1] = targetY_val;
	target[2] = targetZ_val;

	type = camType;

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
	int factor = 3;
	switch (this->type) {
	case MOVING:
		loadIdentity(PROJECTION);
		perspective(53.13f, w / h, 0.1f, 1000000.0f);
		break;
	case PERSPECTIVE:
		loadIdentity(PROJECTION);
		perspective(53.13f, w / h, 0.1f, 1000000.0f);
		break;
	case ORTHOGONAL:
		loadIdentity(PROJECTION);
		ortho(-16 * factor, 16 * factor, -9 * factor, 9 * factor, -1, 1000000.0f);
		break;
	}

}

void Camera::fixPosition() { // seno e cosseno estavam invertidos
	pos[0] = r * cosf(alpha * 3.14f / 180.0f) * sinf(beta * 3.14f / 180.0f);
	pos[1] = r * cosf(beta * 3.14f / 180.0f);
	pos[2] = r * sinf(alpha * 3.14f / 180.0f) * sinf(beta * 3.14f / 180.0f);
}

void Camera::cameraLookAt() {
	lookAt(this->pos[0], this->pos[1], this->pos[2],
		this->target[0], this->target[1], this->target[2],
		0, 1, 0);
}

void Camera::fixAngles() {
	//this->r = sqrtf(this->pos[0] * this->pos[0] + this->pos[0] * this->pos[0] + this->pos[0] * this->pos[0]);
}


void Camera::translateCamera(float dx, float dy) {
	//this->pos[0] += sin(this->alpha * 3.14f / 180.0f) * sin(this->beta * 3.14f / 180.0f)
}
