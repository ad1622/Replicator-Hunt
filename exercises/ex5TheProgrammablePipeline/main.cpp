#include <iostream>
#include <cstdlib>

#include <oogl/glutIncludes.h>

#include <oogl/gl_error.h>
#include <oogl/Model.h>
#include <oogl/Image.h>
#include <oogl/Texture.h>
#include <oogl/GLSLShader.h>
#include <oogl/GLSLProgram.h>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>

#include <sstream>
#include <stdexcept>

#define M_PI 3.1415926535897932384626433832795
#define PLAYER_SPEED 0.25

int windowWidth, windowHeight;

bool leftMouseButtonDown = false;
int mousePosX = 0, mousePosY = 0;
// rotX = neigung, rotY = drehung
float rotationX = 0, rotationY = 0;
//spielerkoordinaten
float eyeX = 0, eyeZ = 0;

oogl::Model *vader = NULL;
oogl::Model *vaderCape = NULL;

oogl::Texture* tex1 = NULL;
oogl::Texture* tex2 = NULL;

GLuint simpleshaderprogram;
GLuint phongshaderprogram;

bool moveLight = true;	//move light
//bool useSimpleShader = true; //use simple shader
float materialShininess = 64.0;   //material shininess

//rotation of sun
float angle = 0;

//replicator animation stuff
float repl_ani_state = 0;
bool repl_ani_dir_up = true;
float repl_side_ud_left = 0;
float repl_side_ud_right = 0;
float repl_side_bf = 0;

float wobbletime = 0;

//gamestats
int score = 0;
int healthpoints = 0;


void cleanup();

void initSimpleShader()
{
	//create program
	simpleshaderprogram = glCreateProgram();

	//compile shaders
	GLuint vertexshader = oogl::loadShader("shader/simpleshader.vert",GL_VERTEX_SHADER);
	GLuint fragmentshader = oogl::loadShader("shader/simpleshader.frag",GL_FRAGMENT_SHADER);

	//attach shaders
	glAttachShader(simpleshaderprogram, vertexshader);
	glAttachShader(simpleshaderprogram, fragmentshader);

	//link program
	GLint status;
	glLinkProgram(simpleshaderprogram);

	//check for link errors
	glGetProgramiv(simpleshaderprogram, GL_LINK_STATUS, &status);
	if (!status) {
		std::string error = "can't link program: " + oogl::GLSLProgram::getInfoLog(simpleshaderprogram);
		LOG_ERROR << error << std::endl;
		throw std::runtime_error(error);
	} else {
		LOG_DEBUG << "linked program - no errors" << std::endl;
	}
}

void initPhongShader()
{
	//create program
	phongshaderprogram = glCreateProgram();

	//compile shaders
	GLuint vertexshader = oogl::loadShader("shader/phong.vert",GL_VERTEX_SHADER);
	GLuint fragmentshader = oogl::loadShader("shader/phong.frag",GL_FRAGMENT_SHADER);

	//attach shaders
	glAttachShader(phongshaderprogram, vertexshader);
	glAttachShader(phongshaderprogram, fragmentshader);

	//link program
	GLint status;
	glLinkProgram(phongshaderprogram);

	//check for link errors
	glGetProgramiv(phongshaderprogram, GL_LINK_STATUS, &status);

	if (!status) {
		std::string error = "can't link program: " + oogl::GLSLProgram::getInfoLog(phongshaderprogram);
		LOG_ERROR << error << std::endl;
		throw std::runtime_error(error);
	} else {
		LOG_DEBUG << "linked program - no errors" << std::endl;
	}
}

