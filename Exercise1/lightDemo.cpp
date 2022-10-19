#include <math.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>


// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

#include <IL/il.h>


// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "geometry.h"
#include "Texture_Loader.h"

#include "avtFreeType.h"

#include "Camera.h"
#include "Rover.h"
#include <list>
#define _USE_MATH_DEFINES

using namespace std;

#define CAPTION "Projecto AVT"
int WindowHandle = 0;
int WinX = 1280, WinY = 720;

unsigned int FrameCount = 0;
float delta = 0.015;

float alpha, beta, r;

#define M_PI       3.14159265358979323846f

//shaders
VSShaderLib shader;  //geometry
VSShaderLib shaderText;  //render bitmap text

//File with the font
const string font_name = "fonts/arial.ttf";

//Vector with objects, and each object can have one or more meshes
vector<MyObject> myObjects;

MyObject ground;
Rover rover;
vector<RollingRock> rollingRocks;
vector<Rock> rocks;
list<Pillar> pillars;

bool pauseActive = false;
bool gameOver = false;

bool spotlight_mode = true;
bool sun_mode = true;
bool point_lights_mode = false;
bool fog_mode = false;
bool multitexture_mode = false;

bool isGoingForward = false;
bool isHittingPillar = false;
bool isHittingRock = false;

GLuint TextureArray[3];

//External array storage defined in AVTmathLib.cpp

/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;
GLint lPos_uniformId; // ?
GLint tex_loc, tex_loc1, tex_loc2;
GLint texMode_uniformId;

Camera cameras[3];
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


float directionalLightPos[4] = { 1.0f, 1000.0f, 1.0f, 0.0f };
float pointLightPos[NUMBER_POINT_LIGHTS][4] = { {-5.0f, 8.0f, -5.0f, 1.0f}, {-5.0f, 8.0f, 5.0f, 1.0f},
	{5.0f, 8.0f, -5.0f, 1.0f}, {5.0f, 8.0f, 5.0f, 1.0f}, {-5.0f, 8.0f, 0.0f, 1.0f},
	{5.0f, 8.0f, 0.0f, 1.0f} };
float spotlightPos[NUMBER_SPOT_LIGHTS][4];
float coneDir[4];

void initialState(bool livesReset) {

	if (livesReset)
		livesCount = 5;

	rover.position[0] = 0;
	rover.position[1] = 0;
	rover.position[2] = 0;
	rover.angle = 0;
	rover.speed = 0;

	cameras[2].position[0] = 10;
	cameras[2].position[1] = 5;
	cameras[2].position[2] = 0;

	r = sqrt(pow(cameras[2].position[0], 2) + pow(cameras[2].position[1], 2) + pow(cameras[2].position[2], 2));
	alpha = acos(cameras[2].position[2] / r) * 180.0 / M_PI;
	beta = atan(cameras[2].position[1] / cameras[2].position[0]) * 180.0 / M_PI;
}

