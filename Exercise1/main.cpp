#include <math.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>


// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

#include <IL/il.h>

// assimp include files. These three are usually needed.
#include "assimp/Importer.hpp"	//OO version Header!
#include "assimp/scene.h"

// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "geometry.h"
#include "Texture_Loader.h"
#include "meshFromAssimp.h"
#include "avtFreeType.h"

#include "flare.h"
#include "Camera.h"
#include "Rover.h"

#include <list>
#define _USE_MATH_DEFINES

using namespace std;

#define CAPTION "Projecto AVT"


// Created an instance of the Importer class in the meshFromAssimp.cpp file
extern Assimp::Importer importer;
// the global Assimp scene object
extern const aiScene* scene;
string model_dir;  //initialized by the user input at the console
// scale factor for the Assimp model to fit in the window
extern float scaleFactor;


int WindowHandle = 0;
int WinX = 1280, WinY = 720;

unsigned int FrameCount = 0;
float delta = 0.015f;

float alpha, beta, r;

#define M_PI       3.14159265358979323846f
#define frand()			((float)rand()/RAND_MAX)

//shaders
VSShaderLib shader;  //geometry
VSShaderLib shaderText;  //render bitmap text

//File with the font
const string font_name = "fonts/arial.ttf";

//Vector with objects, and each object can have one or more meshes
vector<MyObject> myObjects;
MyMesh particleMesh;
vector<struct MyMesh> spaceship;
MyObject skyboxCube;

Rover rover;
LandingSite landingSiteRover;
LandingSite landingSiteSpaceship;
AliensBase aliensBase;
vector<RollingRock> rollingRocks;
vector<StaticRock> staticRocks;
vector<Item> items;
vector<Flag> flags;
vector<Pillar> pillars;
vector<Alien> aliens;

//Flare effect
FLARE_DEF flare;
float lightScreenPos[3];  //Position of the light in Window Coordinates
GLuint FlareTextureArray[5];
MyMesh flareMesh;

// mirror
MyMesh mirror;
bool mirror_mode = false;

// rear view camera object
MyMesh rearViewCam;
bool rear_view_cam_mode = false;

bool pauseActive = false;
bool gameOver = false;

bool spotlight_mode = true;
bool sun_mode = true;
bool point_lights_mode = false;
bool fog_mode = false;
bool multitexture_mode = false;
bool bumpMapping = false;
bool flareEffect = false;

bool normalMapKey = TRUE; // by default if there is a normal map then bump effect is implemented. press key "b" to enable/disable normal mapping 
int reflect_perFragment = 0; //Reflection vetor for cube mapping computed in vertex shader(= 0) or fragment shader (= 1)

bool isGoingForward = false;
bool isRoverHittingSomething = false;

GLuint TextureArray[12];

//External array storage defined in AVTmathLib.cpp

/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint texMode_uniformId;
GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;
GLint lPos_uniformId; // ?
GLint texMap0, texMap1, texText;
GLint texMode;
GLint shadowMode;

GLint normalMap_loc;
GLint specularMap_loc;
GLint diffMapCount_loc;
GLint tex_cube_loc;
GLint reflect_perFragment_uniformId;
GLint model_uniformId;
GLint view_uniformId;



Camera cameras[4];
int currentCamera = 0;

int livesCount;
int points = 0;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Frame counting and FPS computation
long myTime, timebase = 0, frame = 0;
char s[32];

#define NUMBER_POINT_LIGHTS 6
#define NUMBER_SPOT_LIGHTS 2
#define MAX_PARTICLES 50

//float directionalLightPos[4] = { -5.0f, 2.0f, 1.0f, 1.0f };
float directionalLightPos[4] = { 1000.0f, 1000.0f, 1.0f, 1.0f };
float pointLightPos[NUMBER_POINT_LIGHTS][4] = { {-5.0f, 8.0f, -5.0f, 1.0f}, {-5.0f, 8.0f, 5.0f, 1.0f},
	{5.0f, 8.0f, -5.0f, 1.0f}, {5.0f, 8.0f, 5.0f, 1.0f}, {-5.0f, 8.0f, 0.0f, 1.0f},
	{5.0f, 8.0f, 0.0f, 1.0f} };
float spotlightPos[NUMBER_SPOT_LIGHTS][4];
float coneDir[4];

typedef struct {
	float	life;		// vida
	float	fade;		// fade
	float	r, g, b;    // color
	GLfloat x, y, z;    // posi‹o
	GLfloat vx, vy, vz; // velocidade 
	GLfloat ax, ay, az; // acelera‹o
} Particle;

vector <Particle> particles;
int dead_num_particles = 0;


void setMeshColor(MyMesh* amesh, float r, float g, float b, float a)
{
	float amb[] = { r / 4.0f, g / 4.0f, b / 4.0f, 1.0f };
	float diff[] = { r, g, b, a };

	float spec[] = { r, g, b, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	memcpy(amesh->mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh->mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh->mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh->mat.emissive, emissive, 4 * sizeof(float));

	amesh->mat.shininess = 50.0f;
	amesh->mat.texCount = 0;
}


void initialState(bool livesReset) {

	if (livesReset) {
		livesCount = 5;
		points = 0;
	}

	rover.position[0] = 0.0f;
	rover.position[1] = 0.0f;
	rover.position[2] = 0.0f;
	rover.angle = 0.0f;
	rover.speed = 0.0f;

	cameras[2].position[0] = 10.0f;
	cameras[2].position[1] = 4.0f;
	cameras[2].position[2] = 0.0f;


	r = float(sqrt(pow(cameras[2].position[0], 2) + pow(cameras[2].position[1], 2) + pow(cameras[2].position[2], 2)));
	alpha = float(acos(cameras[2].position[2] / r) * 180.0 / M_PI);
	beta = float(atan(cameras[2].position[1] / cameras[2].position[0]) * 180.0 / M_PI);

	for (int i = 0; i < staticRocks.size(); i++) {
		staticRocks[i].position[0] = staticRocks[i].originalPos[0];
		staticRocks[i].position[1] = staticRocks[i].originalPos[1];
		staticRocks[i].position[2] = staticRocks[i].originalPos[2];
	}
}

void createCameras() {
	cameras[0] = Camera(ORTHOGONAL, 1.0f, 20.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	cameras[1] = Camera(PERSPECTIVE, 1.0f, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	cameras[2] = Camera(MOVING, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f);
	cameras[3] = Camera(MOVING, -5.0f, 3.0f, 0.0f, 0.0f, 1.5f, 0.0f); //rear view camera
}

int signal() {
	int signal = rand() % 2;
	if (signal)
		return -1;
	return 1;
}

void createItems(int numToCreate) {
	MyMesh amesh = createTorus(0.8f, 1.0f, 3, 10); // randomize size in the future
	int min = 5, max = 40;

	setMeshColor(&amesh, 1.0f, 1.0f, 1.0f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);

	for (int i = 0; i < numToCreate; i++) {

		Item item;
		float r1 = rand() % (max - min + 1) + min;
		float r2 = rand() % (max - min + 1) + min;

		item.position[0] = signal() * (signal() * r1 + rover.position[0]);
		item.position[1] = 1.5;
		item.position[2] = signal() * (signal() * r2 + rover.position[2]);

		item.radius = 1.0f;

		item.object.meshes.push_back(amesh);

		setIdentityMatrix(item.object.objectTransform, 4);

		myTranslate(item.object.objectTransform, item.position[0], 1, item.position[2]);
		myRotate(item.object.objectTransform, 90.0f, 0.0f, 0.0f, 1.0f);
		myRotate(item.object.objectTransform, item.position[0], 0.0f, 1.0f, 0.0f);
		myRotate(item.object.objectTransform, item.position[0], 1.0f, 0.0f, 0.0f);

		items.push_back(item);
	}
}


vector<RollingRock> createRollingRocks(int numToCreate) {

	MyMesh amesh = createSphere(1.0f, 10); // randomize size in the future
	MyObject stone;
	RollingRock rock;
	vector<RollingRock> rocks;
	int low, high;
	int signal;

	if (numToCreate == 0)
		return rocks;

	float square_size = 50.0f;

	float amb[] = { 0.2f, 0.15f, 0.1f, 1.0f };
	float diff[] = { 0.8f, 0.6f, 0.4f, 1.0f };
	float spec[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));

	amesh.mat.shininess = 100.0f;
	amesh.mat.texCount = 0;

	setIdentityMatrix(amesh.meshTransform, 4);

	for (int i = 0; i < numToCreate; i++) {

		low = 5;
		high = 30;
		signal = rand() % 2;

		float r1 = low + static_cast<float>(rand()) * static_cast<float>(high - low) / RAND_MAX;
		if (signal)
			r1 = -r1;
		float r2 = low + static_cast<float>(rand()) * static_cast<float>(high - low) / RAND_MAX;
		if (signal)
			r2 = -r2;

		low = 0;
		high = 1;
		signal = rand() % 2;
		rock.speed = low + static_cast<float>(rand()) * static_cast<float>(high - low) / RAND_MAX;
		if (signal)
			rock.speed = -rock.speed;

		low = 0;
		high = 1;

		signal = rand() % 2;
		rock.direction[0] = low + static_cast<float>(rand()) * static_cast<float>(high - low) / RAND_MAX;
		if (signal)
			rock.direction[0] = -rock.direction[0];

		signal = rand() % 2;
		rock.direction[2] = low + static_cast<float>(rand()) * static_cast<float>(high - low) / RAND_MAX;
		if (signal)
			rock.direction[2] = -rock.direction[2];


		signal = rand() % 2;
		rock.position[0] = r1 + rover.position[0];
		if (signal) {
			rock.position[0] = -rock.position[0];
		}
		rock.position[1] = 0;
		signal = rand() % 2;
		rock.position[2] = r2 + rover.position[2];
		if (signal)
			rock.position[2] = -rock.position[2];

		rock.radius = 1.0f;

		setIdentityMatrix(stone.objectTransform, 4);

		myTranslate(stone.objectTransform, rock.position[0], 1, rock.position[2]);

		stone.meshes.push_back(amesh);
		rock.object = stone;
		rollingRocks.push_back(rock);
	}
	return rocks;
}


void initParticles(void) {
	GLfloat v, theta, phi;
	int i, j;
	Particle p;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < MAX_PARTICLES; j++) {
			v = 0.8f * frand() + 0.2f;
			phi = frand() * M_PI;
			theta = 2.0f * frand() * M_PI;
			switch (i) {
			case 1: { // back wheel right
				p.x = rover.position[0] + rover.direction[0] * 1.2f - 0.7f * rover.direction[2];
				p.y = rover.position[1] + 0.1f;
				p.z = rover.position[2] - rover.direction[2] * 1.2f - 0.7f * rover.direction[0];
				break;
			}
			case 2: { // back wheel left
				p.x = rover.position[0] + rover.direction[0] * 1.2f + 0.7f * rover.direction[2];
				p.y = rover.position[1] + 0.1f;
				p.z = rover.position[2] - rover.direction[2] * 1.2f + 0.7f * rover.direction[0];
				break;
			}
			case 3: { // front left wheel
				p.x = rover.position[0] - rover.direction[0] * 1.2f + 0.7f * rover.direction[2];
				p.y = rover.position[1] + 0.1f;
				p.z = rover.position[2] + rover.direction[2] * 1.2f + 0.7f * rover.direction[0];
				break;
			}
			case 4: {
				p.x = rover.position[0] - rover.direction[0] * 1.2f - 0.7f * rover.direction[2];
				p.y = rover.position[1] + 0.1f;
				p.z = rover.position[2] + rover.direction[2] * 1.2f - 0.7f * rover.direction[0];
				break;
			}
			}
			//p.x = rover.position[0] + rover.direction[0] * 2;
			//p.y = rover.position[1] + 0.75;
			//p.z = rover.position[2] - rover.direction[2] * 2;
			p.vx = v * cos(theta) * sin(phi);
			p.vy = v * cos(phi);
			p.vz = v * sin(theta) * sin(phi);
			p.ax = 0.1f;   /* simulates wind */
			p.ay = -0.25f; /* simulates gravity */
			p.az = 0.0f;

			p.r = 0.8f;
			p.g = 0.4f;
			p.b = 0.0f;

			p.life = 0.8f;		/* initial life */
			p.fade = 0.008f;	    /* step to decrease life in each iteration */
			particles.push_back(p);
		}
	}
}


