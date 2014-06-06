#include <iostream>
#include <cstdlib>

#include <oogl/glutIncludes.h>

#include <oogl/gl_error.h>
#include <oogl/Model.h>
#include <oogl/Texture2D.h>

#include "accum.h"
#include "jitter.h"


oogl::Model* falcon = NULL;
oogl::Model* smoke = NULL;

int windowWidth, windowHeight;

bool leftMouseButtonDown = false;
int mousePosX = 0, mousePosY = 0;
float rotationX = 0, rotationY = 0;

enum AntiAliasingMode {
	NONE=0, ACCUMLUTION_BUFFER=1, SMOOTH=2, MULTISAMPLING=3
};

AntiAliasingMode antiAliasingMode = NONE;

bool isVisualizeDepth = false;

float opacity = 0.5f;
int selectedBlendFunc = 0; //0..5
int selectedBlendEquation = 0; //0..4

oogl::Texture2D* depthTexture = NULL;

void cleanup();
void initDepthTexture();

void init() {
	atexit(cleanup);

	falcon = oogl::loadModel("models/falcon/Falcon.3ds", oogl::Model::LOAD_SET_SMOOTHING_GROUP);
	smoke = oogl::loadModel("models/smoke.3ds", oogl::Model::LOAD_SET_SMOOTHING_GROUP);

	//enable depth test and let smaller values win the test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//simple lightning and shading more in a later exercise
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);

	depthTexture = oogl::Texture2D::createContainer(GL_DEPTH_COMPONENT);

}

void cleanup() {
	delete falcon;
	delete smoke;
	delete depthTexture;
}

void copyDepthToTexture() {
	depthTexture->bind(); //mark our depthTexture as the active one
	depthTexture->setSize(windowWidth, windowHeight);
	//we just want to draw the texture color nothing else
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	//copy the actual depth buffer and create a texture out of it
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, windowWidth, windowHeight, 0);
}

void visualizeDepth() {
	// copy depth buffer to texture
	copyDepthToTexture();

	// create an orthographic projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

	// reset modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// enable texturing
	glEnable(GL_TEXTURE_2D);
	// clear buffers and draw again
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//render the texture on a simple textured quad
	depthTexture->render();

	//disable texturing
	glDisable(GL_TEXTURE_2D);
}

void renderSmoke() {
	glPushMatrix();

	glEnable(GL_BLEND);

//	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); //how to scale the source and destination rgba values
//	glBlendEquation(GL_FUNC_ADD); //how to combine the source and destination scaled values
	switch(selectedBlendFunc) { //how to scale the source and destination rgba values
	case 0: glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); break;
	case 1: glBlendFunc(GL_SRC_ALPHA,GL_ONE); break;
	case 2: glBlendFunc(GL_SRC_ALPHA,GL_ZERO); break;
	case 3: glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA); break;
	case 4: glBlendFunc(GL_ONE_MINUS_DST_ALPHA,GL_DST_ALPHA); break;
	case 5: glBlendFunc(GL_ONE,GL_DST_ALPHA); break;
	}
	switch(selectedBlendEquation) { //how to combine the source and destination scaled values
	case 0: glBlendEquation(GL_FUNC_ADD); break;
	case 1: glBlendEquation(GL_FUNC_SUBTRACT); break;
	case 2: glBlendEquation(GL_FUNC_REVERSE_SUBTRACT); break;
	case 3: glBlendEquation(GL_MIN); break;
	case 4: glBlendEquation(GL_MAX); break;
	}

	glEnable(GL_COLOR_MATERIAL); //use vertex color instead as material colors

	glColor4f(0.8f, 0., 0., opacity); //alpha = opacity

	// make smoke sit on the falcon
	glTranslatef(-15,15,-2);
	glRotatef(-150, 1.0, 0.0, 0.0);
	glScalef(16,16,16);

	// draw smoke
	//render without materials and textures, next time
	smoke->render(oogl::Model::RENDER_NO_MATERIALS);

	glDisable(GL_BLEND); // disable blending
	glDisable(GL_COLOR_MATERIAL);

	glPopMatrix();
}

void renderFalcon() {
	glPushMatrix();

	glRotatef(-50, 1,0,0);
	glRotatef(60, 0,0,1);
	glScalef(30,30,30);

	falcon->render();

	glPopMatrix();
}

void renderLaserBeams() {
	glPushMatrix();

	glRotatef(-50, 1,0,0);
	glRotatef(60, 0,0,1);
	
	//store some settings that we going to modify
	glPushAttrib(GL_LINE_BIT | GL_POINT_BIT | GL_LIGHTING_BIT);

	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	
	glLineWidth(3); //width of a line in pixel
	glPointSize(15); //radi of a point in pixel
	
	glColor3f(1,0,0); //red

	glBegin(GL_POINTS);
		glVertex3f(0.5f,1.f,4.f);
		glVertex3f(0.5f,1.f,-6.f);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(0.5f,1.f,4.f);
		glVertex3f(1,-100,4.f);
		glVertex3f(0.5f, 1.f,-6.f);
		glVertex3f(1, -100, -6.f);
	glEnd();
	
	glPopAttrib(); //restore the old settings

	glPopMatrix();
}

