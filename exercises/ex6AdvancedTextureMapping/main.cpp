
#include "Scene.h"
#include "ExerciseScenes.h"

//Uncomment the scene you want to see:
Scene* scene = new ThreeDTextureScene();
//Scene* scene = new EnvMappingScene();
//Scene* scene = new ShadowMappingScene();
//Scene* scene = new MultiTexturingScene();

void cleanup();

void init() {
	atexit(cleanup);

	scene->init();
}

void cleanup() {
	scene->cleanup();
	delete scene;
}

/**
 * called when a frame should be rendered
 */
void display() {
	scene->display();
}

/**
 * called when the window was resized
 * @param w new window width in pixel
 * @param h new window height in pixel
 */
void reshape(int w, int h) {
	scene->reshape(w,h);
}

/**
 * called when nothing else needs to be done
 */
void idle() {
	scene->idle();
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
	}
	scene->keyboard(key, x, y);
}

/**
 * called when the user pressed or released a mouse key
 * @param button which mouse button was pressed, one of GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON and GLUT_RIGHT_BUTTON
 * @param state button pressed (GLUT_DOWN) or released(GLUT_UP)
 * @param x mouse x position in pixel relative to the window, when the mouse button was pressed
 * @param y mouse y position in pixel relative to the window, when the mouse button was pressed
 */
void mouse(int button, int state, int x, int y) {
	scene->mouse(button, state, x, y);
}

/**
 * called when the mouse moves
 * @param x mouse x position in pixel relative to the window
 * @param y mouse x position in pixel relative to the window
 */
void mouseMotion(int x, int y) {
	scene->mouseMotion(x, y);
}

void special(int key, int x, int y) {
	scene->special(key, x, y);
}

void update(int value) {
	scene->update();

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
	glutMotionFunc(mouseMotion);
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
