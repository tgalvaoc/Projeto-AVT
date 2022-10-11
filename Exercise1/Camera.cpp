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

void Camera::fixPosition(float alpha, float beta, float r) {
	pos[0] = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	pos[1] = r * sin(beta * 3.14f / 180.0f);
	pos[2] = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
}

void Camera::cameraLookAt() {
	lookAt(this->pos[0], this->pos[1], this->pos[2],
		this->target[0], this->target[1], this->target[2],
		0, 1, 0);
}