void render() {
	renderFalcon();

	renderLaserBeams();
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//	gluPerspective(60.0, (float)windowWidth/windowHeight, 0.000001, 80.0);
	gluPerspective(60.0, (float)windowWidth/windowHeight, 20, 80.0);

	glMatrixMode(GL_MODELVIEW); // switch to modelview matrix
	glLoadIdentity();

	// set vantage point
	gluLookAt(0.0, 0.0, 50.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	
	// add rotation
	glRotatef(rotationX, 1.0f, 0.0f, 0.0f );
	glRotatef(rotationY, 0.0f, 1.0f, 0.0f );

	switch(antiAliasingMode) {
	case NONE:
		render();
		break;
	case ACCUMLUTION_BUFFER:
		//clear accumulation buffer
		glClear(GL_ACCUM_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION); //store our old projection matrix
		glPushMatrix();

		for (int jitter = 0; jitter < 8; ++jitter) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			//setup a different projection matrix
			accPerspective(60.0,		// field of view in degree
						(float)windowWidth/windowHeight,// aspect ratio
						20.0,			// near plane
						80.0,			// far plane
						j8[jitter].x,	// jitter point-x
						j8[jitter].y,	// jitter point-y
						0.0,			// eye-x
						0,				// eye-y
						50.0,			// eye-z
						1.0);			// focus
			glMatrixMode(GL_MODELVIEW); // switch back to modelview matrix

			render();

			//weight rendering
			glAccum(GL_ACCUM, 1.0/8);
		}
		
		glMatrixMode(GL_PROJECTION); //restore it
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW); // switch to modelview matrix

		//read back to color buffer
		glAccum(GL_RETURN, 1.0);
		break;
	case SMOOTH:
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POLYGON_SMOOTH);

		glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

		render();

		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POLYGON_SMOOTH);
		glDisable(GL_POINT_SMOOTH);
		break;
	case MULTISAMPLING:
		glEnable(GL_MULTISAMPLE);

		render();

		glDisable(GL_MULTISAMPLE);
		break;
	}

	renderSmoke();

	if (isVisualizeDepth)
		visualizeDepth(); // draw depth buffer content

	//check for opengl errors and log them
	LOG_GL_ERRORS();
	glutSwapBuffers();
}

/**
 * called when the window was resized
 * @param w new window width in pixel
 * @param h new window height in pixel
 */
void reshape(int w, int h) {
	windowWidth = w;
	windowHeight = h;

	glViewport(0, 0, w, h);
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
	case 'd': //toggle visualize depth and not
		isVisualizeDepth = !isVisualizeDepth;
		std::cout << "depth visualization: " << std::boolalpha << isVisualizeDepth << std::endl;
		break;
	case 'a': //switch between different antialiasing modes
		switch(antiAliasingMode) {
		case NONE:
			antiAliasingMode = ACCUMLUTION_BUFFER;
			std::cout << " antialiasing via " << "ACCUMLUTION_BUFFER" << std::endl;
			break;
		case ACCUMLUTION_BUFFER:
			antiAliasingMode = SMOOTH;
			std::cout << " antialiasing via "<< "SMOOTH" << std::endl;
			break;
		case SMOOTH:
			antiAliasingMode = MULTISAMPLING;
			std::cout << " antialiasing via "<< "MULTISAMPLING" << std::endl;
			break;
		case MULTISAMPLING:
			antiAliasingMode = NONE;
			std::cout << " no antialiasing" << std::endl;
			break;
		}
		
		break;

	}
	glutPostRedisplay();
}

/**
 * called when the user pressed a special key
 * @param key the key value
 * @param x the actual x mouse position
 * @param y the actual y mouse position
 */
void special(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		if (opacity < 1.0f)
			opacity += 0.1f;
		std::cout << "set opacity to " << opacity << std::endl;
		break;
	case GLUT_KEY_DOWN:
		if (opacity > 0.0f)
			opacity -= 0.1f;
		std::cout << "set opacity to " << opacity << std::endl;
		break;
	case GLUT_KEY_F1:
		selectedBlendFunc = (selectedBlendFunc+1)%6;
		std::cout << "use blend func: " << selectedBlendFunc << std::endl;
		break;
	case GLUT_KEY_F2:
		selectedBlendEquation = (selectedBlendEquation+1)%5;
		std::cout << "use blend equation: " << selectedBlendEquation << std::endl;
		break;
	}
	glutPostRedisplay();
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
	mousePosX = x;
	mousePosY = y;

	glutPostRedisplay();
}

/**
 * called when the mouse moves
 * @param x mouse x position in pixel relative to the window
 * @param y mouse x position in pixel relative to the window
 */
void mouseMotion(int x, int y) {
	if (leftMouseButtonDown) {
		rotationY += mousePosX - x;
		rotationX += mousePosY - y;

		mousePosX = x;
		mousePosY = y;
	}
}


int setupGLUT(int argc, char** argv) {
	glutInit(&argc, argv);
	// glutInitContextVersion(3, 0);
//	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
//	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ACCUM);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ACCUM | GLUT_MULTISAMPLE);

	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);

	int windowId = glutCreateWindow("ex3DepthHandlingAndRasterization");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);

	return windowId;
}

int main(int argc, char** argv) {
	setupGLUT(argc, argv);

	oogl::dumpGLInfos();

	init();

	glutMainLoop();

	return 0;
}
