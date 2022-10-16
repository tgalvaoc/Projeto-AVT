#include "Rover.h"

#include "AVTmathlib.h"
#include <iostream>
#include <tuple>

using namespace std;

Rover::Rover() {};

Rover::Rover(MyObject obj) {
	rover = obj;
	velocity.angle = 0;
	velocity.speed = 0;
	position[0] = 0;
	position[1] = 0;
	position[2] = 0;
	updateDirection();
	cout << "\n------------DIR: 0: " << velocity.direction[0] << " DIR1: " << velocity.direction[1] << " DIR2: " << velocity.direction[2];

}

void Rover::updateDirection() {
	float pi = 3.1415;
	this->velocity.direction[0] = cos((pi / 180) * this->velocity.angle);
	this->velocity.direction[2] = sin((pi / 180) * this->velocity.angle);
	this->velocity.direction[1] = 0;
}