void createCameras() {
	cameras[0] = Camera(ORTHOGONAL, 1.0f, 20.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	cameras[1] = Camera(PERSPECTIVE, 1.0f, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	cameras[2] = Camera(MOVING, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f);
}


vector<RollingRock> createRollingRocks(int numToCreate) {

	MyMesh amesh = createSphere(1.0f, 10); // randomize size in the future
	MyObject stone;
	RollingRock rock;
	vector<RollingRock> rocks;
	int low, high;
	int signal;

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
			rock.speed = - rock.speed;

		low = 0;
		high = 1;
		
		signal = rand() % 2;
		rock.direction[0] = low + static_cast<float>(rand()) * static_cast<float>(high - low) / RAND_MAX;
		if (signal)
			rock.direction[0] = - rock.direction[0];

		signal = rand() % 2;
		rock.direction[2] = low + static_cast<float>(rand()) * static_cast<float>(high - low) / RAND_MAX;
		if (signal)
			rock.direction[2] = - rock.direction[2];
		

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


void animateRocks() {
	vector<RollingRock> rocks;
	vector<RollingRock> aux;
	for (int i = 0; i < rollingRocks.size(); i++) {
		rollingRocks[i].speed += rollingRocks[i].speed * 0.005;
		float translateX = rollingRocks[i].speed * rollingRocks[i].direction[0];
		float translateZ = rollingRocks[i].speed * rollingRocks[i].direction[2];

		myTranslate(rollingRocks[i].object.objectTransform, translateX, 0, translateZ);
		rollingRocks[i].position[0] += translateX;
		rollingRocks[i].position[2] += translateZ;

		if (rollingRocks[i].position[0] > rover.position[0] + 200 || rollingRocks[i].position[2] > rover.position[2] + 200 ||
			rollingRocks[i].position[0] < rover.position[0] - 200 || rollingRocks[i].position[2] < rover.position[2] - 200) {
			rollingRocks.erase(rollingRocks.begin() + i);
			aux = createRollingRocks(1);
			if (!aux.empty())
				rocks.push_back(aux[0]);
		}
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
	
	spotlightPos[0][0] = rover.position[0] + rover.direction[0] * (-1.0f);
	spotlightPos[0][1] = rover.position[1] + 0.5f;
	spotlightPos[0][2] = rover.position[2] + 0.5f;
	spotlightPos[0][3] = 1.0f;

	spotlightPos[1][0] = rover.position[0] + rover.direction[0] * (-1.0f);
	spotlightPos[1][1] = rover.position[1] + 0.5f;
	spotlightPos[1][2] = rover.position[2] - 0.5f;
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
	cameras[2].position[1] = 5;
	cameras[2].position[2] = rover.position[2] - rover.direction[2] * 10;

	std::copy(rover.position, rover.position + 3, cameras[2].target);
}


void checkCollisions() {


	float roverFactor = 2.0;

	float roverMaxX = rover.position[0] + roverFactor;
	float roverMinX = rover.position[0] - roverFactor;
	float roverMaxZ = rover.position[2] + roverFactor;
	float roverMinZ = rover.position[2] - roverFactor;

	// collision with static rocks
	isHittingRock = false;
	for each (Rock rock in rocks) {

		float maxX = rock.position[0] + rock.radius;
		float minX = rock.position[0] - rock.radius;
		float maxZ = rock.position[2] + rock.radius;
		float minZ = rock.position[2] - rock.radius;

		if ((minX <= roverMaxX && minX >= roverMinX && rock.position[2] >= roverMinZ && rock.position[2] <= roverMaxZ) ||
			(maxX <= roverMaxX && maxX >= roverMinX && rock.position[2] >= roverMinZ && rock.position[2] <= roverMaxZ) ||
			(rock.position[0] <= roverMaxX && rock.position[0] >= roverMinX && minZ >= roverMinZ && minZ <= roverMaxZ) ||
			(rock.position[0] <= roverMaxX && rock.position[0] >= roverMinX && maxZ >= roverMinZ && maxZ <= roverMaxZ)) {

			rover.position[0] += rover.direction[0] * rover.speed * delta;
			rover.position[2] -= rover.direction[2] * rover.speed * delta;

			rock.speed = 2;
			rover.speed = 0;

			isHittingRock = true;
			
			if (isGoingForward) {
				rock.direction[0] = rover.direction[0];
				rock.direction[1] = rover.direction[1];
				rock.direction[2] = rover.direction[2];
			}
			else {
				rock.direction[0] = -rover.direction[0];
				rock.direction[1] = -rover.direction[1];
				rock.direction[2] = -rover.direction[2];
			}
			//rock.speed = ((isGoingForward) ? rock.speed + 0.1 : rock.speed - 0.1);
			//rock.speed /= rock.radius;

			rock.position[0] = 0;
		}
	}

	// colisão com pilares
	isHittingPillar = false;
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

			isHittingPillar = true;
			break;
		}
	}

	// colisão com as rochas
	for each (RollingRock rock in rollingRocks) {

		float maxX = rock.position[0] + rock.radius;
		float minX = rock.position[0] - rock.radius;
		float maxZ = rock.position[2] + rock.radius;
		float minZ = rock.position[2] - rock.radius;

		if ((minX <= roverMaxX && minX >= roverMinX && rock.position[2] >= roverMinZ && rock.position[2] <= roverMaxZ) ||
			(maxX <= roverMaxX && maxX >= roverMinX && rock.position[2] >= roverMinZ && rock.position[2] <= roverMaxZ) ||
			(rock.position[0] <= roverMaxX && rock.position[0] >= roverMinX && minZ >= roverMinZ && minZ <= roverMaxZ) ||
			(rock.position[0] <= roverMaxX && rock.position[0] >= roverMinX && maxZ >= roverMinZ && maxZ <= roverMaxZ)) {

			
			if(--livesCount <= 0){
				livesCount = 0;
				gameOver = true;
			}
			else{
				initialState(false);
				rollingRocks.clear();
				createRollingRocks(7);
				break;
			}
		}
	}

}



void animate(int value) {

	if (!pauseActive && !gameOver) {

		updateRoverPosition();

		checkCollisions();

		for each (Rock rock in rocks) {

			float translateX = rock.speed * rock.direction[0];
			float translateZ = rock.speed * rock.direction[2];

			rock.position[0] += translateX;
			rock.position[2] += translateZ;

			myTranslate(rock.object.objectTransform, translateX, 0, translateZ);

			if (rock.speed != 0)
				rock.speed += -rock.speed / 2;
		}

		updateSpotlight();


		if (tracking == 0)
			updateRoverCamera();

		animateRocks();
	}

	glutTimerFunc(20, animate, 0);
}


// ------------------------------------------------------------
//
// Render stufff
//

void renderScene(void) {
	//desenha efetivamente os objetos
	GLint loc;

	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);


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


	loc = glGetUniformLocation(shader.getProgramIndex(), "multitexture_mode");
	if (multitexture_mode)
		glUniform1i(loc, 1);
	else
		glUniform1i(loc, 0);


	float res[4];
	multMatrixPoint(VIEW, coneDir, res);
	loc = glGetUniformLocation(shader.getProgramIndex(), "coneDir");
	glUniform4fv(loc, 1, res);
	loc = glGetUniformLocation(shader.getProgramIndex(), "spotCosCutOff");
	glUniform1f(loc, 0.99f);



	//Associar os Texture Units aos Objects Texture
	//stone.tga loaded in TU0; checker.tga loaded in TU1;  lightwood.tga loaded in TU2

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureArray[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TextureArray[1]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, TextureArray[2]);

	//Indicar aos tres samplers do GLSL quais os Texture Units a serem usados
	glUniform1i(tex_loc, 0);
	glUniform1i(tex_loc1, 1);
	glUniform1i(tex_loc2, 2);


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

	myObjects.clear();
	myObjects.push_back(ground);
	for (int j = 0; j < rollingRocks.size(); j++)
		myObjects.push_back(rollingRocks[j].object);
	for (int j = 0; j < rocks.size(); j++)
		myObjects.push_back(rocks[j].object);


	for (int i = 0; i < myObjects.size(); i++) {

		vector<MyMesh> meshes = myObjects[i].meshes;

		pushMatrix(MODEL);

		multMatrix(MODEL, myObjects[i].objectTransform);

		for (int objId = 0; objId < meshes.size(); objId++) {

			loc = glGetUniformLocation(shader.getProgramIndex(), "ground");
			if (i == 0 && objId == 0)
				glUniform1i(loc, 1);
			else
				glUniform1i(loc, 0);

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

			if (i == 0)
				glUniform1i(texMode_uniformId, 0); // textura para a superficie
			else
				glUniform1i(texMode_uniformId, 2); // textura para as rolling rocks

			glBindVertexArray(meshes[objId].vao);
			glDrawElements(meshes[objId].type, meshes[objId].numIndexes, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			popMatrix(MODEL);
		}
		popMatrix(MODEL);
	}

	// Rover ---------------------------------------------------
	
	vector<MyMesh> meshes = rover.object.meshes;

	pushMatrix(MODEL);

	multMatrix(MODEL, rover.object.objectTransform);
	
	translate(MODEL, rover.position[0], 0, rover.position[2]);
	rotate(MODEL, rover.angle, 0, 1, 0);

	
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
		glUniform1i(texMode_uniformId, 1); // textura para o corpo do rover

		glBindVertexArray(meshes[objId].vao);
		glDrawElements(meshes[objId].type, meshes[objId].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
	}
	popMatrix(MODEL);

	//Render text (bitmap fonts) in screen coordinates. So use ortoghonal projection with viewport coordinates.
	glDisable(GL_DEPTH_TEST);
	//the glyph contains transparent background colors and non-transparent for the actual character pixels. So we use the blending


	int m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	//viewer at origin looking down at  negative z direction
	pushMatrix(MODEL);
	loadIdentity(MODEL);
	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);
	pushMatrix(VIEW);
	loadIdentity(VIEW);
	ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);
	RenderText(shaderText, (const GLchar*)("Lives: " + to_string(livesCount)).c_str(), WinX - 190, WinY - 48, 1.0f, 0.8f, 0.8f, 0.8f);
	RenderText(shaderText, "00000", 10, WinY - 48, 1.0f, 0.8f, 0.8f, 0.8f);
	if (pauseActive)
		RenderText(shaderText, "PAUSE", WinX/2 - 80, WinY /2 + 220, 1.0f, 0.8f, 0.8f, 0.8f);
	if (gameOver)
		RenderText(shaderText, "GAME OVER", WinX / 2 - 150 , WinY / 2 + 220, 1.0f, 0.8f, 0.8f, 0.8f);
	popMatrix(PROJECTION);
	popMatrix(VIEW);
	popMatrix(MODEL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

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

		if (isHittingPillar && isGoingForward)
			return;
		
		if (isHittingRock && isGoingForward)
			return;

		rover.speed += 0.8;
		beta = atan(cameras[2].position[1] / cameras[2].position[0]) * 180.0 / M_PI;
		isGoingForward = true;
		break;
	case 'a':
	case'A':
		if (pauseActive || gameOver)
			return;

		if (isHittingPillar && !isGoingForward) 
			return;
		
		if (isHittingRock && !isGoingForward) 
			return;
		
		rover.speed -= 0.8;
		beta = atan(cameras[2].position[1] / cameras[2].position[0]) * 180.0 / M_PI;
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
	
	deltaX = - xx + startX;
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


	//  uncomment this if not using an idle or refresh func
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

	glLinkProgram(shader.getProgramIndex());
	printf("InfoLog for Model Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shader.isProgramValid()) {
		printf("GLSL Model Program Not Valid!\n");
		exit(1);
	}

	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_pos");
	tex_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");


	printf("InfoLog for Per Fragment Phong Lightning Shader\n%s\n\n", shader.getAllInfoLogs().c_str());

	// Shader for bitmap Text
	shaderText.init();
	shaderText.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/text.vert");
	shaderText.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/text.frag");

	glLinkProgram(shaderText.getProgramIndex());
	printf("InfoLog for Text Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shaderText.isProgramValid()) {
		printf("GLSL Text Program Not Valid!\n");
		exit(1);
	}

	return(shader.isProgramLinked() && shaderText.isProgramLinked());
}


void setMeshColor(MyMesh* amesh, float r, float g, float b, float a)
{
	float amb[] = { r / 4.0, g / 4.0, b / 4.0, 1.0f };
	float diff[] = { r, g, b, a };

	float spec[] = { r, g, b, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	memcpy(amesh->mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh->mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh->mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh->mat.emissive, emissive, 4 * sizeof(float));

	amesh->mat.shininess = 100.0f;
	amesh->mat.texCount = 0;
}

void createGround() {
	//surface
	setIdentityMatrix(ground.objectTransform, 4);

	Pillar pillar1, pillar2, pillar3, pillar4;
	MyMesh amesh = createQuad(1000.0f, 1000.0f);

	setMeshColor(&amesh, 0.8, 0.4, 0.0, 1.0);
	setIdentityMatrix(amesh.meshTransform, 4);

	myRotate(amesh.meshTransform, -90.0, 1.0, 0.0, 0.0);

	ground.meshes.push_back(amesh);

	// landing site
	float side = 8.0f;
	amesh = createQuad(side, side);
	setIdentityMatrix(ground.objectTransform, 4);
	setMeshColor(&amesh, 0.8, 0.8, 0.8, 1.0);
	setIdentityMatrix(amesh.meshTransform, 4);

	myTranslate(amesh.meshTransform, 0.0, 0.15, 0.0);
	myRotate(amesh.meshTransform, -90.0, 1.0, 0.0, 0.0);
	ground.meshes.push_back(amesh);

	// pillar
	pillar1.radius = 0.2;
	pillar1.position[0] = side / 2;
	pillar1.position[1] = 0;
	pillar1.position[2] = side / 2;

	amesh = createCylinder(7, pillar1.radius, 10);
	setMeshColor(&amesh, 0.82, 0.17, 0.03, 1.0);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, pillar1.position[0], pillar1.position[1], pillar1.position[2]);
	ground.meshes.push_back(amesh);


	pillar2.radius = 0.2;
	pillar2.position[0] = side /2;
	pillar2.position[1] = 0;
	pillar2.position[2] = -side / 2;

	amesh = createCylinder(7, pillar2.radius, 10);
	setMeshColor(&amesh, 0.82, 0.17, 0.03, 1.0);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, pillar2.position[0], pillar2.position[1], pillar2.position[2]);
	ground.meshes.push_back(amesh);

	pillar3.radius = 0.2;
	pillar3.position[0] = -side / 2;
	pillar3.position[1] = 0;
	pillar3.position[2] = side / 2;

	amesh = createCylinder(7, pillar3.radius, 10);
	setMeshColor(&amesh, 0.82, 0.17, 0.03, 1.0);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, pillar3.position[0], pillar3.position[1], pillar3.position[2]);
	ground.meshes.push_back(amesh);

	pillar4.radius = 0.2;
	pillar4.position[0] = -side / 2;
	pillar4.position[1] = 0;
	pillar4.position[2] = -side / 2;

	amesh = createCylinder(7, pillar4.radius, 10);
	setMeshColor(&amesh, 0.82, 0.17, 0.03, 1.0);
	setIdentityMatrix(amesh.meshTransform, 4);
	myTranslate(amesh.meshTransform, pillar4.position[0], pillar4.position[1], pillar4.position[2]);
	ground.meshes.push_back(amesh);

	pillars.push_back(pillar1);
	pillars.push_back(pillar2);
	pillars.push_back(pillar3);
	pillars.push_back(pillar4);

}

