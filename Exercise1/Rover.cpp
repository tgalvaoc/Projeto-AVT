#include "Rover.h"

#include "AVTmathlib.h"
#include <iostream>
#include <tuple>

using namespace std;

Rover::Rover() {};

Rover::Rover(MyObject obj) {
	object = obj;
	angle = 0.0f;
	speed = 0.0f;
	position[0] = 0.0f;
	position[1] = 0.0f;
	position[2] = 0.0f;
	updateDirection();
}

void Rover::updateDirection() {
	float pi = 3.1415f;
	this->direction[0] = float(cos((pi / 180) * this->angle));
	this->direction[2] = float(sin((pi / 180) * this->angle));
	this->direction[1] = 0.0f;
}