void updateParticles()
{
	int i;
	float h;

	/* Método de Euler de integração de eq. diferenciais ordinárias
	h represents the time step; dv/dt = a; dx/dt = v; initial values of x and v */

	//h = 0.125f;
	h = 0.033f;

	for (i = 0; i < particles.size(); i++) {
		particles[i].x += (h * particles[i].vx);
		particles[i].y += (h * particles[i].vy);
		particles[i].z += (h * particles[i].vz);
		particles[i].vx += (h * particles[i].ax);
		particles[i].vy += (h * particles[i].ay);
		particles[i].vz += (h * particles[i].az);
		particles[i].life -= particles[i].fade;

		if (particles[i].life <= 0)
			particles.erase(particles.begin() + i);
	}

}

void updateItems() {
	for (int i = 0; i < items.size(); i++) {
		myRotate(items[i].object.objectTransform, 1, 1, 0, 0);
	}

	vector< Item >::iterator it = items.begin();
	int numErased = 0;

	while (it != items.end()) {

		if ((*it).position[0] > rover.position[0] + 200 || (*it).position[2] > rover.position[2] + 200 ||
			(*it).position[0] < rover.position[0] - 200 || (*it).position[2] < rover.position[2] - 200) {

			it = items.erase(it);
			numErased++;
		}
		else ++it;
	}

	createItems(numErased);
}

void updateRollingRocks() {

	for (int i = 0; i < rollingRocks.size(); i++) {
		rollingRocks[i].speed += rollingRocks[i].speed * 0.005f;
		float translateX = rollingRocks[i].speed * rollingRocks[i].direction[0];
		float translateZ = rollingRocks[i].speed * rollingRocks[i].direction[2];

		myTranslate(rollingRocks[i].object.objectTransform, translateX, 0, translateZ);
		rollingRocks[i].position[0] += translateX;
		rollingRocks[i].position[2] += translateZ;
	}

	vector< RollingRock >::iterator it = rollingRocks.begin();
	int numErased = 0;

	while (it != rollingRocks.end()) {

		if ((*it).position[0] > rover.position[0] + 200 || (*it).position[2] > rover.position[2] + 200 ||
			(*it).position[0] < rover.position[0] - 200 || (*it).position[2] < rover.position[2] - 200) {

			it = rollingRocks.erase(it);
			numErased++;
		}
		else ++it;
	}

	createRollingRocks(numErased);
}

void updateStaticRocks() {
	for (int i = 0; i < staticRocks.size(); i++) {

		float translateX = staticRocks[i].speed * staticRocks[i].direction[0];
		float translateZ = staticRocks[i].speed * staticRocks[i].direction[2];

		myTranslate(staticRocks[i].object.objectTransform, translateX, 0, translateZ);

		staticRocks[i].position[0] += translateX;
		staticRocks[i].position[2] += translateZ;

		if (staticRocks[i].speed != 0)
			staticRocks[i].speed += -staticRocks[i].speed / 8;
	}

}

void timerFPS(int value)
{
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ") " << value;
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
	FrameCount = 0;
	glutTimerFunc(1000, timerFPS, value + 1);
}

void refresh(int value)
{
	// atualizar a tela a 60fps significa 60 frames por segundo, ou seja, com um intervalo de 1 / 60 = 16.6666 ms a cada frame

	// por causa disso, a funcao refresh vai ser chamada daqui a 16 ms pelo glutTimerFunc
	glutPostRedisplay(); //desenha a janela
	glutTimerFunc(1000 / 60, refresh, 0); // continua chamando refresh(0)
}


// ------------------------------------------------------------
//
// Animate stufff
//

void updateSpotlight() {
	coneDir[0] = -rover.direction[0];
	coneDir[1] = rover.direction[1];
	coneDir[2] = rover.direction[2];
	coneDir[3] = 0.0f;

	spotlightPos[0][0] = rover.position[0] - 0.3f * rover.direction[0] + 0.6f * rover.direction[2];
	spotlightPos[0][1] = rover.position[1] + 0.5f;
	spotlightPos[0][2] = rover.position[2] - 0.4f * rover.direction[2] + 0.6f * rover.direction[0];
	spotlightPos[0][3] = 1.0f;

	spotlightPos[1][0] = rover.position[0] - 0.3f * rover.direction[0] - 0.6f * rover.direction[2];
	spotlightPos[1][1] = rover.position[1] + 0.5f;
	spotlightPos[1][2] = rover.position[2] - 0.4f * rover.direction[2] - 0.6f * rover.direction[0];
	spotlightPos[1][3] = 1.0f;
}

void updateRoverPosition() {
	rover.updateDirection();

	rover.position[0] -= rover.direction[0] * rover.speed * delta;
	rover.position[2] += rover.direction[2] * rover.speed * delta;

	if (rover.speed != 0)
		rover.speed += -2 * rover.speed * delta;

}

void updateRoverCamera() {
	cameras[2].position[0] = rover.position[0] + rover.direction[0] * 10;
	cameras[2].position[1] = 4;
	cameras[2].position[2] = rover.position[2] - rover.direction[2] * 10;

	std::copy(rover.position, rover.position + 3, cameras[2].target);

	cameras[3].position[0] = rover.position[0] - rover.direction[0] * 10;
	cameras[3].position[1] = 4;
	cameras[3].position[2] = rover.position[2] - rover.direction[2] * 10;

	//std::copy(rover.position, rover.position + 3, cameras[3].target);
}

void checkCollisions() {

	float roverFactor = 2.0;

	float roverMaxX = rover.position[0] + roverFactor;
	float roverMinX = rover.position[0] - roverFactor;
	float roverMaxZ = rover.position[2] + roverFactor;
	float roverMinZ = rover.position[2] - roverFactor;

	// collision with static rocks
	isRoverHittingSomething = false;
	for (int i = 0; i < staticRocks.size(); i++) {

		float maxX = staticRocks[i].position[0] + staticRocks[i].radius;
		float minX = staticRocks[i].position[0] - staticRocks[i].radius;
		float maxZ = staticRocks[i].position[2] + staticRocks[i].radius;
		float minZ = staticRocks[i].position[2] - staticRocks[i].radius;

		if ((minX <= roverMaxX && minX >= roverMinX && staticRocks[i].position[2] >= roverMinZ && staticRocks[i].position[2] <= roverMaxZ) ||
			(maxX <= roverMaxX && maxX >= roverMinX && staticRocks[i].position[2] >= roverMinZ && staticRocks[i].position[2] <= roverMaxZ) ||
			(staticRocks[i].position[0] <= roverMaxX && staticRocks[i].position[0] >= roverMinX && minZ >= roverMinZ && minZ <= roverMaxZ) ||
			(staticRocks[i].position[0] <= roverMaxX && staticRocks[i].position[0] >= roverMinX && maxZ >= roverMinZ && maxZ <= roverMaxZ)) {

			staticRocks[i].speed = abs(rover.speed * 0.01f / staticRocks[i].radius);

			rover.position[0] += rover.direction[0] * rover.speed * delta;
			rover.position[2] -= rover.direction[2] * rover.speed * delta;

			rover.speed = 0;

			if (isGoingForward) {
				staticRocks[i].direction[0] = -rover.direction[0];
				staticRocks[i].direction[2] = rover.direction[2];
			}
			else {
				staticRocks[i].direction[0] = rover.direction[0];
				staticRocks[i].direction[2] = -rover.direction[2];
			}

			isRoverHittingSomething = true;
			break;
		}
	}

	// collision with rolling rocks
	for (int i = 0; i < rollingRocks.size(); i++) {
		RollingRock rock = rollingRocks[i];

		float maxX = rock.position[0] + rock.radius;
		float minX = rock.position[0] - rock.radius;
		float maxZ = rock.position[2] + rock.radius;
		float minZ = rock.position[2] - rock.radius;

		// rolling rocks + rover
		if ((minX <= roverMaxX && minX >= roverMinX && rock.position[2] >= roverMinZ && rock.position[2] <= roverMaxZ) ||
			(maxX <= roverMaxX && maxX >= roverMinX && rock.position[2] >= roverMinZ && rock.position[2] <= roverMaxZ) ||
			(rock.position[0] <= roverMaxX && rock.position[0] >= roverMinX && minZ >= roverMinZ && minZ <= roverMaxZ) ||
			(rock.position[0] <= roverMaxX && rock.position[0] >= roverMinX && maxZ >= roverMinZ && maxZ <= roverMaxZ)) {


			if (--livesCount <= 0) {
				livesCount = 0;
				gameOver = true;
			}
			else {
				initialState(false);
				rollingRocks.clear();
				createRollingRocks(7);
				break;
			}
		}

		float groundMaxX = landingSiteRover.position[0] + landingSiteRover.side / 2 + 0.5f;
		float groundMinX = landingSiteRover.position[0] - landingSiteRover.side / 2 + 0.5f;
		float groundMaxZ = landingSiteRover.position[2] + landingSiteRover.side / 2 + 0.5f;
		float groundMinZ = landingSiteRover.position[2] - landingSiteRover.side / 2 + 0.5f;

		// rolling rocks + landing site rover
		if ((minX <= groundMaxX && minX >= groundMinX && rock.position[2] >= groundMinZ && rock.position[2] <= groundMaxZ) ||
			(maxX <= groundMaxX && maxX >= groundMinX && rock.position[2] >= groundMinZ && rock.position[2] <= groundMaxZ) ||
			(rock.position[0] <= groundMaxX && rock.position[0] >= groundMinX && minZ >= groundMinZ && minZ <= groundMaxZ) ||
			(rock.position[0] <= groundMaxX && rock.position[0] >= groundMinX && maxZ >= groundMinZ && maxZ <= groundMaxZ)) {

			rollingRocks.erase(rollingRocks.begin() + i);
			createRollingRocks(1);
			break;

		}

		// rolling rocks + landing site spaceship
		groundMaxX = landingSiteSpaceship.position[0] + landingSiteSpaceship.side / 2 + 0.5f;
		groundMinX = landingSiteSpaceship.position[0] - landingSiteSpaceship.side / 2 + 0.5f;
		groundMaxZ = landingSiteSpaceship.position[2] + landingSiteSpaceship.side / 2 + 0.5f;
		groundMinZ = landingSiteSpaceship.position[2] - landingSiteSpaceship.side / 2 + 0.5f;

		if ((minX <= groundMaxX && minX >= groundMinX && rock.position[2] >= groundMinZ && rock.position[2] <= groundMaxZ) ||
			(maxX <= groundMaxX && maxX >= groundMinX && rock.position[2] >= groundMinZ && rock.position[2] <= groundMaxZ) ||
			(rock.position[0] <= groundMaxX && rock.position[0] >= groundMinX && minZ >= groundMinZ && minZ <= groundMaxZ) ||
			(rock.position[0] <= groundMaxX && rock.position[0] >= groundMinX && maxZ >= groundMinZ && maxZ <= groundMaxZ)) {

			rollingRocks.erase(rollingRocks.begin() + i);
			createRollingRocks(1);
			break;

		}

		//rover + landing site spaceship
		if ((groundMinX <= roverMaxX && groundMinX >= roverMinX && landingSiteSpaceship.position[2] >= roverMinZ && landingSiteSpaceship.position[2] <= roverMaxZ) ||
			(groundMaxX <= roverMaxX && groundMaxX >= roverMinX && landingSiteSpaceship.position[2] >= roverMinZ && landingSiteSpaceship.position[2] <= roverMaxZ) ||
			(landingSiteSpaceship.position[0] <= roverMaxX && landingSiteSpaceship.position[0] >= roverMinX && groundMinZ >= roverMinZ && groundMinZ <= roverMaxZ) ||
			(landingSiteSpaceship.position[0] <= roverMaxX && landingSiteSpaceship.position[0] >= roverMinX && groundMaxZ >= roverMinZ && groundMaxZ <= roverMaxZ)) {

			rover.position[0] += rover.direction[0] * rover.speed * delta;
			rover.position[2] -= rover.direction[2] * rover.speed * delta;
			rover.speed = 0;

			isRoverHittingSomething = true;
			break;
		}

		// rolling rocks + aliens base
		groundMaxX = aliensBase.position[0] + aliensBase.side / 2 + 0.5f;
		groundMinX = aliensBase.position[0] - aliensBase.side / 2 + 0.5f;
		groundMaxZ = aliensBase.position[2] + aliensBase.side / 2 + 0.5f;
		groundMinZ = aliensBase.position[2] - aliensBase.side / 2 + 0.5f;

		if ((minX <= groundMaxX && minX >= groundMinX && rock.position[2] >= groundMinZ && rock.position[2] <= groundMaxZ) ||
			(maxX <= groundMaxX && maxX >= groundMinX && rock.position[2] >= groundMinZ && rock.position[2] <= groundMaxZ) ||
			(rock.position[0] <= groundMaxX && rock.position[0] >= groundMinX && minZ >= groundMinZ && minZ <= groundMaxZ) ||
			(rock.position[0] <= groundMaxX && rock.position[0] >= groundMinX && maxZ >= groundMinZ && maxZ <= groundMaxZ)) {

			rollingRocks.erase(rollingRocks.begin() + i);
			createRollingRocks(1);
			break;

		}

		//rover + aliensBase
		if ((groundMinX <= roverMaxX && groundMinX >= roverMinX && aliensBase.position[2] >= roverMinZ && aliensBase.position[2] <= roverMaxZ) ||
			(groundMaxX <= roverMaxX && groundMaxX >= roverMinX && aliensBase.position[2] >= roverMinZ && aliensBase.position[2] <= roverMaxZ) ||
			(aliensBase.position[0] <= roverMaxX && aliensBase.position[0] >= roverMinX && groundMinZ >= roverMinZ && groundMinZ <= roverMaxZ) ||
			(aliensBase.position[0] <= roverMaxX && aliensBase.position[0] >= roverMinX && groundMaxZ >= roverMinZ && groundMaxZ <= roverMaxZ)) {

			rover.position[0] += rover.direction[0] * rover.speed * delta;
			rover.position[2] -= rover.direction[2] * rover.speed * delta;
			rover.speed = 0;

			isRoverHittingSomething = true;
			break;
		}

	}

	// collision with pillars
	isRoverHittingSomething = false;
	for each (Pillar pillar in pillars) {

		float maxX = pillar.position[0] + pillar.radius;
		float minX = pillar.position[0] - pillar.radius;
		float maxZ = pillar.position[2] + pillar.radius;
		float minZ = pillar.position[2] - pillar.radius;

		if ((minX <= roverMaxX && minX >= roverMinX && pillar.position[2] >= roverMinZ && pillar.position[2] <= roverMaxZ) ||
			(maxX <= roverMaxX && maxX >= roverMinX && pillar.position[2] >= roverMinZ && pillar.position[2] <= roverMaxZ) ||
			(pillar.position[0] <= roverMaxX && pillar.position[0] >= roverMinX && minZ >= roverMinZ && minZ <= roverMaxZ) ||
			(pillar.position[0] <= roverMaxX && pillar.position[0] >= roverMinX && maxZ >= roverMinZ && maxZ <= roverMaxZ)) {

			rover.position[0] += rover.direction[0] * rover.speed * delta;
			rover.position[2] -= rover.direction[2] * rover.speed * delta;
			rover.speed = 0;

			isRoverHittingSomething = true;
			break;
		}
	}


	// collision with items
	for (int i = 0; i < items.size(); i++) {

		Item item = items[i];

		float maxX = item.position[0] + item.radius;
		float minX = item.position[0] - item.radius;
		float maxZ = item.position[2] + item.radius;
		float minZ = item.position[2] - item.radius;

		if ((roverMinX >= minX && roverMinX <= maxX && rover.position[2] >= minZ && rover.position[2] <= maxZ) ||
			(roverMaxX >= minX && roverMaxX <= maxX && rover.position[2] >= minZ && rover.position[2] <= maxZ) ||
			(roverMinZ >= minZ && roverMinZ <= maxZ && rover.position[0] >= minX && rover.position[0] <= maxX) ||
			(roverMaxZ >= minZ && roverMaxZ <= maxZ && rover.position[0] >= minX && rover.position[0] <= maxX)) {

			items.erase(items.begin() + i);
			createItems(1);
			points += 200;
			if (points % 1000 == 0 && livesCount < 5)
				livesCount++;
			break;
		}
	}

}