void createRover() {
	MyObject roverObj;
	setIdentityMatrix(roverObj.objectTransform, 4);

	MyMesh body = createCube();
	setMeshColor(&body, 0.27, 0.71, 0.77, 0.6);
	setIdentityMatrix(body.meshTransform, 4);
	myTranslate(body.meshTransform, -0.5, -0.5, -0.5); // move o cubo pro centro
	myScale(body.meshTransform, 3.0, 1.0, 1.5); // ajusta as dimensoes
	myTranslate(body.meshTransform, -0.75 / 2, 0.5, -0.75 / 4); // coloca o corpo no centro, tocando no chao
	myTranslate(body.meshTransform, 0.0, 0.25, 0.0); // tira o corpo do chao

	// vista de frente
	// =        = 0.25
	// =|------|=
	// =|------|=
	// =|------|= 1.0
	// =|------|=
	// =        = 0.25

	// altura da roda: 0.25 + 1.0 + 0.25 = 1.5
	// altura do corpo: 1.0
	// distancia do corpo pro chao: 0.25


	MyMesh wheel1 = createTorus(0.5, 0.75, 80, 80);
	setMeshColor(&wheel1, 0.35, 0.18, 0.08, 1.0);
	setIdentityMatrix(wheel1.meshTransform, 4);
	myTranslate(wheel1.meshTransform, 1.2, 0.85, 0.75); // coloca a roda no lugar
	myRotate(wheel1.meshTransform, 90.0, 1.0, 0.0, 0.0); // coloca ela na vertical

	MyMesh wheel2 = createTorus(0.5, 0.75, 80, 80);
	setMeshColor(&wheel2, 0.35, 0.18, 0.08, 1.0);
	setIdentityMatrix(wheel2.meshTransform, 4);
	myTranslate(wheel2.meshTransform, 1.2, 0.85, -0.75); // coloca a roda no lugar
	myRotate(wheel2.meshTransform, 90.0, 1.0, 0.0, 0.0); // coloca ela na vertical

	MyMesh wheel3 = createTorus(0.5, 0.75, 80, 80);
	setMeshColor(&wheel3, 0.35, 0.18, 0.08, 1.0);
	setIdentityMatrix(wheel3.meshTransform, 4);
	myTranslate(wheel3.meshTransform, -1.2, 0.85, 0.75); // coloca a roda no lugar
	myRotate(wheel3.meshTransform, 90.0, 1.0, 0.0, 0.0); // coloca ela na vertical

	MyMesh wheel4 = createTorus(0.5, 0.75, 80, 80);
	setMeshColor(&wheel4, 0.35, 0.18, 0.08, 1.0);
	setIdentityMatrix(wheel4.meshTransform, 4);
	myTranslate(wheel4.meshTransform, -1.2, 0.85, -0.75); // coloca a roda no lugar
	myRotate(wheel4.meshTransform, 90.0, 1.0, 0.0, 0.0); // coloca ela na vertical

	MyMesh head = createPawn();
	setMeshColor(&head, 0.27, 0.71, 0.77, 1.0);
	setIdentityMatrix(head.meshTransform, 4);
	myScale(head.meshTransform, 0.8, 0.5, 0.5); // ajusta as dimensoes
	myTranslate(head.meshTransform, -1.0, 0.5, 0); // coloca o corpo no centro, tocando no chao
	myTranslate(head.meshTransform, 0.0, 1, 0.0); // tira o corpo do chao

	roverObj.meshes.push_back(wheel1);
	roverObj.meshes.push_back(wheel2);
	roverObj.meshes.push_back(wheel3);
	roverObj.meshes.push_back(wheel4);
	roverObj.meshes.push_back(head);
	roverObj.meshes.push_back(body);

	rover = Rover(roverObj);
	rover.updateDirection();
	updateSpotlight();
}

