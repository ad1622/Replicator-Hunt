/*
 * EnvMappingScene.cpp
 *
 *  Created on: 21.05.2011
 *      Author: sam
 */

#include "ExerciseScenes.h"

EnvMappingScene::EnvMappingScene() {
}

EnvMappingScene::~EnvMappingScene() {
}


void EnvMappingScene::init() {
	Scene::init();

	model = oogl::loadModel("models/royalstarship.3ds", oogl::Model::LOAD_SET_SMOOTHING_GROUP);
	program = oogl::GLSLProgram::create("shader/envmapping.vert", "shader/envmapping.frag");


	glGenTextures(1, &textureId); //generate a new texture

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId); //bind our texture

	std::auto_ptr<oogl::Image> imageps(oogl::loadImage("models/skybox/Galaxy_RT.jpg"));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, imageps->getWidth(), imageps->getHeight(), 0, imageps->getFormat(), imageps->getType(), imageps->getData());

	std::auto_ptr<oogl::Image> imagenx(oogl::loadImage("models/skybox/Galaxy_LT.jpg"));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, imagenx->getWidth(), imagenx->getHeight(), 0, imagenx->getFormat(), imagenx->getType(), imagenx->getData());

	std::auto_ptr<oogl::Image> imagepy(oogl::loadImage("models/skybox/Galaxy_DN.jpg"));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, imagepy->getWidth(), imagepy->getHeight(), 0, imagepy->getFormat(), imagepy->getType(), imagepy->getData());

	std::auto_ptr<oogl::Image> imageny(oogl::loadImage("models/skybox/Galaxy_UP.jpg"));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, imageny->getWidth(), imageny->getHeight(), 0, imageny->getFormat(), imageny->getType(), imageny->getData());

	std::auto_ptr<oogl::Image> imagepz(oogl::loadImage("models/skybox/Galaxy_FT.jpg"));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, imagepz->getWidth(), imagepz->getHeight(), 0, imagepz->getFormat(), imagepz->getType(), imagepz->getData());

	std::auto_ptr<oogl::Image> imagenz(oogl::loadImage("models/skybox/Galaxy_BK.jpg"));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, imagenz->getWidth(), imagenz->getHeight(), 0, imagenz->getFormat(), imagenz->getType(), imagenz->getData());


	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0); //unbind again

	mipmapping = false;
	anisotopicmax = false;
}


void EnvMappingScene::keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'm': 
		mipmapping = !mipmapping;
		printf("mipmapping %s\n",(mipmapping?"enabled":"disabled"));
		break;
	case 'a': 
		anisotopicmax = !anisotopicmax;
		printf("anisotropic filtering %s\n",(anisotopicmax?"enabled":"disabled"));
		break;
	}
}

void EnvMappingScene::cleanup() {
	delete program;
	delete model;

	glDeleteTextures(1, &textureId);
}

void EnvMappingScene::renderScene() {
	// camera view transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// set vantage point
	gluLookAt(0.0, 0.0,4.5, 0.0, 0.0, -1.0, 0.0, 1.0, 0.0);

	// add rotation / navigation
	glRotatef(rotationX, 1.0f, 0.0f, 0.0f );
	glRotatef(rotationY, 0.0f, 1.0f, 0.0f );

	// camera view transformation done

	//get current model view matrix. It corresponds to the camera view matrix because no model transformations were done so far...
	glm::mat4 cameraviewmatrix;
	glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(cameraviewmatrix));
	//invert camera view matrix to enable eye -> world space transformations
	//and reduce it to a 3x3 matrix as we only need it to transform direction vectors
	//(we use an eye->world matrix in our example because we want to use these predefined transformation matrices of opengl, but there are better solutions for this...)
	glm::mat3 invcam = glm::mat3(glm::inverse(cameraviewmatrix));

	glEnable(GL_TEXTURE_CUBE_MAP); //enable texturing
	glActiveTexture(GL_TEXTURE4); //make the texture4 the active one
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

	//set mipmapping and anisotropic filtering options for texture
	if(mipmapping)
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	else
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if(anisotopicmax)
	{
		float max;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max);
		glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, max);
	}
	else
		glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);

	//render space ship using reflect vector for texture lookup
	{
		glPushMatrix();

		glUseProgram(program->getId()); //enable shader
/*----------2.4----------*/
		glUniform1i(glGetUniformLocation(program->getId(),"useReflection"),GL_TRUE);
		glUniformMatrix3fv(glGetUniformLocation(program->getId(),"eyetoworld") ,1,false,glm::value_ptr(invcam));

		glRotatef(90, 0,1,0);
		glRotatef(180, 0,0,1);
		glScalef(2,2,2);
		model->render();

		glUseProgram(GL_NONE); //disable shader

		glPopMatrix();
	}

	//render a sphere and use the view direction in world space for texture lookup to display the environment map around the viewer (there are more efficient solutions for this...)
	{
		//render sphere at the world center (normally a sky box should always be centered at the camera position and not the world center, however if we make the sphere big enough this should be ok in our case...)
		glPushMatrix();

		glUseProgram(program->getId()); //enable shader
/*----------2.4----------*/
		glUniform1i(glGetUniformLocation(program->getId(),"useReflection"),GL_FALSE);
		glUniformMatrix3fv(glGetUniformLocation(program->getId(),"eyetoworld") ,1,false,glm::value_ptr(invcam));

		glutSolidSphere(200.0, 30, 30);

		glUseProgram(GL_NONE); //disable shader

		glPopMatrix();
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);
	glDisable(GL_TEXTURE_CUBE_MAP);

}
