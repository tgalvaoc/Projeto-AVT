#pragma once

#define MAX_TEXTURES 8
#include <vector>
#include <GL/glew.h>
#include <GL/freeglut.h>

using namespace std;

enum texType {DIFFUSE, SPECULAR, NORMALS, BUMP};

struct Material{
	float diffuse[4];
	float ambient[4];
	float specular[4];
	float emissive[4];
	float shininess;
	int texCount;
};


// A model can be made of many meshes. Each is stored  in the following structure
struct MyMesh {
	//vertex array objects
		GLuint vao;
		GLuint texUnits[MAX_TEXTURES];
		texType texTypes[4];
		float meshTransform[16]; // posiciona a mesh em relacao ao centro do objeto
	//indexes
		GLuint numIndexes;
		unsigned int type;
		struct Material mat;
};

struct MyObject {
	float objectTransform[16]; // posiciona o objeto em relacao ao centro do mundo
	vector<MyMesh> meshes;
};

struct Pillar {
	float position[3];
	float radius;
};

struct Rock{
	float position[3];
	float direction[3];
	float radius;
	float speed;
	MyObject object;
};


struct RollingRock {
	float radius = 0;
	float speed = 0;
	float position[3];
	float direction[3];
	MyObject object;
};

MyMesh createCube();
MyMesh createQuad(float size_x, float size_y);
MyMesh createSphere(float radius, int divisions);
MyMesh createTorus(float innerRadius, float outerRadius, int rings, int sides);
MyMesh createCylinder(float height, float radius, int sides);
MyMesh createCone(float height, float baseRadius, int sides);
MyMesh createPawn();
MyMesh computeVAO(int numP, float *p, float *pfloatoints, int sides, float smoothCos);
int revSmoothNormal2(float *p, float *nx, float *ny, float smoothCos, int beginEnd);
float *circularProfile(float minAngle, float maxAngle, float radius, int divisions, float transX= 0.0f, float transY = 0.0f);
void ComputeTangentArray(int vertexCount, float *vertex, float *normal, float *texcoord, GLuint indexesCount, GLuint *faceIndex, float *tangent);
