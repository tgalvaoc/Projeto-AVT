#pragma once

#include <vector>

enum CameraType { ORTHOGONAL, PERSPECTIVE, MOVING, REAR};

class Camera {
public:
	float position[3] = {0.0f, 0.0f, 0.0f};
	float target[3] = { 0.0f, 0.0f, 0.0f };

	CameraType type = ORTHOGONAL;

	Camera();
	Camera(CameraType camType, float x, float y, float z, float targetX_val, float targetY_val, float targetZ_val);

	void setProjection(float w, float h);

	void cameraLookAt();
};