void init() {
	atexit(cleanup);

	//load vader model, we use LOAD_SET_SMOOTHING_GROUP option to overwrite the (wrong?) smoothing group values in this model
	vader = oogl::loadModel("models/vader/vader.3ds", oogl::Model::LOAD_NO_NORMALIZATION | oogl::Model::LOAD_SET_SMOOTHING_GROUP);
	vaderCape = oogl::loadModel("models/vader/vaderCape.3ds", oogl::Model::LOAD_NO_NORMALIZATION | oogl::Model::LOAD_SET_SMOOTHING_GROUP);

	initSimpleShader();
	initPhongShader();

	tex1 = oogl::loadTexture("models/lava.jpg");
	tex2 = oogl::loadTexture("models/ReplBlock.jpg");

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);

	//sky color = blue
	glClearColor(0.4f, 0.4f, 1.0f, 1.0f);
}

void cleanup() {
	delete vader;
	delete vaderCape;

	delete tex1;

	if(simpleshaderprogram > 0)
		glDeleteProgram(simpleshaderprogram);

	if(phongshaderprogram > 0)
		glDeleteProgram(phongshaderprogram);
}


void renderLightSphere(float lcolor[]) {
	glDisable(GL_LIGHTING); //temporary disable lightning

	//set color for sphere to the given light color
	glColor4fv(lcolor);

	glutSolidSphere(.03,10,10);

	glEnable(GL_LIGHTING);
}

void setLights() {
	glEnable(GL_LIGHTING); // Enable lighting
	glEnable(GL_LIGHT0);

	//setup light0 (SUN)
	//float ambient[] = {0.4f, 0.4f, 0.4f, 1.0f};
	float ambient[] = { 0.05f, 0.05f, 0.05f, 1.00f };
	
	//float diffuse[] = { 0, 0, 0, 0 };
	//float specular[] = { 0, 0, 0, 0.0f };
	float diffuse[] = {1.0f, 1.0f, 0.6f, 1.0f};
	float specular[] = {1.0f, 1.0f, 0.6f, 1.0f};
	float position[] = {0.0f, 0.0f, 0.0f, 1.0f};
	float spotdir[] = { 0.0f, -1.0f, 0.0f};

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	glPushMatrix();

	glTranslatef(32,32,32);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glScalef(50,50,50);
	renderLightSphere(diffuse);
	glPopMatrix();

	glEnable(GL_LIGHT1);

	//setup light1 rotating sun
	diffuse[0] = 0.8f;
	diffuse[1] = 0.8f;
	diffuse[2] = 1.0f;

	specular[0] = 0.8f;
	specular[1] = 0.8f;
	specular[2] = 1.0f;

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular);

	glPushMatrix();
	glTranslatef(32, 32, 32);

	//glRotatef(45, 0, 0, 1);
	glRotatef(angle, 0, 1, 0);
	glTranslatef(15, 0, 0);
	glLightfv(GL_LIGHT1, GL_POSITION, position);
	glScalef(40, 40, 40);
	renderLightSphere(diffuse);
	glPopMatrix();

	//setup light2 flashlight
	glEnable(GL_LIGHT2);
	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 80);
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spotdir);


	glLightfv(GL_LIGHT2, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, specular);

	glPushMatrix();
	glTranslatef(2, 1, 2);
	glLightfv(GL_LIGHT2, GL_POSITION, position);
	renderLightSphere(diffuse);
	glPopMatrix();

}
/*
void setCapeMaterial() {

	float zero[] = {0.0f, 0.0f, 0.0f, 1.0f};

	float ambient[] = {.1f,0,0, 1.0f};
	float diffuse[] = {.7f,0,0, 1.0f};
	float specular[] = {1,0,0, 1.0f};
	float emission[] = {.1f,0,0, 1.0f};

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);
}*/

void print(float x, float y, float r, float g, float b, void* font,const char *string, ...)
{
	glColor3f(r, g, b);
	glRasterPos2f(x, y);

	char text[256];
	va_list ap;

	if (string == NULL) return;

	va_start(ap, string);
	vsprintf(text, string, ap);
	va_end(ap);

	int len, i;
	len = (int)strlen(text);
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(font, text[i]);
	}
}

