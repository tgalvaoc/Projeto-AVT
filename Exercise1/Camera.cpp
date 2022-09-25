#include <vector>

using namespace std;

class Camera {
	float pos[3];
	float target[3];

	static vector<Camera> buildCameras();
};