void animate(int value) {

	if (!pauseActive && !gameOver) {

		checkCollisions();

		updateRoverPosition();
		updateSpotlight();
		updateRollingRocks();
		updateStaticRocks();
		updateItems();

		if (tracking == 0)
			updateRoverCamera();
	}

	glutTimerFunc(20, animate, 0);
}

// ------------------------------------------------------------
//
// Render stuff
//

// Recursive render of the Assimp Scene Graph

void aiRecursive_render(const aiScene* sc, const aiNode* nd)
{
	GLint loc;

	// Get node transformation matrix
	aiMatrix4x4 m = nd->mTransformation;
	// OpenGL matrices are column major
	m.Transpose();

	// save model matrix and apply node transformation
	pushMatrix(MODEL);

	float aux[16];
	memcpy(aux, &m, sizeof(float) * 16);
	multMatrix(MODEL, aux);

	translate(MODEL, 4.0f, 0.0f, -28.0f);
	rotate(MODEL, 60.0f, 0.0f, 1.0f, 0.0f);
	scale(MODEL, 0.12f, 0.14f, 0.12f);


	// draw all meshes assigned to this node
	for (unsigned int n = 0; n < nd->mNumMeshes; ++n) {
		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, spaceship[nd->mMeshes[n]].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, spaceship[nd->mMeshes[n]].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, spaceship[nd->mMeshes[n]].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.emissive");
		glUniform4fv(loc, 1, spaceship[nd->mMeshes[n]].mat.emissive);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, spaceship[nd->mMeshes[n]].mat.shininess);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.texCount");
		glUniform1i(loc, spaceship[nd->mMeshes[n]].mat.texCount);

		unsigned int  diffMapCount = 0;  //read 2 diffuse textures

		//devido ao fragment shader suporta 2 texturas difusas simultaneas, 1 especular e 1 normal map

		glUniform1i(normalMap_loc, false);   //GLSL normalMap variable initialized to 0
		glUniform1i(specularMap_loc, false);
		glUniform1ui(diffMapCount_loc, 0);

		if (spaceship[nd->mMeshes[n]].mat.texCount != 0)
			for (unsigned int i = 0; i < spaceship[nd->mMeshes[n]].mat.texCount; ++i) {
				if (spaceship[nd->mMeshes[n]].texTypes[i] == DIFFUSE) {
					if (diffMapCount == 0) {
						diffMapCount++;
						loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitDiff");
						glUniform1i(loc, spaceship[nd->mMeshes[n]].texUnits[i]);
						glUniform1ui(diffMapCount_loc, diffMapCount);
					}
					else if (diffMapCount == 1) {
						diffMapCount++;
						loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitDiff1");
						glUniform1i(loc, spaceship[nd->mMeshes[n]].texUnits[i]);
						glUniform1ui(diffMapCount_loc, diffMapCount);
					}
					else printf("Only supports a Material with a maximum of 2 diffuse textures\n");
				}
				else if (spaceship[nd->mMeshes[n]].texTypes[i] == SPECULAR) {
					loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitSpec");
					glUniform1i(loc, spaceship[nd->mMeshes[n]].texUnits[i]);
					glUniform1i(specularMap_loc, true);
				}
				else if (spaceship[nd->mMeshes[n]].texTypes[i] == NORMALS) { //Normal map
					loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitNormalMap");
					if (normalMapKey)
						glUniform1i(normalMap_loc, normalMapKey);
					glUniform1i(loc, spaceship[nd->mMeshes[n]].texUnits[i]);

				}
				else printf("Texture Map not supported\n");
			}

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// bind VAO
		glBindVertexArray(spaceship[nd->mMeshes[n]].vao);

		if (!shader.isProgramValid()) {
			printf("Program Not Valid!\n");
			exit(1);
		}
		// draw
		glDrawElements(spaceship[nd->mMeshes[n]].type, spaceship[nd->mMeshes[n]].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	// draw all children
	for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
		aiRecursive_render(sc, nd->mChildren[n]);
	}
	popMatrix(MODEL);
	//glUniform1i(normalMap_loc, false);   //GLSL normalMap variable initialized to 0
	//glUniform1i(specularMap_loc, false);
	glUniform1ui(diffMapCount_loc, 0);
}

unsigned int getTextureId(char* name) {
	int i;

	for (i = 0; i < NUMBER_OF_TEXTURES; ++i)
	{
		if (strncmp(name, flareTextureNames[i], strlen(name)) == 0)
			return i;
	}
	return -1;
}

inline double clamp(const double x, const double min, const double max) {
	return (x < min ? min : (x > max ? max : x));
}

inline int clampi(const int x, const int min, const int max) {
	return (x < min ? min : (x > max ? max : x));
}


void loadFlareFile(FLARE_DEF* flare, char* filename)
{
	int     n = 0;
	FILE* f;
	char    buf[256];
	int fields;

	memset(flare, 0, sizeof(FLARE_DEF));

	f = fopen(filename, "r");
	if (f)
	{
		fgets(buf, sizeof(buf), f);
		sscanf(buf, "%f %f", &flare->scale, &flare->maxSize);

		while (!feof(f))
		{
			char            name[8] = { '\0', };
			double          dDist = 0.0, dSize = 0.0;
			float			color[4];
			int				id;

			fgets(buf, sizeof(buf), f);
			fields = sscanf(buf, "%4s %lf %lf ( %f %f %f %f )", name, &dDist, &dSize, &color[3], &color[0], &color[1], &color[2]);
			if (fields == 7)
			{
				for (int i = 0; i < 4; ++i) color[i] = clamp(color[i] / 255.0f, 0.0f, 1.0f);
				id = getTextureId(name);
				if (id < 0) printf("Texture name not recognized\n");
				else
					flare->element[n].textureId = id;
				flare->element[n].distance = (float)dDist;
				flare->element[n].size = (float)dSize;
				memcpy(flare->element[n].matDiffuse, color, 4 * sizeof(float));
				++n;
			}
		}

		flare->numberOfPieces = n;
		fclose(f);
	}
	else printf("Flare file opening error\n");
}