void drawHUD(){
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDepthMask(GL_FALSE);  // disable writes to Z-Buffer
	glDisable(GL_DEPTH_TEST);

	//draw here -1,-1 left bottom / 1,1 top right
	print(-1, 0.9, 1, 1, 1, GLUT_BITMAP_HELVETICA_18, "SCORE: %d", score);
	print(-1, 0.8, 1, 1, 1, GLUT_BITMAP_HELVETICA_18, "HP: %d", healthpoints);

	glEnable(GL_DEPTH_TEST); 
	glDepthMask(GL_TRUE);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void setReplMaterial() {

	float zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	float ambient[] = { .2f, 0.2, 0.2, 1.0f };
	float diffuse[] = { 0.65, 0.65, 0.65, 1.0f };
	float specular[] = { 0.65, 0.65, 0.65, 1.0f };
	float emission[] = { .1f, .1f, .1f, 1.0f };

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);
}

/*
void renderVader() {
	glPushMatrix();

	glScalef(1.5f,1.5f,1.5f);

	//enable normals rescaling as we apply (uniform) scaling to the model, which also affects the normals (use less efficient GL_NORMALIZE for non uniform scaling)
	glEnable(GL_RESCALE_NORMAL);

	//draw the model
	vader->render();

	setCapeMaterial();
	vaderCape->render(oogl::Model::RENDER_NO_MATERIALS);

	glDisable(GL_RESCALE_NORMAL);

	glPopMatrix();
}
*/

void renderFloor() {
	glPushMatrix();
	glEnable(GL_COLOR_MATERIAL); //use vertex colors instead of material colors
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE); //use the vertex color as diffuse color

	glEnable(GL_TEXTURE_2D); //enable texturing

	tex1->bind(5);

	//glTranslatef(0,-.93f,0);
	//glScalef(.8f,.8f,.8f);

	glBegin(GL_QUADS);
		glColor4f(1,1,1,1);
		glNormal3f(0,1,0); //specify the normal for the following vertices


		glTexCoord2f(0.0, 0.0);
		glVertex3f(0, 0, 0);

		glTexCoord2f(0.0, 64.0);
		glVertex3f(0, 0, 64.0);

		glTexCoord2f(64.0, 64.0);
		glVertex3f(64.0, 0, 64.0);

		glTexCoord2f(64.0, 0.0);
		glVertex3f(64.0, 0, 0);
	glEnd();
	
	tex1->unbind();

	//disable enabled features again
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_MATERIAL);

	glPopMatrix();
}

void renderCellStrip(int length, float width, float depth){

	float hinten, vorne, links, rechts, oben, unten;
	vorne = depth*0.5;
	hinten = -vorne;
	rechts = width*0.5;
	links = -rechts;
	oben = length;//*0.5;
	unten = 0;// -oben;

	glPushMatrix();
	glEnable(GL_COLOR_MATERIAL); //use vertex colors instead of material colors
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE); //use the vertex color as diffuse color
	glEnable(GL_TEXTURE_2D); //enable texturing

		tex2->bind(5);
		
		glColor4f(0.5, 0.5, 0.5, 1);

		//front:
		glBegin(GL_TRIANGLE_STRIP);
		glNormal3f(0, 0, width);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(links, unten, vorne);
		glTexCoord2f(0.0, length);
		glVertex3f(links, oben, vorne);
		glTexCoord2f(width, 0.0);
		glVertex3f(rechts, unten, vorne);
		glTexCoord2f(width, length);
		glVertex3f(rechts, oben, vorne);
		glEnd();

		
		//hinten:
		glBegin(GL_TRIANGLE_STRIP);
		glNormal3f(0, 0, -1);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rechts, unten, hinten);
		glTexCoord2f(0.0, length);
		glVertex3f(rechts, oben, hinten);
		glTexCoord2f(width, 0.0);
		glVertex3f(links, unten, hinten);
		glTexCoord2f(width, length);
		glVertex3f(links, oben, hinten);
		glEnd();

		
		//rechts
		glBegin(GL_TRIANGLE_STRIP);
		glNormal3f(1, 0, 0);
		glTexCoord2f(depth, 0.0);
		glVertex3f(rechts, unten, hinten);
		glTexCoord2f(depth, length);
		glVertex3f(rechts, oben, hinten);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rechts, unten, vorne);
		glTexCoord2f(0.0, length);
		glVertex3f(rechts, oben, vorne);
		glEnd();

		//links:
		glBegin(GL_TRIANGLE_STRIP);
		glNormal3f(-1, 0, 0);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(links, unten, hinten);
		glTexCoord2f(0.0, length);
		glVertex3f(links, oben, hinten);
		glTexCoord2f(depth, 0.0);
		glVertex3f(links, unten, vorne);
		glTexCoord2f(depth, length);
		glVertex3f(links, oben, vorne);
		glEnd();

		//oben:
		glBegin(GL_TRIANGLE_STRIP);
		glNormal3f(0, 1, 0);
		glTexCoord2f(0.0, depth);
		glVertex3f(links, oben, hinten);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(links, oben, vorne);
		glTexCoord2f(width, depth);
		glVertex3f(rechts, oben, hinten);
		glTexCoord2f(width, 0.0);
		glVertex3f(rechts, oben, vorne);
		glEnd();
		
		//unten:
		glBegin(GL_TRIANGLE_STRIP);
		glNormal3f(0, -1, 0);
		glTexCoord2f(0.0, depth);
		glVertex3f(links, unten, hinten);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(links, unten, vorne);
		glTexCoord2f(width, depth);
		glVertex3f(rechts, unten, hinten);
		glTexCoord2f(width, 0.0);
		glVertex3f(rechts, unten, vorne);
		glEnd();

	tex2->unbind();

	//disable enabled features again
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_MATERIAL);

	glPopMatrix();
}

