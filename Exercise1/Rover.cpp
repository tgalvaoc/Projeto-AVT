#include "Rover.h"

#include "AVTmathlib.h"
#include <iostream>
#include <tuple>

using namespace std;

Rover::Rover() {};

Rover::Rover(MyObject obj) {
	rover = obj;
	angle = 0;
	speed = 0;
	position[0] = 0;
	position[1] = 0;
	position[2] = 0;
	updateDirection();
}

void Rover::updateDirection() {
	float pi = 3.1415;
	this->direction[0] = cos((pi / 180) * this->angle);
	this->direction[2] = sin((pi / 180) * this->angle);
	this->direction[1] = 0;
}