void renderFlare(FLARE_DEF* flare, int lx, int ly, int* m_viewport) {  //lx, ly represent the projected position of light on viewport

	int     dx, dy;          // Screen coordinates of "destination"
	int     px, py;          // Screen coordinates of flare element
	int		cx, cy;
	float   maxflaredist, flaredist, flaremaxsize, flarescale, scaleDistance;
	int     width, height, alpha;    // Piece parameters;
	int     i;
	float	diffuse[4];

	GLint loc;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int screenMaxCoordX = m_viewport[0] + m_viewport[2] - 1;
	int screenMaxCoordY = m_viewport[1] + m_viewport[3] - 1;

	//viewport center
	cx = m_viewport[0] + (int)(0.5f * (float)m_viewport[2]) - 1;
	cy = m_viewport[1] + (int)(0.5f * (float)m_viewport[3]) - 1;

	// Compute how far off-center the flare source is.
	maxflaredist = sqrt(cx * cx + cy * cy);
	flaredist = sqrt((lx - cx) * (lx - cx) + (ly - cy) * (ly - cy));
	scaleDistance = (maxflaredist - flaredist) / maxflaredist;
	flaremaxsize = (int)(m_viewport[2] * flare->maxSize);
	flarescale = (int)(m_viewport[2] * flare->scale);

	// Destination is opposite side of centre from source
	dx = clampi(cx + (cx - lx), m_viewport[0], screenMaxCoordX);
	dy = clampi(cy + (cy - ly), m_viewport[1], screenMaxCoordY);

	// Render each element. To be used Texture Unit 0

	glUniform1i(texMode, 1); // draw modulated textured particles

	for (i = 0; i < flare->numberOfPieces; ++i)
	{
		// Position is interpolated along line between start and destination.
		px = (int)((1.0f - flare->element[i].distance) * lx + flare->element[i].distance * dx);
		py = (int)((1.0f - flare->element[i].distance) * ly + flare->element[i].distance * dy);
		px = clampi(px, m_viewport[0], screenMaxCoordX);
		py = clampi(py, m_viewport[1], screenMaxCoordY);

		// Piece size are 0 to 1; flare size is proportion of screen width; scale by flaredist/maxflaredist.
		width = (int)(scaleDistance * flarescale * flare->element[i].size);

		// Width gets clamped, to allows the off-axis flaresto keep a good size without letting the elements get big when centered.
		if (width > flaremaxsize)  width = flaremaxsize;

		height = (int)((float)m_viewport[3] / (float)m_viewport[2] * (float)width);
		memcpy(diffuse, flare->element[i].matDiffuse, 4 * sizeof(float));
		diffuse[3] *= scaleDistance;   //scale the alpha channel

		if (width > 1)
		{
			// send the material - diffuse color modulated with texture
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
			glUniform4fv(loc, 1, diffuse);

			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, FlareTextureArray[flare->element[i].textureId]);
			glUniform1i(texMap0, 4);
			//glUniform1i(texMode, 1);
			pushMatrix(MODEL);
			translate(MODEL, (float)(px - width * 0.0f), (float)(py - height * 0.0f), 0.0f);
			scale(MODEL, (float)width, (float)height, 1);
			computeDerivedMatrix(PROJ_VIEW_MODEL);
			glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

			glBindVertexArray(flareMesh.vao);
			glDrawElements(flareMesh.type, flareMesh.numIndexes, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			popMatrix(MODEL);
		}
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

static void draw_mirror(void)
{
	GLint loc;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mirror.mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mirror.mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mirror.mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mirror.mat.shininess);
	pushMatrix(MODEL);
	translate(MODEL, -0.5f, 0.0f, -0.5f);
	rotate(MODEL, 270, 1, 0, 0);
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	//glUniform1i(texMode_uniformId, 2);
	glBindVertexArray(mirror.vao);
	glDrawElements(mirror.type, mirror.numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	popMatrix(MODEL);
}

static void draw_rearview(void)
{
	pushMatrix(MODEL);
	translate(MODEL, 0.2, 0.0, 0);
	scale(MODEL, 0.5, 0.5, 0.5);
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	glBindVertexArray(rearViewCam.vao);
	glDrawElements(rearViewCam.type, rearViewCam.numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	popMatrix(MODEL);
}

void draw_objects() {
	//desenha efetivamente os objetos
	GLint loc;
	// Ground, Rocks --------------------------------------
	myObjects.clear();
	if (!mirror_mode) {
		myObjects.push_back(landingSiteRover.ground);
	}
	myObjects.push_back(landingSiteSpaceship.ground);
	myObjects.push_back(skyboxCube);

	for (int j = 0; j < staticRocks.size(); j++)
		myObjects.push_back(staticRocks[j].object);
	for (int j = 0; j < rollingRocks.size(); j++)
		myObjects.push_back(rollingRocks[j].object);
	for (int j = 0; j < items.size(); j++)
		myObjects.push_back(items[j].object);


	for (int i = 0; i < myObjects.size(); i++) {

		vector<MyMesh> meshes = myObjects[i].meshes;

		pushMatrix(MODEL);

		multMatrix(MODEL, myObjects[i].objectTransform);

		for (int objId = 0; objId < meshes.size(); objId++) {

			if (i == 0 && objId == 0 && multitexture_mode) {
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, TextureArray[0]);
				glUniform1i(texMap0, 4);
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, TextureArray[1]);
				glUniform1i(texMap1, 5);
				glUniform1i(texMode, 2);
			}
			else if (i > 2 && i <= staticRocks.size() + 1) { // static rocks
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, TextureArray[6]);
				glUniform1i(texMap0, 4);
				if (bumpMapping) {
					loc = glGetUniformLocation(shader.getProgramIndex(), "bumpmap");
					glUniform1i(loc, 1);
					glActiveTexture(GL_TEXTURE5);
					glBindTexture(GL_TEXTURE_2D, TextureArray[7]);
					glUniform1i(texMap1, 5);
				}
				glUniform1i(texMode, 1);
			}
			else {
				glUniform1i(texMode, 0);
				
				if (i == 2) { // cubemap
					glUniform1i(texMode, 0);
					loc = glGetUniformLocation(shader.getProgramIndex(), "cubeMapping");
					glUniform1i(loc, 1);
					glUniform1i(texMode_uniformId, 0);
					glActiveTexture(GL_TEXTURE4);
					glBindTexture(GL_TEXTURE_CUBE_MAP, TextureArray[11]);

					glUniform1i(tex_cube_loc, 4); //  Environmental cube mapping
					
					if (!reflect_perFragment)
						glUniform1i(reflect_perFragment_uniformId, 0); //reflected vector calculated in the vertex shader
					else
						glUniform1i(reflect_perFragment_uniformId, 1); //reflected vector calculated in the fragment shader
				}
			}
			// send the material
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
			glUniform4fv(loc, 1, meshes[objId].mat.ambient);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
			glUniform4fv(loc, 1, meshes[objId].mat.diffuse);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
			glUniform4fv(loc, 1, meshes[objId].mat.specular);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
			glUniform1f(loc, meshes[objId].mat.shininess);
			pushMatrix(MODEL);

			multMatrix(MODEL, meshes[objId].meshTransform);


			// send matrices to OGL
			glUniformMatrix4fv(view_uniformId, 1, GL_FALSE, mMatrix[VIEW]);
			computeDerivedMatrix(PROJ_VIEW_MODEL);
			glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(model_uniformId, 1, GL_FALSE, mMatrix[MODEL]); //Transformação de modelação do cubo unitário para o "Big Cube"
			glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

			// Render mesh

			glBindVertexArray(meshes[objId].vao);
			glDrawElements(meshes[objId].type, meshes[objId].numIndexes, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			loc = glGetUniformLocation(shader.getProgramIndex(), "bumpmap");
			glUniform1i(loc, 0);
			loc = glGetUniformLocation(shader.getProgramIndex(), "cubemapactive");
			glUniform1i(loc, 0);

			popMatrix(MODEL);
		}
		popMatrix(MODEL);
	}

	// Rover ---------------------------------------------------
	glUniform1i(texMode, 0);

	loc = glGetUniformLocation(shader.getProgramIndex(), "rover");
	glUniform1i(loc, 1);

	vector<MyMesh> meshes = rover.object.meshes;

	pushMatrix(MODEL);

	multMatrix(MODEL, rover.object.objectTransform);

	translate(MODEL, rover.position[0], 0, rover.position[2]);
	rotate(MODEL, rover.angle, 0, 1, 0);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, TextureArray[2]); // rover.tga 
	glUniform1i(texMap0, 4);

	for (int objId = 0; objId < meshes.size(); objId++) {

		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, meshes[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, meshes[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, meshes[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, meshes[objId].mat.shininess);
		pushMatrix(MODEL);

		multMatrix(MODEL, meshes[objId].meshTransform);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		if (objId == 0 || objId == 1)
			glUniform1i(texMode, 1); // textura para o corpo do rover
		else
			glUniform1i(texMode, 0);

		glBindVertexArray(meshes[objId].vao);
		glDrawElements(meshes[objId].type, meshes[objId].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
	}
	popMatrix(MODEL);

	loc = glGetUniformLocation(shader.getProgramIndex(), "rover");
	glUniform1i(loc, 0);


	// Particles --------------------------------------------
	loc = glGetUniformLocation(shader.getProgramIndex(), "particles");
	glUniform1i(loc, 1);

	float particle_color[4];

	updateParticles();

	// draw fireworks particles
	//objId = 6;  //quad for particle

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, TextureArray[3]);  //particle.tga 

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthMask(GL_FALSE);  //Depth Buffer Read Only

	glUniform1i(texMode, 1);

	for (int i = 0; i < particles.size(); i++)
	{
		if (particles[i].life > 0.0f) /* só desenha as que ainda estão vivas */
		{

			particle_color[0] = particles[i].r;
			particle_color[1] = particles[i].g;
			particle_color[2] = particles[i].b;
			particle_color[3] = particles[i].life;

			// send the material - diffuse color modulated with texture
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
			glUniform4fv(loc, 1, particle_color);

			pushMatrix(MODEL);
			translate(MODEL, particles[i].x, particles[i].y, particles[i].z);
			rotate(MODEL, 90, 0, 1, 0);

			// send matrices to OGL
			computeDerivedMatrix(PROJ_VIEW_MODEL);
			glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

			glBindVertexArray(particleMesh.vao);
			glDrawElements(particleMesh.type, particleMesh.numIndexes, GL_UNSIGNED_INT, 0);
			popMatrix(MODEL);
		}
		else dead_num_particles++;
	}
	glDepthMask(GL_TRUE); //make depth buffer again writeable

	loc = glGetUniformLocation(shader.getProgramIndex(), "particles");
	glUniform1i(loc, 0);

	// Aliens

	loc = glGetUniformLocation(shader.getProgramIndex(), "billboard");
	glUniform1i(loc, 1);

	for (int i = 0; i < aliens.size(); i++) {

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, TextureArray[8 + aliens[i].type]);
		glUniform1i(texMode, 1);

		meshes = aliens[i].object.meshes;
		pushMatrix(MODEL);
		multMatrix(MODEL, aliens[i].object.objectTransform);
		translate(MODEL, aliens[i].position[0], aliens[i].position[1] + 1, aliens[i].position[2]);

		for (int objId = 0; objId < meshes.size(); objId++) {

			// send the material
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
			glUniform4fv(loc, 1, meshes[objId].mat.ambient);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
			glUniform4fv(loc, 1, meshes[objId].mat.diffuse);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
			glUniform4fv(loc, 1, meshes[objId].mat.specular);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
			glUniform1f(loc, meshes[objId].mat.shininess);

			pushMatrix(MODEL);

			// send matrices to OGL
			computeDerivedMatrix(PROJ_VIEW_MODEL);

			int i, j;

			for (i = 0; i < 3; i += 2)
				for (j = 0; j < 3; j++) {
					if (i == j)
						mCompMatrix[VIEW_MODEL][i * 4 + j] = 1.0;
					else
						mCompMatrix[VIEW_MODEL][i * 4 + j] = 0.0;
				}
			computeDerivedMatrix_PVM();

			glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

			// Render mesh
			glBindVertexArray(meshes[objId].vao);
			glDrawElements(meshes[objId].type, meshes[objId].numIndexes, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			popMatrix(MODEL);
		}
		popMatrix(MODEL);
	}


	loc = glGetUniformLocation(shader.getProgramIndex(), "billboard");
	glUniform1i(loc, 0);

	// Flags! ----------------------------------------------------------

	loc = glGetUniformLocation(shader.getProgramIndex(), "billboard");
	glUniform1i(loc, 1);

	for (int i = 0; i < flags.size(); i++) {


		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, TextureArray[4 + i]);

		glUniform1i(texMode, 1);
		meshes = flags[i].object.meshes;
		pushMatrix(MODEL);
		multMatrix(MODEL, flags[i].object.objectTransform);
		translate(MODEL, flags[i].position[0], flags[i].position[1] + 1, flags[i].position[2]);

		for (int objId = 0; objId < meshes.size(); objId++) {

			// send the material
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
			glUniform4fv(loc, 1, meshes[objId].mat.ambient);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
			glUniform4fv(loc, 1, meshes[objId].mat.diffuse);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
			glUniform4fv(loc, 1, meshes[objId].mat.specular);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
			glUniform1f(loc, meshes[objId].mat.shininess);

			pushMatrix(MODEL);

			// send matrices to OGL
			computeDerivedMatrix(PROJ_VIEW_MODEL);

			int i, j;

			for (i = 0; i < 3; i += 2)
				for (j = 0; j < 3; j++) {
					if (i == j)
						mCompMatrix[VIEW_MODEL][i * 4 + j] = 1.0;
					else
						mCompMatrix[VIEW_MODEL][i * 4 + j] = 0.0;
				}
			computeDerivedMatrix_PVM();

			glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

			// Render mesh
			glBindVertexArray(meshes[objId].vao);
			glDrawElements(meshes[objId].type, meshes[objId].numIndexes, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			popMatrix(MODEL);
		}
		popMatrix(MODEL);
	}


	loc = glGetUniformLocation(shader.getProgramIndex(), "billboard");
	glUniform1i(loc, 0);


	// Spaceship
	glUniform1i(texMap0, 0);
	glUniform1i(texMap1, 0);
	glBindTextureUnit(5, 0);
	glBindTextureUnit(4, 0);
	glUniform1i(texMode, 1);

	aiRecursive_render(scene, scene->mRootNode);

	// Pillars -------------------------------------------------------------------
	glUniform1i(texMode, 0);
	for (int i = 0; i < pillars.size(); i++) {

		MyMesh mesh = pillars[i].object.meshes[0];

		pushMatrix(MODEL);

		multMatrix(MODEL, pillars[i].object.objectTransform);

		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, mesh.mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, mesh.mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, mesh.mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, mesh.mat.shininess);
		pushMatrix(MODEL);

		multMatrix(MODEL, mesh.meshTransform);

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh

		glBindVertexArray(mesh.vao);
		glDrawElements(mesh.type, mesh.numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);

		popMatrix(MODEL);
	}

	// Flare Effect ---------------------------------------------------
	if (flareEffect) {
		glEnable(GL_BLEND);
		loc = glGetUniformLocation(shader.getProgramIndex(), "flare");
		glUniform1i(loc, 1);
		int flarePos[2];
		int m_viewport[4];
		glGetIntegerv(GL_VIEWPORT, m_viewport);

		pushMatrix(MODEL);
		loadIdentity(MODEL);
		computeDerivedMatrix(PROJ_VIEW_MODEL);  //pvm to be applied to lightPost. pvm is used in project function

		if (!project(directionalLightPos, lightScreenPos, m_viewport))
			printf("Error in getting projected light in screen\n");  //Calculate the window Coordinates of the light position: the projected position of light on viewport
		flarePos[0] = clampi((int)lightScreenPos[0], m_viewport[0], m_viewport[0] + m_viewport[2] - 1);
		flarePos[1] = clampi((int)lightScreenPos[1], m_viewport[1], m_viewport[1] + m_viewport[3] - 1);
		popMatrix(MODEL);

		//viewer looking down at  negative z direction
		pushMatrix(PROJECTION);
		loadIdentity(PROJECTION);
		pushMatrix(VIEW);
		loadIdentity(VIEW);
		ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);
		renderFlare(&flare, flarePos[0], flarePos[1], m_viewport);
		popMatrix(PROJECTION);
		popMatrix(VIEW);

		loc = glGetUniformLocation(shader.getProgramIndex(), "flare");
		glUniform1i(loc, 0);
	}

	glBindTextureUnit(0, 0);
}

