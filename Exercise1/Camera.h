#ifndef CAMERA_H
#define CAMERA_H 
#pragma once


#include <vector>

enum CameraType { ORTHOGONAL, PERSPECTIVE, MOVING, };

class Camera {
public:

	float pos[3];
	float target[3];
	float alpha, beta, r;

	CameraType type;
	Camera();
	Camera(CameraType camType, float x, float y, float z, float r_val, float alpha_val, float beta_val, float targetX_val, float targetY_val, float targetZ_val);

	void setProjection(float w, float h);

	void cameraLookAt();

    void rotateCamera(float dAlpha, float dBeta);

	void translateCamera(float dx, float dy);

	void fixPosition();

	void fixAngles();
};
#endif