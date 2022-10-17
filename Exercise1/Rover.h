#pragma once
#include "geometry.h"

enum Key { FRONT, BACK, LEFT, RIGHT, NONE };

class Rover {
public: 
	int angle;
	float speed;
	float direction[3];
	float position[3];
	
	MyObject object;
	Rover();
	Rover(MyObject obj);

	void updateDirection();
};