void renderScene(void) {
			
		//desenha efetivamente os objetos
		GLint loc;

		FrameCount++;
		//glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Transparency
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		// load identity matrices
		loadIdentity(VIEW);
		loadIdentity(MODEL);
		/*
		// Render Skybox 
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_CUBE_MAP, TextureArray[11]);
		glUniform1i(tex_cube_loc, 4);

		loc = glGetUniformLocation(shader.getProgramIndex(), "skybox");
		glUniform1i(loc, 1);

		//it won't write anything to the zbuffer; all subsequently drawn scenery to be in front of the sky box. 
		glDepthMask(GL_FALSE); 
		glFrontFace(GL_CW); // set clockwise vertex order to mean the front
	
		pushMatrix(MODEL);
		pushMatrix(VIEW);  //se quiser anular a translação

		//  Fica mais realista se não anular a translação da câmara 
		// Cancel the translation movement of the camera - de acordo com o tutorial do Antons
		mMatrix[VIEW][12] = 0.0f;
		mMatrix[VIEW][13] = 0.0f;
		mMatrix[VIEW][14] = 0.0f;
	
		scale(MODEL, 100.0f, 100.0f, 100.0f);
		translate(MODEL, -0.5f, -0.5f, -0.5f);

		// send matrices to OGL
		glUniformMatrix4fv(model_uniformId, 1, GL_FALSE, mMatrix[MODEL]); //Transformação de modelação do cubo unitário para o "Big Cube"
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		
		/*
		glUniform1i(tex_cube_loc, 11);
		loc = glGetUniformLocation(shader.getProgramIndex(), "skybox");
		glUniform1i(loc, 1);

		glBindVertexArray(skyboxCube.meshes[0].vao);
		glDrawElements(skyboxCube.meshes[0].type, skyboxCube.meshes[0].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		popMatrix(MODEL);
		popMatrix(VIEW);
	 */
		glFrontFace(GL_CCW); // restore counter clockwise vertex order to mean the front
		glDepthMask(GL_TRUE);

		glUniform1i(texMap0, 4);
		glUniform1i(texMap1, 5);

		if (!rear_view_cam_mode) {
			// ----------------------------------------------
		// set the camera using a function similar to gluLookAt
			cameras[currentCamera].setProjection((float)WinX, (float)WinY);
			cameras[currentCamera].cameraLookAt();

			// use our shader

			glUseProgram(shader.getProgramIndex());

			loc = glGetUniformLocation(shader.getProgramIndex(), "sun_mode");
			if (sun_mode)
				glUniform1i(loc, 1);
			else
				glUniform1i(loc, 0);


			loc = glGetUniformLocation(shader.getProgramIndex(), "point_lights_mode");
			if (point_lights_mode)
				glUniform1i(loc, 1);
			else
				glUniform1i(loc, 0);


			loc = glGetUniformLocation(shader.getProgramIndex(), "spotlight_mode");
			if (spotlight_mode)
				glUniform1i(loc, 1);
			else
				glUniform1i(loc, 0);

			loc = glGetUniformLocation(shader.getProgramIndex(), "fog_mode");
			if (fog_mode)
				glUniform1i(loc, 1);
			else
				glUniform1i(loc, 0);

			float res[4];
			multMatrixPoint(VIEW, coneDir, res);
			loc = glGetUniformLocation(shader.getProgramIndex(), "coneDir");
			glUniform4fv(loc, 1, res);
			loc = glGetUniformLocation(shader.getProgramIndex(), "spotCosCutOff");
			glUniform1f(loc, 0.99f);

			if (!mirror_mode) {
				//lightPos definido em World Coord so is converted to eye space

				for (int i = 0; i < NUMBER_POINT_LIGHTS; i++) {
					multMatrixPoint(VIEW, pointLightPos[i], res);
					loc = glGetUniformLocation(shader.getProgramIndex(),
						(const GLchar*)("pointLights[" + to_string(i) + "].position").c_str());
					glUniform4fv(loc, 1, res);
				}

				multMatrixPoint(VIEW, directionalLightPos, res);
				loc = glGetUniformLocation(shader.getProgramIndex(),
					"dirLight.position");
				glUniform4fv(loc, 1, res);

				for (int i = 0; i < NUMBER_SPOT_LIGHTS; i++) {
					multMatrixPoint(VIEW, spotlightPos[i], res);
					loc = glGetUniformLocation(shader.getProgramIndex(),
						(const GLchar*)("spotLights[" + to_string(i) + "].position").c_str());
					glUniform4fv(loc, 1, res);
				}
			}

		}
		
		if (rear_view_cam_mode) {
			glEnable(GL_STENCIL_TEST);
			loadIdentity(PROJECTION);

			// use our shader
			glUseProgram(shader.getProgramIndex());
			glStencilFunc(GL_NEVER, 0x1, 0x1);

			glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

			// Fill stencil buffer with obj
			draw_rearview();


			for (int i = 0; i < 2; i++) {
				// ----------------------------------------------
				// set the camera using a function similar to gluLookAt
				if (i == 1) {
					glStencilFunc(GL_EQUAL, 0x1, 0x1);
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
					cameras[3].setProjection((float)WinX, (float)WinY);
					cameras[3].cameraLookAt();
				}
				else
				{
					glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
					cameras[currentCamera].setProjection((float)WinX, (float)WinY);
					cameras[currentCamera].cameraLookAt();
				}

				loc = glGetUniformLocation(shader.getProgramIndex(), "sun_mode");
				if (sun_mode)
					glUniform1i(loc, 1);
				else
					glUniform1i(loc, 0);


				loc = glGetUniformLocation(shader.getProgramIndex(), "point_lights_mode");
				if (point_lights_mode)
					glUniform1i(loc, 1);
				else
					glUniform1i(loc, 0);


				loc = glGetUniformLocation(shader.getProgramIndex(), "spotlight_mode");
				if (spotlight_mode)
					glUniform1i(loc, 1);
				else
					glUniform1i(loc, 0);

				loc = glGetUniformLocation(shader.getProgramIndex(), "fog_mode");
				if (fog_mode)
					glUniform1i(loc, 1);
				else
					glUniform1i(loc, 0);

				float res[4];
				multMatrixPoint(VIEW, coneDir, res);
				loc = glGetUniformLocation(shader.getProgramIndex(), "coneDir");
				glUniform4fv(loc, 1, res);
				loc = glGetUniformLocation(shader.getProgramIndex(), "spotCosCutOff");
				glUniform1f(loc, 0.99f);


				for (int i = 0; i < NUMBER_POINT_LIGHTS; i++) {
					multMatrixPoint(VIEW, pointLightPos[i], res);
					loc = glGetUniformLocation(shader.getProgramIndex(),
						(const GLchar*)("pointLights[" + to_string(i) + "].position").c_str());
					glUniform4fv(loc, 1, res);
				}

				multMatrixPoint(VIEW, directionalLightPos, res);
				loc = glGetUniformLocation(shader.getProgramIndex(),
					"dirLight.position");
				glUniform4fv(loc, 1, res);

				for (int i = 0; i < NUMBER_SPOT_LIGHTS; i++) {
					multMatrixPoint(VIEW, spotlightPos[i], res);
					loc = glGetUniformLocation(shader.getProgramIndex(),
						(const GLchar*)("spotLights[" + to_string(i) + "].position").c_str());
					glUniform4fv(loc, 1, res);
				}
				draw_objects();
			}

			glDisable(GL_STENCIL_TEST);

		}

		// ----------------------------------------------
		// reflection and shadowing
		else if (mirror_mode) {
			glEnable(GL_STENCIL_TEST);
			glClearStencil(0);
			glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);


			glEnable(GL_DEPTH_TEST);
			float res[4];
			float mat[16];
			GLfloat floor[4] = { 0,1,0,0 };

			if (cameras[currentCamera].position[1] > 0.0f) {

				glEnable(GL_STENCIL_TEST);
				glStencilFunc(GL_NEVER, 0x1, 0x1);
				glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

				// Fill stencil buffer our ground obj
				draw_mirror();

				glUniform1i(shadowMode, 0);

				glStencilFunc(GL_EQUAL, 0x1, 0x1);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

				// object reflection
				for (int i = 0; i < NUMBER_SPOT_LIGHTS; i++) {
					spotlightPos[i][1] *= (-1.0f);
					multMatrixPoint(VIEW, spotlightPos[i], res);
					loc = glGetUniformLocation(shader.getProgramIndex(),
						(const GLchar*)("spotLights[" + to_string(i) + "].position").c_str());
					glUniform4fv(loc, 1, res);

					pushMatrix(MODEL);
					scale(MODEL, 1.0f, -1.0f, 1.0f);
					glCullFace(GL_FRONT);
					draw_objects();
					glCullFace(GL_BACK);
					popMatrix(MODEL);

					spotlightPos[i][1] *= (-1.0f);
					multMatrixPoint(VIEW, spotlightPos[i], res);
					glUniform4fv(loc, 1, res);
				}

				for (int i = 0; i < NUMBER_POINT_LIGHTS; i++) {
					pointLightPos[i][1] *= (-1.0f);
					multMatrixPoint(VIEW, pointLightPos[i], res);
					loc = glGetUniformLocation(shader.getProgramIndex(),
						(const GLchar*)("pointLights[" + to_string(i) + "].position").c_str());
					glUniform4fv(loc, 1, res);

					pushMatrix(MODEL);
					scale(MODEL, 1.0f, -1.0f, 1.0f);
					glCullFace(GL_FRONT);
					draw_objects();
					glCullFace(GL_BACK);
					popMatrix(MODEL);

					pointLightPos[i][1] *= (-1.0f);
					multMatrixPoint(VIEW, pointLightPos[i], res);
					glUniform4fv(loc, 1, res);
				}

				directionalLightPos[1] *= (-1.0f);

				multMatrixPoint(VIEW, directionalLightPos, res);
				loc = glGetUniformLocation(shader.getProgramIndex(),
					"dirLight.position");
				glUniform4fv(loc, 1, res);

				pushMatrix(MODEL);
				scale(MODEL, 1.0f, -1.0f, 1.0f);
				glCullFace(GL_FRONT);
				draw_objects();
				glCullFace(GL_BACK);
				popMatrix(MODEL);

				directionalLightPos[1] *= (-1.0f);
				multMatrixPoint(VIEW, directionalLightPos, res);
				glUniform4fv(loc, 1, res);

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				draw_mirror();

				// Render the Shadows
				glUniform1i(shadowMode, 1);
				shadow_matrix(mat, floor, directionalLightPos);

				glDisable(GL_DEPTH_TEST); //force the shadows to be rendered even if behind floor

				//darken color stored in color buf
				glBlendFunc(GL_DST_COLOR, GL_ZERO);
				glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

				pushMatrix(MODEL);
				multMatrix(MODEL, mat);
				draw_objects();
				popMatrix(MODEL);

				glDisable(GL_STENCIL_TEST);
				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);

				//render the geometry
				glUniform1i(shadowMode, 0);
				draw_objects();
			}
			else { 
				//Camera behind floor hence render only opaque objects
				glUniform1i(shadowMode, 0);
				draw_mirror();
				draw_objects();
			}
		}
		else {

			draw_objects();
		}

		// Text -------------------------------------------------

		//Render text (bitmap fonts) in screen coordinates. So use ortoghonal projection with viewport coordinates.
		glDisable(GL_DEPTH_TEST);
		//the glyph contains transparent background colors and non-transparent for the actual character pixels. So we use the blending
		glEnable(GL_BLEND);
		int m_viewport[4];
		glGetIntegerv(GL_VIEWPORT, m_viewport);

		//viewer at origin looking down at  negative z direction
		pushMatrix(MODEL);
		loadIdentity(MODEL);
		pushMatrix(PROJECTION);
		loadIdentity(PROJECTION);
		pushMatrix(VIEW);
		loadIdentity(VIEW);
		ortho(float(m_viewport[0]), float(m_viewport[0] + m_viewport[2] - 1), float(m_viewport[1]), float(m_viewport[1] + m_viewport[3] - 1), -1.0f, 1.0f);

		glUniform1i(texText, 4);
		RenderText(shaderText, (const GLchar*)("Lives: " + to_string(livesCount)).c_str(), float(WinX - 190), float(WinY - 48), 1.0f, 0.8f, 0.8f, 0.8f);

		string num = to_string(points);
		string str = "00000";
		str.replace(str.size() - num.size(), 5, num);
		RenderText(shaderText, str, 10, float(WinY - 48), 1.0f, 0.8f, 0.8f, 0.8f);

		if (pauseActive)
			RenderText(shaderText, "PAUSE", float(WinX / 2 - 80), float(WinY / 2 + 220), 1.0f, 0.8f, 0.8f, 0.8f);
		if (gameOver)
			RenderText(shaderText, "GAME OVER", float(WinX / 2 - 150), float(WinY / 2 + 220), 1.0f, 0.8f, 0.8f, 0.8f);


		glBindTextureUnit(0, 0);
		popMatrix(PROJECTION);
		popMatrix(VIEW);
		popMatrix(MODEL);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glutSwapBuffers();
	
	
}

// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h) {

	float ratio;
	// Prevent a divide by zero, when window is too short
	if (h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// set the projection matrix
	ratio = (1.0f * w) / h;
	loadIdentity(PROJECTION);
	perspective(53.13f, ratio, 0.1f, 1000.0f);
}


// ------------------------------------------------------------
//
// Events from the Keyboard
//
void processKeys(unsigned char key, int xx, int yy)
{
	std::tuple<float, float> aux;
	switch (key) {
	case 27:
		glutLeaveMainLoop();
		break;
	case 'q':
	case'Q':
		if (pauseActive || gameOver)
			return;

		if (isRoverHittingSomething && isGoingForward)
			return;

		initParticles();
		rover.speed += 0.8f;
		beta = float(atan(cameras[2].position[1] / cameras[2].position[0]) * 180.0 / M_PI);
		isGoingForward = true;
		break;
	case 'a':
	case'A':
		if (pauseActive || gameOver)
			return;

		if (isRoverHittingSomething && !isGoingForward)
			return;

		initParticles();
		rover.speed -= 0.8f;
		beta = float(atan(cameras[2].position[1] / cameras[2].position[0]) * 180.0 / M_PI);
		isGoingForward = false;
		break;
	case 'o':
	case'O':
		if (pauseActive || gameOver)
			return;

		rover.angle += 1;
		alpha += 1;
		break;
	case 'p':
	case'P':
		if (pauseActive || gameOver)
			return;

		rover.angle -= 1;
		alpha -= 1;
		break;
	case '1':
		if (pauseActive || gameOver)
			return;

		currentCamera = 0;
		break;
	case '2':
		if (pauseActive || gameOver)
			return;

		currentCamera = 1;
		break;
	case '3':
		if (pauseActive || gameOver)
			return;

		currentCamera = 2;
		break;
	case 'c':
	case 'C':
		if (pauseActive || gameOver)
			return;

		point_lights_mode = !point_lights_mode;
		break;
	case 'h':
	case 'H':
		if (pauseActive || gameOver)
			return;

		spotlight_mode = !spotlight_mode;
		break;
	case 'n':
	case 'N':
		if (pauseActive || gameOver)
			return;

		sun_mode = !sun_mode;
		break;
	case 'f':
	case 'F':
		if (pauseActive || gameOver)
			return;

		fog_mode = !fog_mode;
		break;
	case 't':
	case 'T':
		if (pauseActive || gameOver)
			return;

		multitexture_mode = !multitexture_mode;
		break;
	case 's':
	case 'S':
		if (gameOver)
			return;
		pauseActive = !pauseActive;
		break;
	case 'r':
	case'R':
		gameOver = false;
		pauseActive = false;
		initialState(true);
		rollingRocks.clear();
		createRollingRocks(7);
		items.clear();
		createItems(10);
		break;
	case 'b':
		if (pauseActive || gameOver)
			return;
		bumpMapping = !bumpMapping;
		break;
	case 'l':
	case 'L':
		if (pauseActive || gameOver)
			return;
		flareEffect = !flareEffect;
		break;
	case 'm':
	case 'M':
		if (pauseActive || gameOver)
			return;
		mirror_mode = !mirror_mode;
		break;
	case 'x':
	case 'X':
		if (reflect_perFragment == 0) {
			reflect_perFragment = 1;
			printf("Reflection vector calculated in the fragment shader\n");
		}
		else {
			reflect_perFragment = 0;
			printf("Reflection vector calculated in the vertex shader\n");
		}
		break;
	case 'y':
	case 'Y':
		if (pauseActive || gameOver)
			return;
		if (mirror_mode) {
			return;
		}
		rear_view_cam_mode = !rear_view_cam_mode;
		break;
	}
}


// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN) {
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
	}

	//stop tracking the mouse
	else if (state == GLUT_UP) {
		tracking = 0;
	}
}

// Track mouse motion while buttons are pressed

void processMouseMotion(int xx, int yy)
{

	int deltaX, deltaY;
	float alphaAux = alpha, betaAux = beta;

	deltaX = -xx + startX;
	deltaY = yy - startY;

	// left mouse button: move camera
	if (currentCamera == 2 && tracking == 1) {
		alphaAux = alpha + deltaX - 90;
		betaAux = beta + deltaY;

		if (betaAux > 85.0f)
			betaAux = 85.0f;
		else if (betaAux < -85.0f)
			betaAux = -85.0f;

		cameras[2].position[0] = rover.position[0] + cos((M_PI / 180) * alphaAux) * 10;
		cameras[2].position[1] = rover.position[1] + cos((M_PI / 180) * betaAux) * 10;
		cameras[2].position[2] = rover.position[2] - sin((M_PI / 180) * alphaAux) * 10;

		std::copy(rover.position, rover.position + 3, cameras[2].target);
	}

	glutPostRedisplay();
}



// --------------------------------------------------------
//
// Shader Stuff
//


