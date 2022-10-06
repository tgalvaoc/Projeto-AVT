#pragma once
#include "geometry.h"

enum Key { FRONT, BACK, LEFT, RIGHT, NONE };

struct Velocity {
	int angle;
	float speed;
	float direction[3];
};

class Rover {
public: 
	MyObject rover;
	Velocity velocity;
	float position[3];

	Rover();
	Rover(MyObject obj);

	void rotateRover(Key key);

	tuple<float, float> updatePosition(Key key);

	void updateDirection();
};

