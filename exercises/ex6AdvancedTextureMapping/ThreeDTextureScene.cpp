/*
 * ThreeDTextureScene.cpp
 *
 *  Created on: 21.05.2011
 *      Author: sam
 */

#include "ExerciseScenes.h"

ThreeDTextureScene::ThreeDTextureScene() {
	texZCoord = 0;
}

ThreeDTextureScene::~ThreeDTextureScene() {

}

void ThreeDTextureScene::init() {
	Scene::init();

	glClearColor(0,0,1,1);

	program = oogl::GLSLProgram::create("shader/3dtexture.vert", "shader/3dtexture.frag");

	//load a image using the library and devil
	std::auto_ptr<oogl::Image> image(oogl::loadImage("models/bucky.dds"));
	glGenTextures(1, &textureId); //generate a new texture

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, textureId); //bind our texture

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//define the texture and load data
	glTexImage3D(GL_TEXTURE_3D,
		0, //mip map level 0..top
		GL_RGBA, //internal format of the data in memory
		image->getWidth(),
		image->getHeight(),
		image->getDepth(),
		0,//border width in pixels (can either be 1 or 0)
		image->getFormat(),	// Image format (i.e. RGB, RGBA, BGR etc.)
		image->getType(),// Image data type
		image->getData());// The actual image data itself

	glBindTexture(GL_TEXTURE_3D, 0); //unbind again
	
}

void ThreeDTextureScene::cleanup() {
	delete program;
	if(textureId > 0) //cleanup the texture
		glDeleteTextures(1, &textureId);
}

void ThreeDTextureScene::renderScene() {
	// camera view transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// set vantage point
	gluLookAt(0.0, 0.0,4.5, 0.0, 0.0, -1.0, 0.0, 1.0, 0.0);

	// add rotation / navigation
	glRotatef(rotationX, 1.0f, 0.0f, 0.0f );
	glRotatef(rotationY, 0.0f, 1.0f, 0.0f );

	// camera view transformation done

	glEnable(GL_TEXTURE_3D); //enable (3D) texturing
	glActiveTexture(GL_TEXTURE0); //make the texture0 the active one
	glBindTexture(GL_TEXTURE_3D, textureId);

	glUseProgram(program->getId()); //enable shader
	glUniform1i(glGetUniformLocation(program->getId(),"tex"),0);
	glUniform1f(glGetUniformLocation(program->getId(),"texZCoord"),texZCoord);

	glRotatef(90,1,1,1);
	glScalef(1.5,1.5,1.5);

	glBegin(GL_QUADS);
		glColor4f(1,1,1,1);
		glNormal3f(0,1,0);

		glTexCoord2f(0.0, 0.0);
		glVertex3f(-1.0, 0, -1.0);

		glTexCoord2f(0.0, 1.0);
		glVertex3f(-1.0, 0, 1.0);

		glTexCoord2f(1.0, 1.0);
		glVertex3f(1.0, 0, 1.0);

		glTexCoord2f(1.0, 0.0);
		glVertex3f(1.0, 0, -1.0);
	glEnd();

	glUseProgram(GL_NONE); //disable shader

	glBindTexture(GL_TEXTURE_3D, GL_NONE);
	glDisable(GL_TEXTURE_3D);

}

void ThreeDTextureScene::special(int key, int x, int y) {
	switch(key) {
	case GLUT_KEY_UP:
		if(texZCoord < 1)
			texZCoord += 0.01f;
		break;
	case GLUT_KEY_DOWN:
		if(texZCoord > 0)
			texZCoord -= 0.01f;
		break;
	}
	glutPostRedisplay();
}
