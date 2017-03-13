#ifndef AMBIENT_OCCLUSION_H
#define AMBIENT_OCCLUSION_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"

#include <random>

class AmbientOcclusion {
public:

	AmbientOcclusion();

	~AmbientOcclusion();

	void createHemisphere();

	void bindForScreenDepthPass();

	void bindForDraw();

private:

	// Local copies of screen width and height
	// TODO(rgarmsen2295): Need to re-init if screen resize occurs
	int screenWidth;
	int screenHeight;

	// FBO and texture used to get the depth map of the scene
	GLuint screenDepthFBO;
	GLuint screenDepthMap;

	GLuint ssaoFBO;

	GLuint ssaoColorBuffer;

	std::vector<glm::vec3> ssaoKernel;

};

#endif