GLuint setupShaders() {

	// Shader for models
	shader.init();
	shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/pointlight.vert");
	shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/pointlight.frag");

	// set semantics for the shader variables
	glBindFragDataLocation(shader.getProgramIndex(), 0, "colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");
	glBindAttribLocation(shader.getProgramIndex(), TANGENT_ATTRIB, "tangent");
	glBindAttribLocation(shader.getProgramIndex(), BITANGENT_ATTRIB, "bitangent");

	glLinkProgram(shader.getProgramIndex());
	printf("InfoLog for Model Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shader.isProgramValid()) {
		printf("GLSL Model Program Not Valid!\n");
		exit(1);
	}

	texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texModeVert"); // different modes of texturing
	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_pos");
	normalMap_loc = glGetUniformLocation(shader.getProgramIndex(), "normalMap");
	specularMap_loc = glGetUniformLocation(shader.getProgramIndex(), "specularMap");
	diffMapCount_loc = glGetUniformLocation(shader.getProgramIndex(), "diffMapCount");
	texMap0 = glGetUniformLocation(shader.getProgramIndex(), "texmap0");
	texMap1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	texMode = glGetUniformLocation(shader.getProgramIndex(), "texMode"); // multitex, one tex or no tex
	tex_cube_loc = glGetUniformLocation(shader.getProgramIndex(), "cubeMap");
	reflect_perFragment_uniformId = glGetUniformLocation(shader.getProgramIndex(), "reflect_perFrag"); //reflection vector calculated in the frag shader
	model_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_Model");
	view_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_View");
	shadowMode = glGetUniformLocation(shader.getProgramIndex(), "shadowMode");


	printf("InfoLog for Per Fragment Phong Lightning Shader\n%s\n\n", shader.getAllInfoLogs().c_str());

	// Shader for bitmap Text
	shaderText.init();
	shaderText.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/text.vert");
	shaderText.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/text.frag");
	texText = glGetUniformLocation(shaderText.getProgramIndex(), "text");

	glLinkProgram(shaderText.getProgramIndex());
	printf("InfoLog for Text Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shaderText.isProgramValid()) {
		printf("GLSL Text Program Not Valid!\n");
		exit(1);
	}

	return(shader.isProgramLinked() && shaderText.isProgramLinked());
}

void createGround() {
	//surface
	MyMesh amesh = createQuad(1000.0f, 1000.0f);
	setIdentityMatrix(landingSiteRover.ground.objectTransform, 4);
	setMeshColor(&amesh, 0.8f, 0.4f, 0.0f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);

	myRotate(amesh.meshTransform, -90.0f, 1.0f, 0.0f, 0.0f);

	landingSiteRover.ground.meshes.push_back(amesh);

	// landing site spaceship
	float side = 28.0f;
	landingSiteSpaceship.side = side;
	landingSiteSpaceship.position[0] = 0.0f;
	landingSiteSpaceship.position[1] = 0.15f;
	landingSiteSpaceship.position[2] = -30.0f;

	amesh = createQuad(side, side);
	setIdentityMatrix(landingSiteSpaceship.ground.objectTransform, 4);
	setMeshColor(&amesh, 0.8f, 0.8f, 0.8f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);

	myTranslate(amesh.meshTransform, landingSiteSpaceship.position[0], landingSiteSpaceship.position[1], landingSiteSpaceship.position[2]);
	myRotate(amesh.meshTransform, -90.0f, 1.0f, 0.0f, 0.0f);
	landingSiteSpaceship.ground.meshes.push_back(amesh);

	// path
	side = 3.0f;
	amesh = createQuad(side, side * 4);
	setIdentityMatrix(landingSiteSpaceship.ground.objectTransform, 4);
	setMeshColor(&amesh, 0.9f, 0.8f, 0.8f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, 0.0f, 0.15f, -10.0f);
	myRotate(amesh.meshTransform, -90.0f, 1.0f, 0.0f, 0.0f);
	landingSiteSpaceship.ground.meshes.push_back(amesh);

	// landing site rover
	side = 8.0f;
	landingSiteRover.side = side;
	landingSiteRover.position[0] = 0.0f;
	landingSiteRover.position[1] = 0.15f;
	landingSiteRover.position[2] = 0.0f;

	amesh = createQuad(side, side);
	setIdentityMatrix(landingSiteRover.ground.objectTransform, 4);
	setMeshColor(&amesh, 0.8f, 0.8f, 0.8f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);

	myTranslate(amesh.meshTransform, landingSiteRover.position[0], landingSiteRover.position[1], landingSiteRover.position[2]);
	myRotate(amesh.meshTransform, -90.0f, 1.0f, 0.0f, 0.0f);
	landingSiteRover.ground.meshes.push_back(amesh);

	// aliens base

	amesh = createCone(20.0f, 3.0f, 10.0f);
	setIdentityMatrix(landingSiteRover.ground.objectTransform, 4);
	setMeshColor(&amesh, 0.2f, 0.2f, 0.2f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, 0.0f, 0.0f, 78.0f);

	landingSiteRover.ground.meshes.push_back(amesh);

	amesh = createCone(15.0f, 3.0f, 10.0f);
	setIdentityMatrix(landingSiteRover.ground.objectTransform, 4);
	setMeshColor(&amesh, 0.2f, 0.2f, 0.2f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, 4.0f, 0.0f, 80.0f);

	landingSiteRover.ground.meshes.push_back(amesh);

	amesh = createCone(18.0f, 3.0f, 10.0f);
	setIdentityMatrix(landingSiteRover.ground.objectTransform, 4);
	setMeshColor(&amesh, 0.2f, 0.2f, 0.2f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, -3.0f, 0.0f, 82.0f);

	landingSiteRover.ground.meshes.push_back(amesh);

	aliensBase.position[0] = 0.0f;
	aliensBase.position[1] = 0.0f;
	aliensBase.position[2] = 80.0f;
	aliensBase.side = 14;
}

void createPillars() {
	Pillar pillar1, pillar2, pillar3, pillar4;
	float side = 8.0f;

	setIdentityMatrix(pillar1.object.objectTransform, 4);
	setIdentityMatrix(pillar2.object.objectTransform, 4);
	setIdentityMatrix(pillar3.object.objectTransform, 4);
	setIdentityMatrix(pillar4.object.objectTransform, 4);


	// pillar
	pillar1.radius = 0.2f;
	pillar1.position[0] = side / 2;
	pillar1.position[1] = 0;
	pillar1.position[2] = side / 2;

	MyMesh amesh = createCylinder(7.0f, pillar1.radius, 10);
	setMeshColor(&amesh, 0.82f, 0.17f, 0.03f, 0.5f);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, pillar1.position[0], pillar1.position[1], pillar1.position[2]);
	pillar1.object.meshes.push_back(amesh);

	pillar2.radius = 0.2f;
	pillar2.position[0] = side / 2;
	pillar2.position[1] = 0;
	pillar2.position[2] = -side / 2;

	amesh = createCylinder(7.0f, pillar2.radius, 10);
	setMeshColor(&amesh, 0.82f, 0.17f, 0.03f, 0.5f);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, pillar2.position[0], pillar2.position[1], pillar2.position[2]);
	pillar2.object.meshes.push_back(amesh);


	pillar3.radius = 0.2f;
	pillar3.position[0] = -side / 2;
	pillar3.position[1] = 0;
	pillar3.position[2] = side / 2;

	amesh = createCylinder(7.0f, pillar3.radius, 10);
	setMeshColor(&amesh, 0.82f, 0.17f, 0.03f, 0.5f);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, pillar3.position[0], pillar3.position[1], pillar3.position[2]);
	pillar3.object.meshes.push_back(amesh);

	pillar4.radius = 0.2f;
	pillar4.position[0] = -side / 2;
	pillar4.position[1] = 0;
	pillar4.position[2] = -side / 2;

	amesh = createCylinder(7.0f, pillar4.radius, 10);
	setMeshColor(&amesh, 0.82f, 0.17f, 0.03f, 0.5f);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, pillar4.position[0], pillar4.position[1], pillar4.position[2]);
	pillar4.object.meshes.push_back(amesh);

	pillars.push_back(pillar1);
	pillars.push_back(pillar2);
	pillars.push_back(pillar3);
	pillars.push_back(pillar4);
}

void createRover() {
	MyObject roverObj;
	setIdentityMatrix(roverObj.objectTransform, 4);

	MyMesh body = createCube();
	setMeshColor(&body, 0.4f, 0.4f, 0.4f, 1.0f);
	setIdentityMatrix(body.meshTransform, 4);
	myTranslate(body.meshTransform, -0.5f, -0.5f, -0.5f); // move o cubo pro centro
	myScale(body.meshTransform, 3.0f, 1.0f, 1.5f); // ajusta as dimensoes
	myTranslate(body.meshTransform, -0.75f / 2, 0.5f, -0.75f / 4); // coloca o corpo no centro, tocando no chao
	myTranslate(body.meshTransform, 0.0f, 0.25f, 0.0f); // tira o corpo do chao

	MyMesh wheel1 = createTorus(0.5f, 0.75f, 80, 80);
	setMeshColor(&wheel1, 0.35f, 0.18f, 0.08f, 1.0f);
	setIdentityMatrix(wheel1.meshTransform, 4);
	myTranslate(wheel1.meshTransform, 1.2f, 0.85f, 0.75f); // coloca a roda no lugar
	myRotate(wheel1.meshTransform, 90.0f, 1.0f, 0.0f, 0.0f); // coloca ela na vertical

	MyMesh wheel2 = createTorus(0.5f, 0.75f, 80, 80);
	setMeshColor(&wheel2, 0.35f, 0.18f, 0.08f, 1.0f);
	setIdentityMatrix(wheel2.meshTransform, 4);
	myTranslate(wheel2.meshTransform, 1.2f, 0.85f, -0.75f); // coloca a roda no lugar
	myRotate(wheel2.meshTransform, 90.0f, 1.0f, 0.0f, 0.0f); // coloca ela na vertical

	MyMesh wheel3 = createTorus(0.5f, 0.75f, 80, 80);
	setMeshColor(&wheel3, 0.35f, 0.18f, 0.08f, 1.0f);
	setIdentityMatrix(wheel3.meshTransform, 4);
	myTranslate(wheel3.meshTransform, -1.2f, 0.85f, 0.75f); // coloca a roda no lugar
	myRotate(wheel3.meshTransform, 90.0f, 1.0f, 0.0f, 0.0f); // coloca ela na vertical

	MyMesh wheel4 = createTorus(0.5f, 0.75f, 80, 80);
	setMeshColor(&wheel4, 0.35f, 0.18f, 0.08f, 1.0f);
	setIdentityMatrix(wheel4.meshTransform, 4);
	myTranslate(wheel4.meshTransform, -1.2f, 0.85f, -0.75f); // coloca a roda no lugar
	myRotate(wheel4.meshTransform, 90.0f, 1.0f, 0.0f, 0.0f); // coloca ela na vertical

	MyMesh head = createPawn();
	setMeshColor(&head, 0.4f, 0.4f, 0.4f, 1.0f);
	setIdentityMatrix(head.meshTransform, 4);
	myScale(head.meshTransform, 0.8f, 0.5f, 0.5f); // ajusta as dimensoes
	myTranslate(head.meshTransform, -1.0f, 0.5f, 0.0f); // coloca o corpo no centro, tocando no chao
	myTranslate(head.meshTransform, 0.0f, 1.0f, 0.0f); // tira o corpo do chao

	roverObj.meshes.push_back(head);
	roverObj.meshes.push_back(body);
	roverObj.meshes.push_back(wheel1);
	roverObj.meshes.push_back(wheel2);
	roverObj.meshes.push_back(wheel3);
	roverObj.meshes.push_back(wheel4);

	rover = Rover(roverObj);
	rover.updateDirection();
	updateSpotlight();
}

