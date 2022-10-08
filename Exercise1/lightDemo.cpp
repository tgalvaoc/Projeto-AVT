//
// AVT: Phong Shading and Text rendered with FreeType library
// The text rendering was based on https://learnopengl.com/In-Practice/Text-Rendering
// This demo was built for learning purposes only.
// Some code could be severely optimised, but I tried to
// keep as simple and clear as possible.
//
// The code comes with no warranties, use it at your own risk.
// You may use it, or parts of it, wherever you want.
// 
// Author: Jo�o Madeiras Pereira
//

#include <math.h>
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

using namespace std;

#define CAPTION "Projecto AVT"
int WindowHandle = 0;
int WinX = 1280, WinY = 720;

unsigned int FrameCount = 0;

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
bool spotlight_mode = false;
bool sun_mode = true;
bool point_lights_mode = false;

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

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Frame counting and FPS computation
long myTime, timebase = 0, frame = 0;
char s[32];

#define NUMBER_POINT_LIGHTS 6
#define NUMBER_SPOT_LIGHTS 2


float directionalLightPos[4] = { 1.0f, 1000.0f, 1.0f, 0.0f };
float pointLightPos[NUMBER_POINT_LIGHTS][4] = { {-5.0f, 4.0f, -35.0f, 1.0f}, {0.0f, 0.0f, -10.0f, 1.0f},
	{0.0f, 0.0f, -10.0f, 1.0f}, {0.0f, 0.0f, -10.0f, 1.0f}, {0.0f, 0.0f, -10.0f, 1.0f},
	{0.0f, 0.0f, -10.0f, 1.0f} };
float spotlightPos[NUMBER_SPOT_LIGHTS][4];

