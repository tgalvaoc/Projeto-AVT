#pragma once

#include <vector>

enum CameraType { ORTHOGONAL, PERSPECTIVE, MOVING, };

class Camera {
public:
	float pos[3];
	float target[3];

	CameraType type;

	Camera();
	Camera(CameraType camType, float x, float y, float z, float targetX_val, float targetY_val, float targetZ_val);

	void setProjection(float w, float h);

	void cameraLookAt();
};