//alpha degree first angle, beta second ....
//a = 110, b = 70, c = 105 default values
void renderReplicatorLeg(int alpha, int beta, int gamma){
	glPushMatrix();
	glRotatef(-90, 0, 0, 1);
	renderCellStrip(3, 1, 1);
	
	glRotatef(alpha, 0, 0, 1);
	glTranslatef(0, 0, 0);
	renderCellStrip(2, 1, 1);

	glTranslatef(0, 2, 0);
	glRotatef(180-beta, 0, 0, 1);
	renderCellStrip(6, 1, 1);

	glTranslatef(0, 6, 0);
	glRotatef(180-gamma, 0, 0, 1);
	renderCellStrip(3, 1, 1);

	//disable enabled features again

	glPopMatrix();
}

void renderReplicator(float x, float y, int rot){
	glPushMatrix();
	setReplMaterial();
	glTranslatef(x, 0.25, y);
	glRotatef(rot,0,1,0);
	glScalef(.05, .05f, .05f);
	renderCellStrip(3, 5, 7);

	//side left
	glPushMatrix();
	glRotatef(repl_side_bf, 0, 1, 0);
	glRotatef(repl_side_ud_left, 0, 0, 1);
	glTranslatef(-4, 0, 1);
	renderReplicatorLeg(110, 70, 105);
	glPopMatrix();


	//side right
	glPushMatrix();
	glRotatef(180 + repl_side_bf, 0, 1, 0);
	glRotatef(repl_side_ud_right, 0, 0, 1);
	glTranslatef(-4, 0, -1);
	renderReplicatorLeg(110, 70, 105);
	glPopMatrix();


	//front left
	glPushMatrix();
	glTranslatef(1, 0, -5);
	glRotatef(-90-10, 0, 1, 0);
	renderReplicatorLeg(110 + 10 * -repl_ani_state, 70 + 15 * -repl_ani_state, 130 + 20 * -repl_ani_state);
	glPopMatrix();


	//front right
	glPushMatrix();
	glTranslatef(-1, 0,-5);
	glRotatef(-90+10, 0, 1, 0);
	renderReplicatorLeg(110 + 10 * repl_ani_state, 70 + 15 * repl_ani_state, 130 + 20 * repl_ani_state);
	glPopMatrix();

	glPopMatrix();
}

