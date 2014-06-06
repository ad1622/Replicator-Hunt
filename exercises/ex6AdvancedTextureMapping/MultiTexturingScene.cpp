/*
 * MultiTexturingScene.cpp
 *
 *  Created on: 21.05.2011
 *      Author: sam
 */

#include "ExerciseScenes.h"

MultiTexturingScene::MultiTexturingScene() {
}

MultiTexturingScene::~MultiTexturingScene() {
}


void MultiTexturingScene::init() {
	Scene::init();

	program = oogl::GLSLProgram::create("shader/multitexturing.vert", "shader/multitexturing.frag");

	colorTexture = oogl::loadTexture("models/terrain/colormap.jpg");
	normalTexture = oogl::loadTexture("models/terrain/normalmap.jpg");
	specularTexture = oogl::loadTexture("models/terrain/specmap.jpg");
	displacementTexture = oogl::loadTexture("models/terrain/displacementmap.jpg");

	generateTerrain();
}

void setTerrainMaterial() {

	float zero[] = {0.0f, 0.0f, 0.0f, 1.0f};

	float ambient[] = {0.1f,0.1f,0.1f, 1.0f};
	float diffuse[] = {1.0f,1.0f,1.0f, 1.0f};
	float specular[] = {0.5f,0.5f,0.5f, 1.0f};
	float emission[] = {0,0,0, 1.0f};

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10);
}

void MultiTexturingScene::generateTerrain() {
	terrainList = glGenLists(1);
	glNewList(terrainList, GL_COMPILE);

	setTerrainMaterial();

	const int SIZE = 128;
	GLfloat mesh[SIZE][SIZE][3];
	// create SIZExSIZE mesh from -1.0 to 1.0
	float invSIZE10 = 2.0f / SIZE;
	for (int x = 0; x < SIZE; x++) {
		for (int y = 0; y < SIZE; y++) {
			mesh[x][y][0] = -1.0f + (x * invSIZE10);
			mesh[x][y][1] = -1.0f + (y * invSIZE10);
			mesh[x][y][2] = 0.0f;
		}
	}

	GLuint tangentAttribute = glGetAttribLocation(program->getId(),"tangentVec");
	// call the function that contains the rendering commands
	float invSIZE = 1.0f/ SIZE;
	for (int x = 0; x < SIZE-1; ++x) {
		glBegin(GL_TRIANGLE_STRIP);
			glNormal3f(0,0,1);
			glVertexAttrib3f(tangentAttribute,1,0,0);
			for (int z = 0; z < SIZE-1; ++z) {
				glTexCoord2f(x * invSIZE, z * invSIZE);
				glVertex3f(mesh[x][z][0], mesh[x][z][1], mesh[x][z][2]);
				glTexCoord2f((x + 1) * invSIZE, z * invSIZE);
				glVertex3f(mesh[x + 1][z][0], mesh[x + 1][z][1], mesh[x + 1][z][2]);
			}
		glEnd();
	}

	glEndList();

	std::cout << "terrain generated" << std::endl;
}

void MultiTexturingScene::cleanup() {
	delete program;
	delete colorTexture;
	delete normalTexture;
	delete specularTexture;
	delete displacementTexture;

	glDeleteLists(terrainList,1);
}


void MultiTexturingScene::setLights() {
	glEnable(GL_LIGHTING); // Enable lighting
	glEnable(GL_LIGHT0);

	//setup light0
	float ambient[] = {1.0f, 1.0f, 1.0f, 1.0f};
	float diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
	float specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
	float position[] = {0.0f, 0.0f, 0.0f, 1.0f};

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	glPushMatrix();
	{
		//the light position is affected by the model view matrix, too
		
		glTranslatef(0,2,0);
		glRotatef(angle, 0,1,0);
		glTranslatef(2,0,0);

		glLightfv(GL_LIGHT0, GL_POSITION, position); //position gets multiplied with the current modelview matrix

		glDisable(GL_LIGHTING); //temporary disable lightning
		//set color for sphere to the given light color
		glColor4fv(diffuse);
		glutSolidSphere(.03,10,10);
		glEnable(GL_LIGHTING);
	}
	glPopMatrix();
}

void MultiTexturingScene::renderScene() {
	// camera view transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// set vantage point
	gluLookAt(0.0, 0.0,4.5, 0.0, 0.0, -1.0, 0.0, 1.0, 0.0);

	// add rotation / navigation
	glRotatef(rotationX, 1.0f, 0.0f, 0.0f );
	glRotatef(rotationY, 0.0f, 1.0f, 0.0f );

	// camera view transformation done

	setLights();

	glEnable(GL_TEXTURE_2D); //enable texturing
	colorTexture->bind(0);
	normalTexture->bind(1);
	specularTexture->bind(2);
	displacementTexture->bind(3);

	{
		glPushMatrix();

		glUseProgram(program->getId()); //enable shader
		glUniform1i(glGetUniformLocation(program->getId(),"texColor"),0);
		glUniform1i(glGetUniformLocation(program->getId(),"texNormal"),1);
		glUniform1i(glGetUniformLocation(program->getId(),"texSpecular"),2);
		glUniform1i(glGetUniformLocation(program->getId(),"texDisplacement"),3);

		glRotatef(-80,1,0,0);
		glRotatef(30,0,0,1);
		glScalef(1.5f,1.5f,1.5f);
		glCallList(terrainList);

		glUseProgram(0); //disable shader

		glPopMatrix();
	}

	displacementTexture->unbind();
	specularTexture->unbind();
	normalTexture->unbind();
	colorTexture->unbind();
	glDisable(GL_TEXTURE_2D);
}
