#include <iostream>
#include <cstdlib>

#include <GLee.h>
#include <oogl/glutIncludes.h>

#include <oogl/gl_error.h>
#include <oogl/Model.h>
#include <oogl/Image.h>


int windowWidth, windowHeight;

bool leftMouseButtonDown = false;
int mousePosX = 0, mousePosY = 0;
float rotationX = 0, rotationY = 0;

oogl::Model *vader = NULL;
oogl::Model *vaderCape = NULL;

GLuint textureId = -1;

bool useFlatShading = true;	//use flat or smooth shading
bool useLights = false;	//enable first light
bool useMaterial = false; //use materials
float materialShininess = 64.0;   //material shininess

float angle = 0;

void cleanup();

void init() {
	atexit(cleanup);

	//load vader model, we use LOAD_SET_SMOOTHING_GROUP option to overwrite the (wrong?) smoothing group values in this model
	vader = oogl::loadModel("models/vader/vader.3ds", oogl::Model::LOAD_NO_NORMALIZATION | oogl::Model::LOAD_SET_SMOOTHING_GROUP);
	vaderCape = oogl::loadModel("models/vader/vaderCape.3ds", oogl::Model::LOAD_NO_NORMALIZATION | oogl::Model::LOAD_SET_SMOOTHING_GROUP);

	{
		//load a image using the library and devil
		std::auto_ptr<oogl::Image> image(oogl::loadImage("models/lava.jpg"));
		glGenTextures(1, &textureId); //generate a new texture

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureId); //bind our texture

		//what should be done, if you want to access a texture outside the normal 0..1 range?
		//one of GL_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER or GL_MIRRORED_REPEAT
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//set texture interpolation method to use linear interpolation (no MIPMAPS)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		//define the texture and load data
		glTexImage2D(GL_TEXTURE_2D,
			0, //mip map level 0..top
			GL_RGBA, //internal format of the data in memory
			image->getWidth(),
			image->getHeight(),
			0,//border width in pixels (can either be 1 or 0)
			image->getFormat(),	// Image format (i.e. RGB, RGBA, BGR etc.)
			image->getType(),// Image data type
			image->getData());// The actual image data itself

		glBindTexture(GL_TEXTURE_2D, 0); //unbind again
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

void cleanup() {
	delete vader;
	delete vaderCape;

	if(textureId > 0) //cleanup the texture
		glDeleteTextures(1, &textureId);
}


void renderLightSphere() {
	glDisable(GL_LIGHTING); //temporary disable lightning

	glColor4f(1,1,1,1);
	glutSolidSphere(.03,10,10);

	glEnable(GL_LIGHTING);
}

void setLights() {
	glEnable(GL_LIGHTING); // Enable lighting
	glEnable(GL_LIGHT0);

	//setup light0
	float ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
	float diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
	float specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
	float position[] = {0.0f, 0.0f, 0.0f, 1.0f};

	if(useLights) {
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

		glPushMatrix();
		//the light position is affected by the model view matrix, too
		glRotatef(45, 0,0,1);
		glRotatef(angle, 0,1,0);
		glTranslatef(0.8f,0,0);
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		renderLightSphere();
		glPopMatrix();
	} else {
		float zero[] = {0.0f, 0.0f, 0.0f, 1.0f};
		//set to black values
		glLightfv(GL_LIGHT0, GL_AMBIENT, zero);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, zero);
		glLightfv(GL_LIGHT0, GL_SPECULAR, zero);
		glLightfv(GL_LIGHT0, GL_POSITION, zero);
	}
}

void setCapeMaterial() {
	float zero[] = {0.0f, 0.0f, 0.0f, 1.0f};

	float ambient[] = {.1f,0,0, 1.0f};
	float diffuse[] = {.7f,0,0, 1.0f};
	float specular[] = {1,0,0, 1.0f};
	float emission[] = {.1f,0,0, 1.0f};

	if (useMaterial) {
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);
	} else {
		//set to black values
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, zero);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, zero);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0);
	}
}

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

void renderFloor() {
	glPushMatrix();
	glEnable(GL_COLOR_MATERIAL); //use vertex colors instead of material colors
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE); //use the vertex color as diffuse color

	glEnable(GL_TEXTURE_2D); //enable texturing
	glActiveTexture(GL_TEXTURE0); //make the texture0 the active one
	glBindTexture(GL_TEXTURE_2D, textureId); //bind our texture to texture0

	glTranslatef(0,-.93f,0);
	glScalef(.8f,.8f,.8f);


	glBegin(GL_QUADS);
		glColor4f(1,1,1,1);
		glNormal3f(0,1,0); //specify the normal for the following vertices

		glTexCoord2f(0.0, 0.0);
		glVertex3f(-1.0, 0, -1.0);

		glTexCoord2f(0.0, 5.0);
		glVertex3f(-1.0, 0, 1.0);

		glTexCoord2f(5.0, 5.0);
		glVertex3f(1.0, 0, 1.0);

		glTexCoord2f(5.0, 0.0);
		glVertex3f(1.0, 0, -1.0);
	glEnd();

	glDisable(GL_COLOR_MATERIAL);
	//disable enabled features again
	glDisable(GL_TEXTURE_2D);

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
	gluLookAt(0.0, 0.0, 2.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// add rotation
	glRotatef(rotationX, 1.0f, 0.0f, 0.0f );
	glRotatef(rotationY, 0.0f, 1.0f, 0.0f );

	// set specific light properties
	setLights();

	if(useFlatShading) {
		glShadeModel(GL_FLAT);
	} else {
		glShadeModel(GL_SMOOTH);
	}

	renderVader();
	renderFloor();

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
	case 'f':
		useFlatShading = !useFlatShading;
		std::cout << "shading: " << (useFlatShading?"flat":"smooth") << std::endl;
		break;
	case 'l':
		useLights = !useLights;
		std::cout << "light: " << std::boolalpha << useLights << std::endl;
		break;
	case 'm':
		useMaterial = !useMaterial;
		std::cout << "material: " << std::boolalpha << useMaterial << std::endl;
		break;
	}
	glutPostRedisplay();
}

void special(int key, int x, int y) {
	switch(key) {
	case GLUT_KEY_UP :
		if (materialShininess < 128.0)
			materialShininess += 1.0;
		std::cout << "shininess: "  << materialShininess << std::endl;
		break;
	case GLUT_KEY_DOWN :
		if (materialShininess > 0.0)
			materialShininess -= 1.0;
		std::cout << "shininess: "  << materialShininess << std::endl;
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

void update(int value) {
	angle += 1.0f;
	if (angle > 360)
		angle -= 360;

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

	int windowId = glutCreateWindow("ex4LocalShading");

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