/**
 * called when a frame should be rendered
 */
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// set vantage point
	gluLookAt(0.01, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	
	// add rotation
	//rotation z(neigung)
	glRotatef(rotationX, 0.0f, 0.0f, 1.0f);
	//rotation y(drehung)
	glRotatef(rotationY, 0.0f, 1.0f, 0.0f );

	//translate scene
	glTranslatef(eyeX, -1.5, eyeZ);

	// Map from (0,0,0) to (64,0,64) coordinates, 1 equals about 1m

	// set specific light properties
	setLights();
	
	glUseProgram(phongshaderprogram);

	//renderVader();

	glUniform4f(glGetUniformLocation(phongshaderprogram, "mycolor"), 1.0, 1.0, 1.0, 1.0);
	glUniform1i(glGetUniformLocation(phongshaderprogram, "mytexture"), 5);
	
	renderReplicator(4, 4, 0);
	renderReplicator(3, 5, 35);
	renderReplicator(4, 3, 12);
	renderReplicator(3, 2, 97);
	renderReplicator(5, 4, 182);
	renderReplicator(3, 4, 310);
	renderReplicator(5, 5, 250);

	renderFloor();
	glUseProgram(0);

	drawHUD();

	//render floor with our simple shader (light is ignored)
	//if(useSimpleShader)
	//{
	//	glUseProgram(simpleshaderprogram);

	//	/*
	//	Examples for setting uniform parameters in a shader program:

	//	Get Index of uniform parameter:
	//		glGetUniformLocation(program,"name")

	//	set uniform float:
	//		glUniform1f(glGetUniformLocation(program,"name") ,1.0);

	//	set uniform int (or texture unit):
	//		glUniform1i(glGetUniformLocation(program,"name") , 1);

	//	set uniform vec4 with 4 values:
	//		glUniform4f(glGetUniformLocation(program,"name") ,1.0,1.0,1.0,1.0);

	//	*/

	//	glUniform4f(glGetUniformLocation(simpleshaderprogram,"mycolor"),1.0,0.0,0.0,1.0);
	//	glUniform1i(glGetUniformLocation(simpleshaderprogram,"mytexture"),5);
	//	glUniform1f(glGetUniformLocation(simpleshaderprogram,"wobbletime"),wobbletime);
	//}

	////renderFloor();

	//if(useSimpleShader)
	//	glUseProgram(0);
	LOG_GL_ERRORS();
	glutSwapBuffers();
}

/**
 * called when the window was resized
 * @param w new window width in pixel
 * @param h new window height in pixel
 */
void reshape(int w, int h) {
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, ((float)w)/h,1, 300);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/**
 * called when nothing else needs to be done
 */
void idle() {
	//force a redisplay
	glutPostRedisplay();
}

/**
 * called when the user pressed a key
 * @param key ASCII character code
 * @param x mouse x position in pixel relative to the window, when the key was pressed
 * @param y mouse y position in pixel relative to the window, when the key was pressed
 */
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27: //27=esc
		exit(0);
		break;
	case 's':
		break;
	case 'l':
		break;
	case 'a':
		break;
	}
	glutPostRedisplay();
}

void special(int key, int x, int y) {
	switch(key) {
	case GLUT_KEY_UP :
		eyeZ -= PLAYER_SPEED*sinf(rotationY*M_PI / 180.0);
		eyeX -= PLAYER_SPEED*cosf(rotationY*M_PI / 180.0);
		break;
	case GLUT_KEY_DOWN :
		eyeZ += PLAYER_SPEED*sinf(rotationY*M_PI / 180.0);
		eyeX += PLAYER_SPEED*cosf(rotationY*M_PI / 180.0);
		break;
	case GLUT_KEY_LEFT:
		eyeZ -= PLAYER_SPEED*sinf((rotationY - 90.0)*M_PI / 180.0);
		eyeX -= PLAYER_SPEED*cosf((rotationY - 90.0)*M_PI / 180.0);
		break;
	case GLUT_KEY_RIGHT:
		eyeZ -= PLAYER_SPEED*sinf((rotationY + 90.0)*M_PI / 180.0);
		eyeX -= PLAYER_SPEED*cosf((rotationY + 90.0)*M_PI / 180.0);
		break;
	}
}

