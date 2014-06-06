#include <iostream>
#include <cstdlib>

#include <GLee.h>
#if defined(__APPLE__) || defined(__APPLE_CC__)
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
# include <GLUT/glut.h>
#else
# include <GL/gl.h>		// OpenGL header
# include <GL/glu.h>	// OpenGL Utility header
# include <GL/glut.h>	// GLUT header
#endif

#include <oogl/gl_error.h>
#include <oogl/timer.h>

#include <oogl/Model.h>


oogl::Model* complexModel = NULL;

void cleanup();

void init() {
	atexit(cleanup);

	complexModel = oogl::loadModel("models/NabooFighter.3ds", oogl::Model::LOAD_NORMALIZE_TWO);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void cleanup() {
	delete complexModel;
}

/**
 * called when a frame should be rendered
 */
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//	glBegin(GL_TRIANGLES);
//		glColor3f(1,0,0);
//		glVertex3f(-2,-1.5,-5.0);
//
//		glColor3f(0,1,0);
//		glVertex3f( 2,-1.5,-5.0);
//
//		glColor3f(0,0,1);
//		glVertex3f( 0, 1.5,-5.0);
//	glEnd();
	glLoadIdentity();
	glTranslatef(0,0,-5);
//	glColor3f(0,1,0);
//
//	glutSolidTeapot(1);
	complexModel->render();

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
	gluPerspective(45, ((float)w)/h,0.1f, 100);

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
	case 'x': //x-key example
		//do something
		break;
	case 'b':
		std::cout << "change clear color to dark blue" << std::endl;
		glClearColor(0,0,0.5f,1.0f);
		break;
	case 'g':
		std::cout << "change clear color to dark green" << std::endl;
		glClearColor(0,0.5f,0,1.0f);
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
	glutPostRedisplay();
}

/**
 * called when the mouse moves
 * @param x mouse x position in pixel relative to the window
 * @param y mouse x position in pixel relative to the window
 */
void mouseMotion(int x, int y) {
}


int setupGLUT(int argc, char** argv) {
	glutInit(&argc, argv);
	// glutInitContextVersion(3, 0);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);

	int windowId = glutCreateWindow("ex1Introduction");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
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
