#include "Camera.h"

#include "AVTmathlib.h"

#include <iostream>

using namespace std;

Camera::Camera() {};

Camera::Camera(CameraType camType, float x, float y, float z, float targetX_val, float targetY_val, float targetZ_val) {

	position[0] = x;
	position[1] = y;
	position[2] = z;

	target[0] = targetX_val;
	target[1] = targetY_val;
	target[2] = targetZ_val;

	type = camType;

}

void Camera::setProjection(float w, float h) {
	float factor = 3.0f;
	float ratio = w / h;
	switch (this->type) {
	case MOVING:
		loadIdentity(PROJECTION);
		perspective(53.13f, ratio, 0.1f, 1000000.0f);
		break;
	case PERSPECTIVE:
		loadIdentity(PROJECTION);
		perspective(53.13f, ratio, 0.1f, 1000.0f);
		break;
	case ORTHOGONAL:
		loadIdentity(PROJECTION);
		ortho(-16 * factor, 16 * factor, -9 * factor, 9 * factor, -1.0f, 1000000.0f);
		break;
	}

}

void Camera::cameraLookAt() {
	lookAt(this->position[0], this->position[1], this->position[2],
		this->target[0], this->target[1], this->target[2],
		0.0f, 1.0f, 0.0f);
}