void createCameras() {
	cameras[0] = Camera(ORTHOGONAL, 1.0f, 20.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	cameras[1] = Camera(PERSPECTIVE, 1.0f, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	cameras[2] = Camera(MOVING, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f);
}


vector<RollingRock> createRollingRocks(int numToCreate) {

	MyMesh amesh = createSphere(1.0f, 10);
	MyObject stone;
	RollingRock rock;
	vector<RollingRock> rocks;
	int low, high;

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

		low = -25;
		high = 25;

		float r1 = low + static_cast<float>(rand()) * static_cast<float>(high - low) / RAND_MAX;
		float r2 = low + static_cast<float>(rand()) * static_cast<float>(high - low) / RAND_MAX;
		//std::cout << "R1: " << r1 << " r2: " << r2 << "\n";

		low = -1;
		high = 1;

		rock.speed = rand() / static_cast<float>(RAND_MAX);
		rock.directionX = low + static_cast<float>(rand()) * static_cast<float>(high - low) / RAND_MAX;
		rock.directionZ = low + static_cast<float>(rand()) * static_cast<float>(high - low) / RAND_MAX;
		rock.posX = r1;
		rock.posZ = r2;

		//std::cout << "speed: " << stone.speed << " dir0 " << stone.direction[0] << "\n";
		setIdentityMatrix(stone.objectTransform, 4);

		myTranslate(stone.objectTransform, r1, 1, r2);

		stone.meshes.push_back(amesh);
		rock.object = stone;
		rollingRocks.push_back(rock);
	}
	return rocks;
}

void updateRoverCamera(float vx, float vy) {
	cameras[2].target[0] = rover.position[0];
	cameras[2].target[1] = rover.position[1];
	cameras[2].target[2] = rover.position[2];
	cameras[2].pos[0] += vx;
	cameras[2].pos[1] += vy;
}

void updateSpotlightPos() {

	spotlightPos[0][0] = rover.position[0] + 0.5; rover.position[1] - 0.2;
	spotlightPos[0][1] = rover.position[1] - 0.2;
	spotlightPos[0][2] = rover.position[2];
	spotlightPos[0][3] = 1;

	spotlightPos[1][0] = rover.position[0] + 0.5; rover.position[1] - 0.2;
	spotlightPos[1][1] = rover.position[1] + 0.2;
	spotlightPos[1][2] = rover.position[2];
	spotlightPos[1][3] = 1;
}

void animateRocks() {
	vector<RollingRock> rocks;
	vector<RollingRock> aux;
	for (int i = 0; i < rollingRocks.size(); i++) {
		float translateX = rollingRocks[i].speed * rollingRocks[i].directionX;
		float translateZ = rollingRocks[i].speed * rollingRocks[i].directionZ;

		myTranslate(rollingRocks[i].object.objectTransform, translateX, 0, translateZ);
		rollingRocks[i].posX += translateX;
		rollingRocks[i].posZ += translateZ;

		// TODO: melhorar barreiras
		if (rollingRocks[i].posX > 50 || rollingRocks[i].posZ > 50 ||
			rollingRocks[i].posX < -50 || rollingRocks[i].posZ < -50) {
			rollingRocks.erase(rollingRocks.begin() + i);
			aux = createRollingRocks(1);
			if(!aux.empty())
				rocks.push_back(aux[0]);
			//std::cout << "speed: " << rollingRocks[i].speed << " posX: " << rollingRocks[i].posX << " posX: " << rollingRocks[i].posZ << "\n";
		}
	}
	for (int j = 0; j < rocks.size(); j++)
		myObjects.push_back(rocks[j].object);
}

void timer(int value)
{
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ") " << value;
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
	FrameCount = 0;
	glutTimerFunc(1000, timer, value + 1);
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
// Render stufff
//

void renderScene(void) {
	//desenha efetivamente os objetos
	GLint loc;

	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);

	// set the camera using a function similar to gluLookAt

	cameras[currentCamera].setProjection((float)WinX, (float)WinY);
	cameras[currentCamera].cameraLookAt();

	// use our shader

	glUseProgram(shader.getProgramIndex());

	//send the light position in eye coordinates
	//glUniform4fv(lPos_uniformId, 1, lightPos); //efeito capacete do mineiro, ou seja lighPos foi definido em eye coord 


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


	float res[4];	//lightPos definido em World Coord so is converted to eye space

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


	animateRocks();
	rover.updatePosition(NONE);

	myObjects.clear();

	myObjects.push_back(ground);
	for (int j = 0; j < rollingRocks.size(); j++)
		myObjects.push_back(rollingRocks[j].object);

	for (int i = 0; i < myObjects.size(); i++) {

		vector<MyMesh> meshes = myObjects[i].meshes;

		pushMatrix(MODEL);

		multMatrix(MODEL, myObjects[i].objectTransform);

		for (int objId = 0; objId < meshes.size(); objId++) {
			//if (j == 2 && i == 1) continue;
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
	// separei para o rover pq dava problema de insercao/remoção muita rápida

	vector<MyMesh> meshes = rover.rover.meshes;

	pushMatrix(MODEL);

	multMatrix(MODEL, rover.rover.objectTransform);

	for (int objId = 0; objId < meshes.size(); objId++) {
		//if (j == 2 && i == 1) continue;
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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
	//RenderText(shaderText, "This is a sample text", 25.0f, 25.0f, 1.0f, 0.5f, 0.8f, 0.2f);
	//RenderText(shaderText, "AVT Light and Text Rendering Demo", 440.0f, 570.0f, 0.5f, 0.3, 0.7f, 0.9f);
	popMatrix(PROJECTION);
	popMatrix(VIEW);
	popMatrix(MODEL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glutSwapBuffers();
}

void specialFunc(int key, int xx, int yy)
{
	float amount = 1.0f;

	switch (key) {
	case GLUT_KEY_UP:
		//carrinho.mover(amount, 0.0f);
		break;
	case GLUT_KEY_DOWN:
		//carrinho.mover(-amount, 0.0f);
		break;
	case GLUT_KEY_LEFT:
		//carrinho.mover(0.0f, amount);
		break;
	case GLUT_KEY_RIGHT:
		//carrinho.mover(0.0f, -amount);
		break;

	}

}

// ------------------------------------------------------------
//
// Events from the Keyboard
//
//AS TECLAS EST�O AQUI
void processKeys(unsigned char key, int xx, int yy)
{
	std::tuple<float, float> aux;
	switch (key) {
	case 27:
		glutLeaveMainLoop();
		break;
	case 'q':
	case'Q':
		aux = rover.updatePosition(FRONT);
		updateRoverCamera(std::get<0>(aux), std::get<1>(aux));
		updateSpotlightPos();
		break;
	case 'a':
	case'A':
		aux = rover.updatePosition(BACK);
		updateRoverCamera(std::get<0>(aux), std::get<1>(aux));
		updateSpotlightPos();
		break;
	case 'o':
	case'O':
		rover.rotateRover(LEFT);
		break;
	case 'p':
	case'P':
		rover.rotateRover(RIGHT);
		break;
	
	//case 'm': glEnable(GL_MULTISAMPLE); break;
	//case 'n': glDisable(GL_MULTISAMPLE); break;

	case '1':
		currentCamera = 0;
		break;
	case '2':
		currentCamera = 1;
		break;
	case '3':
		currentCamera = 2;
		break;
	case 'c':
	case 'C':
		if (!point_lights_mode)
			point_lights_mode = true;
		else
			point_lights_mode = false;
		break;
	case 'h':
	case 'H':
		if (!spotlight_mode)
			spotlight_mode = true;
		else
			spotlight_mode = false;
		break;
	case 'n':
	case 'N':
		if (!sun_mode)
			sun_mode = true;
		else
			sun_mode = false;
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
		else if (button == GLUT_RIGHT_BUTTON)
			tracking = 2;
	}

	//stop tracking the mouse
	else if (state == GLUT_UP) {
		if (tracking == 1) {
			//cout << "soltando" << endl;
			//alpha -= (xx - startX);
			//beta += (yy - startY);
		}
		else if (tracking == 2) {
			//r += (yy - startY) * 0.01f;
			//if (r < 0.1f)
			//	r = 0.1f;
		}
		tracking = 0;
	}
}

// Track mouse motion while buttons are pressed

void processMouseMotion(int xx, int yy)
{

	int deltaX, deltaY;
	float alphaAux, betaAux;
	float rAux;

	deltaX = xx - startX;
	deltaY = yy - startY;

	// cout << "teste" << endl;

	// left mouse button: move camera
	if (currentCamera == 2 && tracking == 1) {
		cameras[currentCamera].rotateCamera(deltaX, deltaY);
	}


	//  uncomment this if not using an idle or refresh func
	glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

	/*r += direction * 0.1f;
	if (r < 0.1f)
		r = 0.1f;

	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r *   						     sin(beta * 3.14f / 180.0f);
	*/

	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
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

void setMeshColor(MyMesh* amesh, float r, float g, float b)
{
	float amb[] = { r / 4.0, g / 4.0, b / 4.0, 1.0f };
	float diff[] = { r, g, b, 1.0f };
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

	setIdentityMatrix(ground.objectTransform, 4);

	MyMesh amesh = createQuad(1000.0f, 1000.0f);
	/*
	float amb[] = { 0.2f, 1.0f, 0.1f, 1.0f };
	float diff[] = { 1.0f, 0.6f, 0.4f, 1.0f };
	float spec[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));

	amesh.mat.shininess = 100.0f;
	amesh.mat.texCount = 0;
	*/
	setMeshColor(&amesh, 0.9, 0.8, 0.9);
	setIdentityMatrix(amesh.meshTransform, 4);

	myRotate(amesh.meshTransform, -90.0, 1.0, 0.0, 0.0);

	ground.meshes.push_back(amesh);
	myObjects.push_back(ground);
}

void createRover() {
	MyObject roverObj;
	setIdentityMatrix(roverObj.objectTransform, 4);

	MyMesh corpo = createCube();
	setMeshColor(&corpo, 0.5, 0.5, 0.5);
	setIdentityMatrix(corpo.meshTransform, 4);
	myTranslate(corpo.meshTransform, -0.5, -0.5, -0.5); // move o cubo pro centro
	myScale(corpo.meshTransform, 3.0, 1.0, 1.5); // ajusta as dimensoes
	myTranslate(corpo.meshTransform, -0.75/2, 0.5, -0.75/4); // coloca o corpo no centro, tocando no chao
	myTranslate(corpo.meshTransform, 0.0, 0.25, 0.0); // tira o corpo do chao

	//myTranslate(corpo.meshTransform, -0.5 2.5, -0.5);



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

	

	MyMesh roda1 = createTorus(0.5, 0.75, 80, 80);
	setIdentityMatrix(roda1.meshTransform, 4);
	myTranslate(roda1.meshTransform, 1.2, 0.85, -0.75); // coloca a roda no lugar
	myRotate(roda1.meshTransform, 90.0, 1.0, 0.0, 0.0); // coloca ela na vertical

	MyMesh roda2 = createTorus(0.5, 0.75, 80, 80);
	setIdentityMatrix(roda2.meshTransform, 4);
	myTranslate(roda2.meshTransform, -1.2, -0.85, -0.75); // coloca a roda no lugar
	myRotate(roda2.meshTransform, 90.0, 1.0, 0.0, 0.0); // coloca ela na vertical




	
	roverObj.meshes.push_back(corpo);
	roverObj.meshes.push_back(roda1);
	roverObj.meshes.push_back(roda2);
	//roverObj.meshes.push_back(roda3);
	//roverObj.meshes.push_back(roda4);
	rover = Rover(roverObj);
	updateSpotlightPos();
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

	createCameras();

	// -------


	// place objects in world

	//srand(0);

	createGround();
	createRover();
	createRollingRocks(10);

	// some GL settings
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

	glutTimerFunc(0, timer, 0);
	//glutIdleFunc(renderScene);  // Use it for maximum performance
	glutTimerFunc(0, refresh, 0);    //use it to to get 60 FPS whatever

	//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
	glutSpecialFunc(specialFunc);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	glutMouseWheelFunc(mouseWheel);


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


