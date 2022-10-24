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


typedef struct FLARE_DEF {
    float           scale;     // Scale factor for adjusting overall size of flare elements.
    float           maxSize;   // Max size of largest element, as proportion of screen width (0.0-1.0)
    int             numberOfPieces;    // Number of elements in use
    FLARE_ELEMENT    element[MAX_ELEMENTS_PER_FLARE];
} FLARE_DEF;

char* flareTextureNames[NUMBER_OF_TEXTURES] = { "crcl", "flar", "hxgn", "ring", "sun" };
