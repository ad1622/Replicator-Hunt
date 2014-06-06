#include <iostream>
#include <iomanip>
#include <cstdlib>

#include <GLee.h>
#include <oogl/glutIncludes.h>
#include <oogl/gl_error.h>

#include <oogl/Model.h>


int windowWidth, windowHeight;

bool leftMouseButtonDown = false;
int mousePosX = 0, mousePosY = 0;

bool useOrtho = false;
float angle = 0.0f;
float rotationX = 0, rotationY = 0;
float reflectionMatrix[] = {
		-1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1};

std::auto_ptr<oogl::Model> modelDeathStar;
std::auto_ptr<oogl::Model> modelEndor;
std::auto_ptr<oogl::Model> modelAWing;
std::auto_ptr<oogl::Model> modelStarDestroyer;
std::auto_ptr<oogl::Model> modelTie;

void init() {
	modelDeathStar.reset(oogl::loadModel("models/deathstar/dstar.3ds"));
	modelEndor.reset(oogl::loadModel("models/endor/endor.3ds"));
	modelAWing.reset(oogl::loadModel("models/awing/Awing.3ds"));
	modelStarDestroyer.reset(oogl::loadModel("models/stardestroyer/stardestroyer.3ds"));
	modelTie.reset(oogl::loadModel("models/tie.3ds"));

	glEnable(GL_DEPTH_TEST); // we enable the depth test, to handle occlusions

	// enable lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);

	// set clear color to black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}


//void drawSolarSystem() {
//	//yellow sun
//	glColor3f(1.0f, 1.0f, 0.0f);
//	glutSolidSphere(5.0, 30, 30);
//
//	glPushMatrix(); // push current activated matrix
//	{
//		glRotatef(angle, 0.0f, 1.0f, 0.0f);
//		glTranslatef(20.0f, 0.0f, 0.0f);
//		glScalef(0.5f, 0.5f, 0.5f);
//		//blue planet
//		glColor3f(0.0f, 0.0f, 1.0f);
//		glutSolidSphere(5.0, 30, 30);
//
//		glRotatef(30, 1.0f, 0.0f, 0.0f);
//		glRotatef(angle*2, 0.0f, 1.0f, 0.0f);
//		glTranslatef(10.0f, 0.0f, 0.0f);
//		glScalef(0.5f, 0.5f, 0.5f);
//		//grey moon
//		glColor3f(0.8f, 0.8f, 0.8f);
//		glutSolidSphere(5.0, 30, 30);
//	}
//	glPopMatrix(); // restore matrix
//
//	glPushMatrix(); // save current matrix
//	{
//		//translate, scale and rotate
//		glRotatef(-angle, 0.0f, 1.0f, 0.0f);
//		glTranslatef(10.0f, 0.0f, 0.0f);
//		glScalef(0.3f, 0.3f, 0.3f);
//
//		//mercury
//		glColor3f(0.55f, 0.15f, 0.15f);
//		glutSolidSphere(5.0, 30, 30);
//	}
//	// and restore it
//	glPopMatrix();
//}
//void display() {
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear color and depth buffer
//
//	glMatrixMode( GL_PROJECTION); // switch to projection matrix
//	glLoadIdentity(); // reset current matrix
//	if (useOrtho) {
//		// define an orthographic projection
//		glOrtho(-25, 25, -9.4, 9.4, 1.0, 300.0);
//	} else {
//		// define a perspective projection matrix
//		gluPerspective(60, windowWidth/((float)windowHeight/2), 1.0, 300.0);
//	}
//
//	// switch to modelview matrix
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//
//	// set vantage point
////	glTranslatef(0.0f, 0.0f, -50.0f);
//	gluLookAt(0.0, 0.0, angle, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
//
//	// add rotation
//	glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
//	glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
//
//	// set first viewport (lower half)
//	glViewport(0, 0, windowWidth, windowHeight / 2);
//	drawSolarSystem();
//
//	glMultMatrixf(reflectionMatrix); //multiply with custom reflection matrix
//
//	// set second viewport (upper half)
//	glViewport(0, windowHeight / 2, windowWidth, windowHeight / 2);
//	drawSolarSystem();
//
//	LOG_GL_ERRORS();
//	glutSwapBuffers(); // draw scene
//}
//
void drawEndor() {
	glTranslatef(0,0,-35);
	glPushMatrix();
	{
		glRotatef(angle*360,0,1,0);
		glRotatef(90,1,0,0);
		glScalef(32,32,32);
		modelEndor->render();
	}
	glPopMatrix();

	glRotatef(angle*2*360,0,1,0);
	glTranslatef(0,0,40);

	glPushMatrix();
		glRotatef(-angle*2*360,0,1,0);
		glRotatef(-80,0,1,0);
		glScalef(4,4,4);
		modelDeathStar->render();
	glPopMatrix();

	glPushMatrix();
		glRotatef(30, 1.0f, 0.0f, 0.0f);
		glRotatef(-angle*5*360,0,1,0);

		glTranslatef(6,0,0);
		glRotatef(250,0,1,0);
		glRotatef(-60,1,0,0);
		glScalef(2.4f,2.4f,2.4f);
		modelStarDestroyer->render();
	glPopMatrix();
}

void drawFighters() {
	if(angle < 0.0635f || angle > 0.5f)
		return;
	float time = (angle-0.0635f)/(0.5f-0.0635f); //to 0..1 range

	glTranslatef(0,0,25);
	glRotatef(45,0,1,1);

	glPushMatrix();
		glTranslatef(-20*(1-time)+110*(time),0,0);
		glRotatef(30,1,0,0);
		glRotatef(180,0,1,0);
		glScalef(4,4,4);
		modelAWing->render();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(-50*(1-time)+90*(time),0,0);
		glRotatef(30,1,0,0);
		glScalef(2.4f,2.4f,2.4f);
		modelTie->render();
	glPopMatrix();
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear color and depth buffer

	glMatrixMode(GL_MODELVIEW); // switch to modelview matrix
	glLoadIdentity();
	gluLookAt(0.0, 0.0, 50.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	glPushMatrix();
		drawEndor();
	glPopMatrix();

	glPushMatrix();
		drawFighters();
	glPopMatrix();

	LOG_GL_ERRORS();
	glutSwapBuffers(); // draw scene
}

/**
 * callback function, which can be used for the glutTimerFunc
 * @param value parameter, which can be specified in glutTimerFunc
 */
void update(int value) {


//	//increment angle variable and keep it in the range of 0..32s
//	angle += 2.0f;
//	if (angle > 360)
//		angle -= 360;
	angle += .0005f;
	if (angle >= 1.f)
		angle -= 1.f;
	glutPostRedisplay();
	glutTimerFunc(25, update, 0); //request to call again in at least 25ms
}

void reshape(int w, int h) {
	windowWidth = w;
	windowHeight = h;

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, w/(float)h, 1.0, 1000.0);
}

void idle() {
	glutPostRedisplay(); //force a redisplay
}

//utility method to dump a float matrix
void dumpMatrix(float* m, int c, int r) {
	for(int i = 0; i < r; ++i) {
		for(int j = 0; j < c; ++j)
			std::cout << std::fixed << std::setw(4) << std::setprecision(3) << m[i*c+j] << " ";
		std::cout << std::endl;
	}
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27://27=esc
		exit(0);
		break;
	case 'm': {
		float m[4*4];
		glGetFloatv(GL_MODELVIEW_MATRIX, m);
		std::cout << std::endl << "model view matrix" << std::endl;
		dumpMatrix(m, 4,4);
		break;
	}
	case 'o':
		useOrtho = !useOrtho;
		std::cout << "use " << (useOrtho?"orthographic":"perspective") << " projection" << std::endl;
		break;
	}
}

void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		leftMouseButtonDown = true;
	else
		leftMouseButtonDown = false;

	mousePosX = x;
	mousePosY = y;
}

void mouseMotion(int x, int y) {
	if (leftMouseButtonDown) {
		//add the relative movement of the mouse to the rotation variables
		rotationX += mousePosY - y;
		rotationY += mousePosX - x;

		mousePosX = x;
		mousePosY = y;
	}
}

int setupGLUT(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	int windowId = glutCreateWindow("ex2TransformationAndProjection");

	glutDisplayFunc(display);
	//glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);

	glutTimerFunc(25, update, 0);  //request to call in at least 25ms

	return windowId;
}

int main(int argc, char** argv) {
	setupGLUT(argc, argv);

	oogl::dumpGLInfos();

	init();

	glutMainLoop();

	return 0;
}
