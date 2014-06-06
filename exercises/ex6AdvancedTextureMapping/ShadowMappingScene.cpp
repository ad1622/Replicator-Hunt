/*
 * ShadowMappingScene.cpp
 *
 *  Created on: 21.05.2011
 *      Author: sam
 */

#include "ExerciseScenes.h"

ShadowMappingScene::ShadowMappingScene() {
	//bias matrix transforms clip coordinates (-1 to 1) to texture coordinates (0 to 1)
	bias = glm::mat4( 	0.5f, 0.0f, 0.0f, 0.0f,
						0.0f, 0.5f, 0.0f, 0.0f,
						0.0f, 0.0f, 0.5f, 0.0f,
						0.5f, 0.5f, 0.5f, 1.0f );
}

ShadowMappingScene::~ShadowMappingScene() {

}

void ShadowMappingScene::init() {
	Scene::init();

	//load vader model, we use LOAD_SET_SMOOTHING_GROUP option to overwrite the (wrong?) smoothing group values in this model
	vader = oogl::loadModel("models/vader/vader.3ds", oogl::Model::LOAD_NO_NORMALIZATION | oogl::Model::LOAD_SET_SMOOTHING_GROUP);
	vaderCape = oogl::loadModel("models/vader/vaderCape.3ds", oogl::Model::LOAD_NO_NORMALIZATION | oogl::Model::LOAD_SET_SMOOTHING_GROUP);

	initDepthMap();

	shadowprogram = oogl::GLSLProgram::create("shader/shadow.vert", "shader/shadow.frag");
}


void ShadowMappingScene::initDepthMap() {
	//init depthmap just like a normal texture, but specify that it's used as depth component
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 512, 512, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//set the depth compare mode that should be used for our shadow calculation
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

	//init a color texture that we use for debugging the output of our light render pass (not necessary for depth map calculation)
	glGenTextures(1, &testFBO);
	glBindTexture(GL_TEXTURE_2D, testFBO);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//use a frame buffer for the light view, here it is initialized
	glGenFramebuffers(1,&depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	//add our textures as so called attachments
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap,0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, testFBO,0);

	glDrawBuffer(GL_COLOR_ATTACHMENT0); //set our color texture in GL_COLOR_ATTACHMENT0 as drawbuffer for rendering (should be the default setting), use GL_NONE to disable any color output

	glBindFramebuffer(GL_FRAMEBUFFER, 0);	//for now use screen framebuffer
}

void ShadowMappingScene::cleanup() {
	delete vader;
	delete vaderCape;
	delete shadowprogram;

	glDeleteTextures(1, &depthMap);
	glDeleteTextures(1, &testFBO);
	glDeleteFramebuffers(1, &depthMapFBO);
}


void ShadowMappingScene::setLights() {
	glEnable(GL_LIGHTING); // Enable lighting
	glEnable(GL_LIGHT0);

	//setup light0
	float ambient[] = {1.0f, 1.0f, 1.0f, 1.0f};
	float diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
	float specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
	float position[] = {0.0f, 0.0f, 0.0f, 1.0f};
	float dir[] = {-1.0f, 0.0f, 0.0f}; //set light direction in the opposit direction of translation to make the light facing to the center

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	glPushMatrix();
	//the light position is affected by the model view matrix, too
	glTranslatef(3.0,0,0);
	glRotatef(angle, 0,1,0);
	glRotatef(45, 0,0,1);
	glTranslatef(2.8f,0,0); //translation in the opposit direction of the light "viewing" direction

	//set light position and direction
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);

	renderLightSphere(diffuse);

	//render line to see light direction
	glBegin(GL_LINES);
	glVertex3f(0,0, 0);
	glVertex3f(dir[0], dir[1], dir[2]);
	glEnd();

	glPopMatrix();
}

void ShadowMappingScene::renderLightSphere(float lcolor[]) {
	glDisable(GL_LIGHTING); //temporary disable lightning

	//set color for sphere to the given light color
	glColor4fv(lcolor);

	glutSolidSphere(.03,10,10);

	glEnable(GL_LIGHTING);
}

void ShadowMappingScene::setCapeMaterial() {

	float zero[] = {0.0f, 0.0f, 0.0f, 1.0f};

	float ambient[] = {.1f,0,0, 1.0f};
	float diffuse[] = {.7f,0,0, 1.0f};
	float specular[] = {1,0,0, 1.0f};
	float emission[] = {.1f,0,0, 1.0f};

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64);
}


void ShadowMappingScene::renderVader() {
	glPushMatrix();

	glTranslatef(3.0,0,0); //move Vader a bit off center to make wrong light computation to fail

	glScalef(1.5f,1.5f,1.5f);

	//enable normals rescaling as we apply (uniform) scaling to the model, which also affects the normals (use less efficent GL_NORMALIZE for non uniform scaling)
	glEnable(GL_RESCALE_NORMAL);

	//draw the model
	vader->render();

	setCapeMaterial();
	vaderCape->render(oogl::Model::RENDER_NO_MATERIALS);

	glDisable(GL_RESCALE_NORMAL);

	glPopMatrix();
}

