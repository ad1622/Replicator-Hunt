/*
 * Scene.h
 *
 *  Created on: 21.05.2011
 *      Author: sam
 */

#ifndef SCENE_H_
#define SCENE_H_

#include <iostream>
#include <cstdlib>

#include <oogl/glutIncludes.h>


class Scene {
public:
	Scene();
	virtual ~Scene();

	virtual void init();
	virtual void display();
	virtual void cleanup();
	virtual void reshape(int w, int h);
	virtual void idle();
	virtual void keyboard(unsigned char key, int x, int y);
	virtual void special(int key, int x, int y);
	virtual void mouse(int button, int state, int x, int y);
	virtual void mouseMotion(int x, int y);
	virtual void update();

	virtual void renderScene() = 0;

protected:
	float angle;

protected:
	bool leftMouseButtonDown;
	int mousePosX, mousePosY;
	float rotationX, rotationY;
};

#endif /* SCENE_H_ */

