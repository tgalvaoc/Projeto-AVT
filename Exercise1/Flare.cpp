#include "Flare.h"
#include "AVTmathlib.h"
#include <iostream>

using namespace std;

Flare::Flare() {};

unsigned int Flare::getTextureId(char *name) {
	for(int i = 0; i < NUMBER_OF_TEXTURES; ++i) {
		if(strncmp(name, this->flareTextureName[i], strlen(name)) == 0)
			return i;
	}
	return -1;
}

inline double Flare::clamp(const double x, const double min, const double max) {
	return (x < min ? min : (x > max ? max : x));
}

inline int clampi(const int x, const int min, const int max) {
	return (x < min ? min : (x > max ? max : x));
}

Flare::Flare(char* filename) {
	FILE *file;
	char buffer[256];
	int fields;
	int obj_counter = 0;

	file = fopen(filename, "r");
	if(file) {
		fgets(buffer, sizeof(buffer), file);
		sscanf(buffer, "%f %f", this->scale, this->maxSize);
		while(!feof(file)) {

		char name[8] = {'\0',};
		double distance = 0.0f;
		double size = 0.0f;
		float color[4];
		int id;

		fgets(buffer, sizeof(buffer), file);
		fields = sscanf(buffer, "%4s %lf %lf ( %f %f %f %f )", name, &distance, &size, &color[3], &color[0], &color[1], &color[2]);
		if(fields == 7) {
				for (int i = 0; i < 4; ++i) color[i] = clamp(color[i] / 255.0f, 0.0f, 1.0f);
				id = getTextureId(name);
				if (id < 0) printf("Texture name not recognized!\n");
				else
					this->element[obj_counter].textureId = id;
					this->element[obj_counter].distance = (float)distance;
					this->element[obj_counter].size = (float)size;
					memcpy(this->element[obj_counter].matDiffuse, color, 4 * sizeof(float));
					++obj_counter;
		}

		this->numberOfPieces = obj_counter;
		fclose(file);
		}
	}
	else printf("Could not open Flare file!\n");
}

void Flare:: renderFlare(int lx, int ly, int *m_viewport) {
//TODO: add stuff
}


