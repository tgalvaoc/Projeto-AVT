#pragma once

#include <vector>

enum CameraType { ORTHOGONAL, PERSPECTIVE, MOVING, };

class Camera {
public:
	float pos[3];
	float target[3];

	float alpha, beta, r;

	CameraType type;

	static std::vector<Camera> buildCameras();

	static Camera moving();
	static Camera orthogonal();
	static Camera perspective();

	void setProjection();

	void cameraLookAt();


    void rotateCamera(float dAlpha, float dBeta);

	void translateCamera(float dx, float dy);

	void fixPosition();
};