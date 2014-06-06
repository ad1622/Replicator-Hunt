/*
 * Scene.cpp
 *
 *  Created on: 21.05.2011
 *      Author: sam
 */

#include "Scene.h"
#include <oogl/gl_error.h>

Scene::Scene(): angle(0),leftMouseButtonDown(false), mousePosX(0), mousePosY(0),
	rotationX(0), rotationY(0) {
}

Scene::~Scene() {

}

void Scene::init() {
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void Scene::cleanup() {

}

void Scene::display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderScene();

	LOG_GL_ERRORS();
	glutSwapBuffers();
}

void Scene::reshape(int w, int h) {
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, ((float) w) / h, 0.1f, 1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void Scene::idle() {
	//force a redisplay
	glutPostRedisplay();
}

void Scene::update() {
	angle += 0.5f;
	if (angle > 360)
		angle -= 360;

	glutPostRedisplay();
}

void Scene::keyboard(unsigned char key, int x, int y) {

}

void Scene::special(int key, int x, int y) {
}

void Scene::mouse(int button, int state, int x, int y) {
	leftMouseButtonDown = (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN);
	mousePosX = x;
	mousePosY = y;

	glutPostRedisplay();
}

void Scene::mouseMotion(int x, int y) {
	if (leftMouseButtonDown) {
		rotationY += mousePosX - x;
		rotationX += mousePosY - y;

		mousePosX = x;
		mousePosY = y;
	}
}
