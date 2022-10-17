#pragma once
#include "geometry.h"

enum Key { FRONT, BACK, LEFT, RIGHT, NONE };

class Rover {
public: 
	MyObject rover;
	int angle;
	float speed;
	float direction[3];
	float position[3];

	Rover();
	Rover(MyObject obj);

	void updateDirection();
};