/**
 * called when the user pressed or released a mouse key
 * @param button which mouse button was pressed, one of GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON and GLUT_RIGHT_BUTTON
 * @param state button pressed (GLUT_DOWN) or released(GLUT_UP)
 * @param x mouse x position in pixel relative to the window, when the mouse button was pressed
 * @param y mouse y position in pixel relative to the window, when the mouse button was pressed
 */
void mouse(int button, int state, int x, int y) {
	leftMouseButtonDown = (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN);
	//mousePosX = x;
	//mousePosY = y;

	//glutPostRedisplay();
}

/**
 * called when the mouse moves (and no buttons pressed)
 * @param x mouse x position in pixel relative to the window
 * @param y mouse x position in pixel relative to the window
 */
void passiveMouseMotion(int x, int y) {
	//if (leftMouseButtonDown) {
		rotationY -= mousePosX - x;
		rotationX -= mousePosY - y;
		if (rotationX >= 90) rotationX = 90;
		else if (rotationX <= -90) rotationX = -90;
		mousePosX = x;
		mousePosY = y;
	//}
}

void update(int value) {
	if(moveLight)
	{
		angle += 1.0f;
		if (angle > 360)
			angle -= 360;
	}
	wobbletime += 0.1f;

	if (repl_ani_dir_up){
		repl_ani_state += 0.2f;
		if (repl_ani_state > 1){
			repl_ani_state = 1.0f;
			repl_ani_dir_up = false;
		}
	}
	else{
		repl_ani_state -= 0.2f;
		if (repl_ani_state < -1){
			repl_ani_state = -1.0f;
			repl_ani_dir_up = true;
		}
	}

	//calculate angels
	repl_side_bf = 25 * repl_ani_state;
	repl_side_ud_left = 10 * (!repl_ani_dir_up ? abs(repl_ani_state) : 1) -10;
	repl_side_ud_right = 10 * (repl_ani_dir_up ? abs(repl_ani_state) : 1) - 10;

	glutPostRedisplay();

	//call update method in 25ms
	glutTimerFunc(25, update, 0);
}


int setupGLUT(int argc, char** argv) {
	glutInit(&argc, argv);
	// glutInitContextVersion(3, 0);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);

	int windowId = glutCreateWindow("Replicator Hunt");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mouse);
	//glutMotionFunc(mouseMotion);
	glutPassiveMotionFunc(passiveMouseMotion);

	glutTimerFunc(25, update, 0);

	return windowId;
}

int main(int argc, char** argv) {
	setupGLUT(argc, argv);

	oogl::dumpGLInfos();

	init();

	glutMainLoop();

	return 0;
}

/*Additional Examples for setting shader parameters

		set uniform vec4 with vector variable:
			float vector[] = {1, 1, 1, 1};
			glUniform4fv(glGetUniformLocation(program,"name") ,1,vector);

			or

			glm::vec4 gvector = glm::vec4(1.0f,1.0f,1.0f,1.0f);
			glUniform4fv(glGetUniformLocation(program,"name") ,1,glm::value_ptr(gvector));

		set uniform mat4 with matrix variable:
			float matrix[] = {1, 0, 0, 0,
							0, 1, 0, 0,
							0, 0, 1, 0,
							0, 0, 0, 1};
			lUniformMatrix4fv(glGetUniformLocation(program,"name") ,1,false,matrix);

			or

			glm::mat4 gmatrix = glm::mat4(1.0f,0.0f,0.0f,0.0f,
										0.0f,1.0f,0.0f,0.0f,
										0.0f,0.0f,1.0f,0.0f,
										0.0f,0.0f,0.0f,1.0f);
			glUniformMatrix4fv(glGetUniformLocation(program,"name") ,1,false,glm::value_ptr(gmatrix));
*/
