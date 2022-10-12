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
}

void Rover::rotateRover(Key key) {
	switch (key) {
	case LEFT:
		this->velocity.angle += 1;
		this->velocity.angle = this->velocity.angle % 360;
		cout << "Left: New angle: " << this->velocity.angle << "\n";
		this->velocity.angle = -this->velocity.angle % 360;
		cout << "Left % angle: " << this->velocity.angle << "\n";

		myTranslate(this->rover.objectTransform, 1.5, 0, 0.75);
		myRotate(this->rover.objectTransform, 1, 0, 1, 0);
		myTranslate(this->rover.objectTransform, -1.5, 0, -0.75);
		break;
	case RIGHT:
		this->velocity.angle -= 1;
		cout << "Right New angle: " << this->velocity.angle << "\n";
		this->velocity.angle = - this->velocity.angle % 360;
		cout << "Rigth % angle: " << this->velocity.angle << "\n";

		myTranslate(this->rover.objectTransform, 1.5, 0, 0.75);
		myRotate(this->rover.objectTransform, -1, 0, 1, 0);
		myTranslate(this->rover.objectTransform, -1.5, 0, -0.75);
		break;
	}

	updateDirection();
}

std::tuple<float, float> Rover::updatePosition(Key key) {
	updateDirection();
	switch (key) {
	case FRONT:
		if (this->velocity.speed < 0)
			this->velocity.speed = this->velocity.speed / 2;
		this->velocity.speed -= 0.1;
		break;
	case BACK:
		if (this->velocity.speed > 0)
			this->velocity.speed = this->velocity.speed / 2;
		this->velocity.speed += 0.1;
		break;
	case NONE:
		if (this->velocity.speed < 0)
			this->velocity.speed += 0.002;
		else if(this->velocity.speed != 0)
			this->velocity.speed -= 0.002;
		break;
	}
	//TODO : fix angles
	float translateX = this->velocity.speed * this->velocity.direction[0];
	float translateZ = this->velocity.speed * this->velocity.direction[1];
	position[0] -= translateX;
	position[2] -= translateZ;

	myTranslate(this->rover.objectTransform, -translateX, -translateZ, 0);
	return { translateX, translateZ };
}

void Rover::updateDirection() {
	float pi = 3.1415;
	this->velocity.direction[0] = cos((pi / 180) * this->velocity.angle);
	this->velocity.direction[2] = sin((pi / 180) * this->velocity.angle);
	this->velocity.direction[1] = 0;
}