#pragma once

#include <vector>

#define MAX_ELEMENTS_PER_FLARE 15
#define NUMBER_OF_TEXTURES 5

typedef struct FLARE_ELEMENT {
    float distance;
    float size;
	float matDiffuse[4];
	int textureId;	
} FLARE_ELEMENT;

class Flare {
public:
    char* flareTextureName[NUMBER_OF_TEXTURES] = {"crcl", "flar", "hxgn", "ring", "sun"};
    float scale;
    float maxSize;
    int numberOfPieces;
    FLARE_ELEMENT element[MAX_ELEMENTS_PER_FLARE];

    Flare();
    Flare(char* filename);
    unsigned int getTextureId(char *name);
    void renderFlare(int lx, int ly, int *m_viewport);
    inline double clamp(const double x, const double min, const double max);
    inline int clampi(const int x, const int min, const int max);
};