void createStaticRocks() {

	MyMesh amesh;
	StaticRock rock;
	
	rock.radius = 4.0f;
	rock.originalPos[0] = 20.0f;
	rock.originalPos[1] = 0.2f;
	rock.originalPos[2] = 10.0f;
	rock.position[0] = 20.0f;
	rock.position[1] = 0.2f;
	rock.position[2] = 10.0f;
	rock.speed = 0.0f;
	amesh = createSphere(rock.radius, 10);
	setIdentityMatrix(rock.object.objectTransform, 4);
	setMeshColor(&amesh, 0.35f, 0.20f, 0.05f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, rock.position[0], rock.position[1], rock.position[2]);
	rock.object.meshes.push_back(amesh);
	staticRocks.push_back(rock);

	rock.radius = 1.0f;
	rock.originalPos[0] = -15.0f;
	rock.originalPos[1] = 0.2f;
	rock.originalPos[2] = -15.0f;
	rock.position[0] = -15.0f;
	rock.position[1] = 0.2f;
	rock.position[2] = -15.0f;
	rock.speed = 0.0f;
	amesh = createSphere(rock.radius, 10);
	setIdentityMatrix(rock.object.objectTransform, 4);
	setMeshColor(&amesh, 0.35f, 0.20f, 0.05f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, rock.position[0], rock.position[1], rock.position[2]);
	rock.object.meshes.push_back(amesh);
	staticRocks.push_back(rock);

	rock.radius = 3.0f;
	rock.originalPos[0] = -10.0f;
	rock.originalPos[1] = 0.2f;
	rock.originalPos[2] = 15.0f;
	rock.position[0] = -10.0f;
	rock.position[1] = 0.2f;
	rock.position[2] = 15.0f;
	rock.speed = 0.0f;
	amesh = createSphere(rock.radius, 10);
	setIdentityMatrix(rock.object.objectTransform, 4);
	setMeshColor(&amesh, 0.35f, 0.20f, 0.05f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, rock.position[0], rock.position[1], rock.position[2]);
	rock.object.meshes.push_back(amesh);
	staticRocks.push_back(rock);

	rock.radius = 3.0f;
	rock.originalPos[0] = -16.0f;
	rock.originalPos[1] = 0.2f;
	rock.originalPos[2] = 3.0f;
	rock.position[0] = -16.0f;
	rock.position[1] = 0.2f;
	rock.position[2] = 3.0f;
	rock.speed = 0.0f;
	amesh = createSphere(rock.radius, 10);
	setIdentityMatrix(rock.object.objectTransform, 4);
	setMeshColor(&amesh, 0.35f, 0.20f, 0.05f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, rock.position[0], rock.position[1], rock.position[2]);
	rock.object.meshes.push_back(amesh);
	staticRocks.push_back(rock);

	rock.radius = 6.0f;
	rock.originalPos[0] = -15.0f;
	rock.originalPos[1] = 0.2f;
	rock.originalPos[2] = 50.0f;
	rock.position[0] = -15.0f;
	rock.position[1] = 0.2f;
	rock.position[2] = 50.0f;
	rock.speed = 0.0f;
	amesh = createSphere(rock.radius, 10);
	setIdentityMatrix(rock.object.objectTransform, 4);
	setMeshColor(&amesh, 0.35f, 0.20f, 0.05f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, rock.position[0], rock.position[1], rock.position[2]);
	rock.object.meshes.push_back(amesh);
	staticRocks.push_back(rock);

	rock.radius = 10.0f;
	rock.originalPos[0] = 37.0f;
	rock.originalPos[1] = 0.2f;
	rock.originalPos[2] = -50.0f;
	rock.position[0] = 37.0f;
	rock.position[1] = 0.2f;
	rock.position[2] = -50.0f;
	rock.speed = 0.0f;
	amesh = createSphere(rock.radius, 10);
	setIdentityMatrix(rock.object.objectTransform, 4);
	setMeshColor(&amesh, 0.35f, 0.20f, 0.05f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, rock.position[0], rock.position[1], rock.position[2]);
	rock.object.meshes.push_back(amesh);
	staticRocks.push_back(rock);

}

void createCubeSkybox() {
	setIdentityMatrix(skyboxCube.objectTransform, 4);
	// create geometry and VAO of the cube, objId=2;
	MyMesh amesh = createCube();
	setMeshColor(&amesh, 0.35f, 0.20f, 0.05f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, -5, 0, 8);

	skyboxCube.meshes.push_back(amesh);
}

void createSpaceship() {
	model_dir = "SciFi_Fighter_AK5";

	ostringstream oss;
	oss << model_dir << "/" << model_dir << ".obj";
	string filepath = oss.str();

	model_dir += "/";

	//check if file exists
	ifstream fin(filepath.c_str());
	if (!fin.fail()) {
		fin.close();
	}
	else
		printf("Couldn't open file: %s\n", filepath.c_str());

	//import 3D file into Assimp scene graph
	if (!Import3DFromFile(filepath))
		return;

	//creation of Mymesh array with VAO Geometry and Material
	spaceship = createMeshFromAssimp(scene);
}

void createFlags() {
	MyMesh amesh = createQuad(3.0f, 3.0f);
	Flag portugal, austria;

	setIdentityMatrix(portugal.object.objectTransform, 4);
	setMeshColor(&amesh, 0.2f, 0.2f, 0.2f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);
	portugal.position[0] = -8.0f;
	portugal.position[1] = 0.0f;
	portugal.position[2] = 3.0f;
	myTranslate(amesh.meshTransform, portugal.position[0], portugal.position[1], portugal.position[1]);

	portugal.object.meshes.push_back(amesh);

	amesh = createQuad(3.0f, 3.0f);

	setIdentityMatrix(austria.object.objectTransform, 4);
	setMeshColor(&amesh, 0.2f, 0.2f, 0.2f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);
	austria.position[0] = -11.0f;
	austria.position[1] = 0.0f;
	austria.position[2] = 3.0f;
	myTranslate(amesh.meshTransform, austria.position[0], austria.position[1], austria.position[1]);

	austria.object.meshes.push_back(amesh);

	flags.push_back(portugal);
	flags.push_back(austria);
}


void createAliens() {
	MyMesh amesh;
	Alien alien;
	
	// standing
	amesh = createQuad(4.0f, 4.0f);
	setIdentityMatrix(alien.object.objectTransform, 4);
	setMeshColor(&amesh, 0.5f, 0.5f, 0.5f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);
	alien.position[0] = 0.0f;
	alien.position[1] = 0.8f;
	alien.position[2] = 70.0f;
	alien.type = 0;
	myTranslate(amesh.meshTransform, alien.position[0], alien.position[1], alien.position[1]);

	alien.object.meshes.push_back(amesh);
	aliens.push_back(alien);

	// standing
	amesh = createQuad(4.0f, 4.0f);
	setIdentityMatrix(alien.object.objectTransform, 4);
	setMeshColor(&amesh, 0.5f, 0.5f, 0.5f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);
	alien.position[0] = 3.0f;
	alien.position[1] = 0.8f;
	alien.position[2] = 75.0f;
	alien.type = 0;
	myTranslate(amesh.meshTransform, alien.position[0], alien.position[1], alien.position[1]);

	alien.object.meshes.push_back(amesh);
	aliens.push_back(alien);

	// pointing
	amesh = createQuad(4.0f, 4.0f);
	setIdentityMatrix(alien.object.objectTransform, 4);
	setMeshColor(&amesh, 0.5f, 0.5f, 0.5f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);
	alien.position[0] = -5.0f;
	alien.position[1] = 0.8f;
	alien.position[2] = 71.5f;
	alien.type = 1;
	myTranslate(amesh.meshTransform, alien.position[0], alien.position[1], alien.position[1]);

	alien.object.meshes.push_back(amesh);
	aliens.push_back(alien);

	// together
	amesh = createQuad(4.0f, 4.0f);
	setIdentityMatrix(alien.object.objectTransform, 4);
	setMeshColor(&amesh, 0.5f, 0.5f, 0.5f, 1.0f);
	setIdentityMatrix(amesh.meshTransform, 4);
	alien.position[0] = -8.0f;
	alien.position[1] = 0.8f;
	alien.position[2] = 71.5f;
	alien.type = 2;
	myTranslate(amesh.meshTransform, alien.position[0], alien.position[1], alien.position[1]);

	alien.object.meshes.push_back(amesh);
	aliens.push_back(alien);

}


// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//

void init()
{

	// wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	/* Initialization of DevIL */
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		printf("wrong DevIL version \n");
		exit(0);
	}
	ilInit();

	glGenTextures(11, TextureArray);
	Texture2D_Loader(TextureArray, "textures/ground0.tga", 0);
	Texture2D_Loader(TextureArray, "textures/ground1.tga", 1);
	Texture2D_Loader(TextureArray, "textures/rover.tga", 2);
	Texture2D_Loader(TextureArray, "textures/particle.tga", 3);
	Texture2D_Loader(TextureArray, "textures/flagPortugal.tga", 4);
	Texture2D_Loader(TextureArray, "textures/flagAustria.tga", 5);
	Texture2D_Loader(TextureArray, "textures/RockBaseTex.jpg", 6);
	Texture2D_Loader(TextureArray, "textures/RockNormalTex.jpg", 7);
	Texture2D_Loader(TextureArray, "textures/standing.tga", 8);
	Texture2D_Loader(TextureArray, "textures/pointing.tga", 9);
	Texture2D_Loader(TextureArray, "textures/together.tga", 10);


	//Flare elements textures
	glGenTextures(5, FlareTextureArray);
	Texture2D_Loader(FlareTextureArray, "textures/crcl.tga", 0);
	Texture2D_Loader(FlareTextureArray, "textures/flar.tga", 1);
	Texture2D_Loader(FlareTextureArray, "textures/hxgn.tga", 2);
	Texture2D_Loader(FlareTextureArray, "textures/ring.tga", 3);
	Texture2D_Loader(FlareTextureArray, "textures/sun.tga", 4);

	//Sky Box Texture Object
	const char* filenames[] = { "textures/posx.jpg", "textures/negx.jpg", "textures/posy.jpg", "textures/negy.jpg", "textures/posz.jpg", "textures/negz.jpg" };
	TextureCubeMap_Loader(TextureArray, filenames, 11);

	/// Initialization of freetype library with font_name file
	freeType_init(font_name);

	createGround();
	createPillars();
	createStaticRocks();
	createRollingRocks(10);
	createRover();
	createCameras();
	createSpaceship();
	createFlags();
	createAliens();
	createItems(10);
	createCubeSkybox();

	particleMesh = createQuad(0.03f, 0.01f);
	//particleMesh.mat.texCount = 3; // attribute for texture

	// create geometry and VAO of the quad for flare elements
	flareMesh = createQuad(1, 1);

	//Load flare from file
	loadFlareFile(&flare, "flare.txt");
	initialState(true);
	glutTimerFunc(0, animate, 0);

	// mirror object
	float amb2[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	float diff2[] = { 0.99f, 0.99f, 0.99f, 1.0f };
	float diff3[] = { 0.99f, 0.99f, 0.99f, 0.30f };
	float spec2[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 100.0f;
	int texcount = 0;

	mirror = createQuad(1000, 1000);
	memcpy(mirror.mat.ambient, amb2, 4 * sizeof(float));
	memcpy(mirror.mat.diffuse, diff3, 4 * sizeof(float));
	memcpy(mirror.mat.specular, spec2, 4 * sizeof(float));
	memcpy(mirror.mat.emissive, emissive, 4 * sizeof(float));
	mirror.mat.shininess = shininess;
	mirror.mat.texCount = texcount;

	// rear view camera object
	float amb1[] = { 0.3f, 0.0f, 0.0f, 1.0f };
	float diff1[] = { 0.8f, 0.1f, 0.1f, 1.0f };
	float spec1[] = { 0.3f, 0.3f, 0.3f, 1.0f };

	// create geometry and VAO of the cube
	rearViewCam = createCube();

	// some GL settings
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(243.0f / 255.0f, 206.0f / 255.0f, 180.0f / 255.0f, 1.0f);

}

// ------------------------------------------------------------
//
// Main function
//


int main(int argc, char** argv) {

	srand((unsigned int)time(NULL));
	//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL | GLUT_MULTISAMPLE);

	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WinX, WinY);
	WindowHandle = glutCreateWindow(CAPTION);

	//  Callback Registration
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	glutTimerFunc(0, timerFPS, 0);
	glutTimerFunc(0, refresh, 0);

	//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);


	//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	if (!setupShaders())
		return(1);

	init();

	//  GLUT main loop
	glutMainLoop();

	return(0);
}