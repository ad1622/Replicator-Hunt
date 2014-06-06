/*
 * BumpMappingScene.h
 *
 *  Created on: 21.05.2011
 *      Author: sam
 */

#ifndef EXERCISESCENES_H_
#define EXERCISESCENES_H_

#include "Scene.h"
#include <oogl/gl_error.h>
#include <oogl/Model.h>
#include <oogl/Image.h>
#include <oogl/Texture.h>
#include <oogl/GLSLShader.h>
#include <oogl/GLSLProgram.h>
#include <glm/gtc/type_ptr.hpp>

class ShadowMappingScene: public Scene {
public:
	ShadowMappingScene();
	virtual ~ShadowMappingScene();

	virtual void init();
	virtual void cleanup();
	virtual void renderScene();

private:
	oogl::GLSLProgram *shadowprogram;

	GLuint depthMap;
	GLuint depthMapFBO;

	GLuint testFBO;

	oogl::Model *vader;
	oogl::Model *vaderCape;

	glm::mat4 lightprojmatrix;
	glm::mat4 lightcameramatrix;
	glm::mat4 bias;

	void initDepthMap();
	void setLights();
	void renderLightSphere(float lcolor[]);
	void setCapeMaterial();
	void renderVader();
	void renderFloor();
	void generateShadowMap(glm::mat4 inversecameramatrix);
};

class ThreeDTextureScene: public Scene {
public:
	ThreeDTextureScene();
	virtual ~ThreeDTextureScene();

	virtual void init();
	virtual void cleanup();
	virtual void renderScene();
	void special(int key, int x, int y);

private:
	oogl::GLSLProgram *program;
	GLuint textureId;
	float texZCoord;
};

class EnvMappingScene: public Scene {
public:
	EnvMappingScene();
	virtual ~EnvMappingScene();

	virtual void init();
	virtual void cleanup();
	virtual void renderScene();
	virtual void keyboard(unsigned char key, int x, int y);

private:
	oogl::GLSLProgram *program;
	oogl::Model *model;
	GLuint textureId;
	GLboolean mipmapping;
	GLboolean anisotopicmax;
};

class MultiTexturingScene: public Scene {
public:
	MultiTexturingScene();
	virtual ~MultiTexturingScene();

	virtual void init();
	virtual void cleanup();
	virtual void renderScene();

private:
	oogl::GLSLProgram *program;
	oogl::Texture *colorTexture;
	oogl::Texture *normalTexture;
	oogl::Texture *specularTexture;
	oogl::Texture *displacementTexture;

	GLuint terrainList;

	void generateTerrain();
	void setLights();
};

#endif /* EXERCISESCENES_H_ */