void createRocks() {

	MyMesh amesh = createSphere(4.0f, 10);
	Rock rock1;
    MyObject obj1;
    setIdentityMatrix(obj1.objectTransform, 4);

    setMeshColor(&amesh, 0.27, 0.71, 0.77, 1.0);
    setIdentityMatrix(amesh.meshTransform, 4);
    myTranslate(amesh.meshTransform, 20.0, 0.2, 10.0);
    obj1.meshes.push_back(amesh);
	rock1.object = obj1;
	rock1.radius = 4.0;
	rock1.position[0] = 20;
	rock1.position[2] = 10;
	rock1.speed = 0;
    rocks.push_back(rock1);

	Rock rock2;
	MyObject obj2;

	setIdentityMatrix(obj2.objectTransform, 4);
    amesh = createSphere(1.0f, 10);
    setMeshColor(&amesh, 0.27, 0.71, 0.77, 1.0);
    setIdentityMatrix(amesh.meshTransform, 4);
    myTranslate(amesh.meshTransform, -15.0, 0.2, -15.0);
    obj2.meshes.push_back(amesh);
	rock2.object = obj2;
	rock2.radius = 1.0;
	rock2.position[0] = -15;
	rock2.position[2] = -15;
	rock2.speed = 0;
	rocks.push_back(rock2);
	
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

	glGenTextures(3, TextureArray);
	Texture2D_Loader(TextureArray, "mars_texture.jpg", 0);
	Texture2D_Loader(TextureArray, "steel_texture.jpg", 1);
	Texture2D_Loader(TextureArray, "rock_texture.jpg", 2);


	/// Initialization of freetype library with font_name file
	freeType_init(font_name);


	createGround();
	createRocks();
	createRollingRocks(10);
	createRover();
	createCameras();

	initialState(true);
	glutTimerFunc(0, animate, 0);

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

	//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);

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


