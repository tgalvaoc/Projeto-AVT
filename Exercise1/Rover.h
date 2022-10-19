#pragma once
#include "geometry.h"

enum Key { FRONT, BACK, LEFT, RIGHT, NONE };

class Rover {
public: 
	float angle = 0.0f;
	float speed = 0.0f;
	float direction[3] = { 0.0f, 0.0f, 0.0f };
	float position[3] = { 0.0f, 0.0f, 0.0f };
	
	MyObject object;
	Rover();
	Rover(MyObject obj);

	void updateDirection();
};