void ShadowMappingScene::renderFloor() {
	glPushMatrix();
	glEnable(GL_COLOR_MATERIAL); //use vertex colors instead of material colors
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE); //use the vertex color as diffuse color

	//we only want diffuse lighting for the floor so we set all other material properties to zero
	float zero[] = {0.0f, 0.0f, 0.0f, 1.0f};
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, zero);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64);

	glTranslatef(2.0,-.93f,0);
	glScalef(5,5,5);

	glBegin(GL_QUADS);
		glColor4f(1,1,1,1);
		glNormal3f(0,1,0); //specify the normal for the following vertices


		glTexCoord2f(0.0, 0.0);
		glVertex3f(-1.0, 0, -1.0);

		glTexCoord2f(0.0, 1.0);
		glVertex3f(-1.0, 0, 1.0);

		glTexCoord2f(1.0, 1.0);
		glVertex3f(1.0, 0, 1.0);

		glTexCoord2f(1.0, 0.0);
		glVertex3f(1.0, 0, -1.0);
	glEnd();

	//disable enabled features again
	glDisable(GL_COLOR_MATERIAL);

	glPopMatrix();
}

void ShadowMappingScene::generateShadowMap(glm::mat4 inversecameramatrix)
{
	//save all the parameters we are going to change
	glPushAttrib(GL_VIEWPORT_BIT);
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();

	//first render pass, draw from lights point of view

	//bind FBO
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	//clear frame buffer (depthMapFBO)
	glClearColor(1.0,0,0,0.0); //set clear color to red for testing
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0,0,0,0); //change it back to black

	//set Viewport to  size of depth map texture
	glViewport(0, 0, 512, 512);

	//setup light camera
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective(50, 1.0f ,0.1f, 100);

	//get the light projection matrix as we need it in the shader
	glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(lightprojmatrix));

	//now setup light camera position and direction
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//get light position (eye space)
	glm::vec4 position;
	glGetLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(position));
	//get light direction (eye space)
	glm::vec4 dir;
	glGetLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, glm::value_ptr(dir));

	//light position and direction are in eye coordinates of our "screen"-camera so multiply with inverse camera matrix to get world coordinates
	glm::vec4 worldlightpos = inversecameramatrix*position;
	glm::vec4 worldlightdir = inversecameramatrix*dir;

	//setup light camera position and direction (use gluLookAt(...) with worldlightpos and worldlightdir)

/*----------3.1----------*/

	//get the light camera matrix as we need it in the shader
	glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(lightcameramatrix));

	//draw the part of the scene that's supposed to throw shadows,
	//we do not need to enable any shaders for this pass

/*----------3.2----------*/

	//unbind depthMap FBO, use screen frame buffer again
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//restore previous parameters
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
	glPopAttrib();

}

void ShadowMappingScene::renderScene() {
	// camera view transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// set vantage point
	gluLookAt(3.0, 0.0,4.5, 3.0, 0.0, -1.0, 0.0, 1.0, 0.0);

	// add rotation / navigation
	glTranslatef(3,0,0);
	glRotatef(rotationX, 1.0f, 0.0f, 0.0f );
	glRotatef(rotationY, 0.0f, 1.0f, 0.0f );
	glTranslatef(-3,0,0);
	// camera view transformation done

	//get camera view matrix, required for light calculations
	glm::mat4 cameraviewmatrix;
	glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(cameraviewmatrix));
	glm::mat4 invcam = glm::inverse(cameraviewmatrix);

	// set specific light properties
	setLights();

	//generate depth map for shadows based on our light (first render pass)
	generateShadowMap(invcam);

	//continue rendering second pass

	//enable the shadowmap shader
	glUseProgram(shadowprogram->getId());

	//matrix to transform coordinates: eye -(invcam)-> world -(lightcameramatrix)-> light-eye -(lightprojmatrix)-> light-clip -(bias)-> texture-coordinates
	glm::mat4 eyetolightmatrix = bias*lightprojmatrix*lightcameramatrix*invcam;
	glUniformMatrix4fv(glGetUniformLocation(shadowprogram->getId(),"eyeToLightMatrix") ,1,false,glm::value_ptr(eyetolightmatrix));

	//enable and pass textures
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(glGetUniformLocation(shadowprogram->getId(),"depthMap"),1);
	glUniform1i(glGetUniformLocation(shadowprogram->getId(),"objectTexture"),0);

	//render vader (has no texture)
	glUniform1i(glGetUniformLocation(shadowprogram->getId(),"enableObjectTexture"),0);
	renderVader();

	//bind color light view texture to show it on the floor (to see if the setup works)
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, testFBO);

	//render floor (with our "test" texture)
	glUniform1i(glGetUniformLocation(shadowprogram->getId(),"enableObjectTexture"),1);
	renderFloor();

	//disable textures
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//disable shader
	glUseProgram(0);
}
