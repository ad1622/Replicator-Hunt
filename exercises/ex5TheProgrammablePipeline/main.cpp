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
#include <stdio.h>

#include <sstream>
#include <stdexcept>

#include <glm/glm.hpp> 
#include <glm/gtc/constants.hpp> 
#include <glm/gtc/matrix_transform.hpp> 

#define M_PI 3.1415926535897932384626433832795
#define PLAYER_SPEED 0.25
#define REPLICATOR_SPEED 0.02
#define PROJECTILE_SPEED 0.5

#define MAP_LEFT_FRONT_X 0
#define MAP_LEFT_FRONT_Z 0

#define MAP_RIGHT_BACK_X 64
#define MAP_RIGHT_BACK_Z 64

#define MAP_BOTTOM 0
#define MAP_TOP 2

#define MATERIAL_NEEDED_FOR_CLONING 3



int windowWidth, windowHeight;

bool leftMouseButtonDown = false;
int mousePosX = 0, mousePosY = 0;
// rotX = neigung, rotY = drehung
float rotationX = 0, rotationY = 0;
//spielerkoordinaten (negativ da verschiebung der map, m�ssen negiert werden um sie zu verwenden)
float eyeX = -20, eyeZ = -20;

oogl::Model *vader = NULL;
oogl::Model *vaderCape = NULL;

oogl::Texture* tex1 = NULL;
oogl::Texture* tex2 = NULL;
oogl::Texture* tex3 = NULL;
oogl::Texture* tex4 = NULL;

GLuint simpleshaderprogram;
GLuint phongshaderprogram;

bool moveLight = true;	//move light
bool enableflashlight = false;

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
bool gamerunning = true;
bool isinmenu = false;
bool gameover = false;

typedef struct{
	float x, z;
	int rot, index;
	int gathered;
} Replicator;

typedef struct{
	float x,z;
	int count;
} ReplicatorBasicMaterial;

typedef struct{
	float x, y, z;
	float tilt, rot;
	int index;
} Projectile;

Replicator * Replicators[50];
int ReplicatorCount;
int REPLICATOR_MAX = 50;

Projectile * Projectiles[25];
int ProjectileCount;
int PROJECTILE_MAX = 25;

ReplicatorBasicMaterial* rMaterials[50];
int rMaterialCount;
int rMaterialMax=50;


void cleanup();
void addReplicator(float x, float z, int rot);
int addRMaterial(float x,float z, int count);
void RemoveProjectile(int index);
void removeReplicator(int index);
void initgame();
void renderCellStrip(int length, float width, float depth, oogl::Texture* tex);

void initSimpleShader()
{
	//create program
	simpleshaderprogram = glCreateProgram();

	//compile shaders
	GLuint vertexshader = oogl::loadShader("shader/simpleshader.vert", GL_VERTEX_SHADER);
	GLuint fragmentshader = oogl::loadShader("shader/simpleshader.frag", GL_FRAGMENT_SHADER);

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
	}
	else {
		LOG_DEBUG << "linked program - no errors" << std::endl;
	}
}

void initPhongShader()
{
	//create program
	phongshaderprogram = glCreateProgram();

	//compile shaders
	GLuint vertexshader = oogl::loadShader("shader/phong.vert", GL_VERTEX_SHADER);
	GLuint fragmentshader = oogl::loadShader("shader/phong.frag", GL_FRAGMENT_SHADER);

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
	}
	else {
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

	glutSetCursor(GLUT_CURSOR_CROSSHAIR);

	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH)*0.5, glutGet(GLUT_WINDOW_HEIGHT)*0.5);

	tex1 = oogl::loadTexture("models/Gravel.png");
	tex2 = oogl::loadTexture("models/ReplBlock.jpg");
	tex3 = oogl::loadTexture("models/Concrete.png");
	tex4 = oogl::loadTexture("models/oak.png");



	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

	//sky color = blue
	glClearColor(0.4f, 0.4f, 1.0f, 1.0f);
	initgame();
}

void initgame(){
	isinmenu = false;
	gamerunning = true;
	gameover = false;
	healthpoints = 100;

	int i;
	if (ProjectileCount != 0)
		for (i = 0; i < PROJECTILE_MAX; i++){
			if (Projectiles[i] != NULL)
				RemoveProjectile(i);
		}

		if (ReplicatorCount != 0)
			for (i = 0; i < REPLICATOR_MAX; i++){
				if (Replicators[i] != NULL)
					removeReplicator(i);
			}

			ReplicatorCount = 0;
			ProjectileCount = 0;

			addReplicator(0, 0, 45);
			addReplicator(64, 64, 225);
			addReplicator(64, 0,315);
			addReplicator(0, 64, 135);
			addReplicator(0, 0, 90);
			
			addRMaterial(5,5,100);
			addRMaterial(50,50,100);


}

void cleanup() {
	delete vader;
	delete vaderCape;

	delete tex1;
	delete tex2;
	delete tex3;
	delete tex4;

	if (simpleshaderprogram > 0)
		glDeleteProgram(simpleshaderprogram);

	if (phongshaderprogram > 0)
		glDeleteProgram(phongshaderprogram);
}

void renderLightSphere(float lcolor[]) {
	glDisable(GL_LIGHTING); //temporary disable lightning

	//set color for sphere to the given light color
	glColor4fv(lcolor);

	glutSolidSphere(.03, 10, 10);

	glEnable(GL_LIGHTING);
}

void setLights() {
	glEnable(GL_LIGHTING); // Enable lighting
	glEnable(GL_LIGHT0);

	//setup light0 (SUN)
	float ambient[] = { 0.05f, 0.05f, 0.05f, 1.00f };

	float diffuse[] = { 1.0f, 1.0f, 0.7f, 1.0f };
	float specular[] = { 1.0f, 1.0f, 0.7f, 1.0f };
	float position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float spotdir[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	glPushMatrix();

	glTranslatef(32, 32, 32);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glScalef(50, 50, 50);
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

	//spot
	diffuse[0] = 1.0f;
	diffuse[1] = 1.0f;
	diffuse[2] = 1.0f;

	specular[0] = 1.0f;
	specular[1] = 1.0f;
	specular[2] = 1.0f;

	position[0] = -eyeX;
	position[1] = 1.5f;
	position[2] = -eyeZ;

	glm::mat4 m = glm::rotate(glm::mat4(1.0f), rotationX, glm::vec3(0.0f, 0.0f, 1.0f));
	m = glm::rotate(m, rotationY, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec4 t = glm::vec4(1.0, 0.0, 0.0, 1.0);
	t = t*m;
	t = glm::normalize(t);
	spotdir[0] = t.x;
	spotdir[1] = t.y;
	spotdir[2] = t.z;

	glEnable(GL_LIGHT2);
	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 15.0f);
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spotdir);
	glLightfv(GL_LIGHT2, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, specular);

	glPushMatrix();
	glTranslatef(0, 0.1, 0);
	glLightfv(GL_LIGHT2, GL_POSITION, position);
	glPopMatrix();
}

//GUI-HUD STUFF
//prints text do display
void print(float x, float y, float r, float g, float b, void* font, const char *string, ...)
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

//draws a hud over the 3d stuff
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
	glDisable(GL_LIGHTING);

	//draw here -1,-1 left bottom / 1,1 top right

	print(-0.99, 0.9, 1, 1, 1, GLUT_BITMAP_HELVETICA_18, "SCORE: %d", score);
	print(-0.99, 0.8, 1, 1, 1, GLUT_BITMAP_HELVETICA_18, "HP: %d", healthpoints);
	print(-0.99, 0.7, 1, 1, 1, GLUT_BITMAP_HELVETICA_18, "Replicators: %d /Projectiles: %d", ReplicatorCount, ProjectileCount);
	print(-0.99, 0.6, 1, 1, 1, GLUT_BITMAP_HELVETICA_18, "x: %f / z: %f", eyeX, eyeZ);
	print(-0.99, 0.5, 1, 1, 1, GLUT_BITMAP_HELVETICA_18, "neigung: %f / drehung: %f", rotationX, rotationY);

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void drawMenu(){
	glUseProgram(simpleshaderprogram);
	glUniform4f(glGetUniformLocation(simpleshaderprogram, "mycolor"), 1.0, 1.0, 1.0, 1.0);
	glUniform1i(glGetUniformLocation(simpleshaderprogram, "mytexture"), 5);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDepthMask(GL_FALSE);  // disable writes to Z-Buffer
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	//draw here -1,-1 left bottom / 1,1 top right

	glPushMatrix();
	glEnable(GL_COLOR_MATERIAL); //use vertex colors instead of material colors
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE); //use the vertex color as diffuse color

	glEnable(GL_TEXTURE_2D); //enable texturing

	tex3->bind(5);

	glBegin(GL_QUADS);
	glColor4f(1, 1, 1, 1);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(-1.0, -1.0);

	glTexCoord2f(1.0, 0.0);
	glVertex2f(1.0, -1.0);

	glTexCoord2f(1.0, 1.0);
	glVertex2f(1.0, 1.0);

	glTexCoord2f(0.0, 1.0);
	glVertex2f(-1.0, 1.0);
	glEnd();

	tex3->unbind();
	tex1->bind(5);
	glBegin(GL_QUADS);
	glColor4f(1, 1, 1, 1);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(-0.5, 0.25);

	glTexCoord2f(5.0, 0.0);
	glVertex2f(0.5, 0.25);

	glTexCoord2f(5.0, 1.0);
	glVertex2f(0.5, 0.5);

	glTexCoord2f(0.0, 1.0);
	glVertex2f(-0.5, 0.5);
	glEnd();

	glBegin(GL_QUADS);
	glColor4f(1, 1, 1, 1);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(-0.5, -0.125);

	glTexCoord2f(5.0, 0.0);
	glVertex2f(0.5, -0.125);

	glTexCoord2f(5.0, 1.0);
	glVertex2f(0.5, 0.125);

	glTexCoord2f(0.0, 1.0);
	glVertex2f(-0.5, 0.125);
	glEnd();

	glBegin(GL_QUADS);
	glColor4f(1, 1, 1, 1);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(-0.5, -0.5);

	glTexCoord2f(5.0, 0.0);
	glVertex2f(0.5, -0.5);

	glTexCoord2f(5.0, 1.0);
	glVertex2f(0.5, -0.25);

	glTexCoord2f(0.0, 1.0);
	glVertex2f(-0.5, -0.25);
	glEnd();

	tex1->unbind();

	//disable enabled features again
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_MATERIAL);

	glPopMatrix();
	glUseProgram(0);
	glPushMatrix();

	if (gameover){
		if (healthpoints <= 0)
		print(-0.5, 0.6, 0, 0, 0, GLUT_BITMAP_HELVETICA_18, "Game over, your score is: %d", score);
		else
		print(-0.5, 0.6, 0, 0, 0, GLUT_BITMAP_HELVETICA_18, "You Won, your score is: %d", score);
		print(-0.15, 0.35, 0.5, 0.5, 0.5, GLUT_BITMAP_HELVETICA_18, "Return to Game");
	}
	else{
		print(-0.15, 0.35, 1, 1, 1, GLUT_BITMAP_HELVETICA_18, "Return to Game");
	}
	
	print(-0.075, -0.025, 1, 1, 1, GLUT_BITMAP_HELVETICA_18, "Restart");
	print(-0.05, -0.4, 1, 1, 1, GLUT_BITMAP_HELVETICA_18, "Exit");

	glPopMatrix();
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
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


void renderFloor(){

	float hinten, vorne, links, rechts, oben, unten, width, length, depth;
	vorne = 0;
	hinten = 64;
	rechts = 64;
	links = 0;
	oben = 2;//*0.5;
	unten = 0;// -oben;
	width = 64;
	length = 2;
	depth = 64;

	glPushMatrix();
	glEnable(GL_COLOR_MATERIAL); //use vertex colors instead of material colors
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE); //use the vertex color as diffuse color
	glEnable(GL_TEXTURE_2D); //enable texturing

	tex3->bind(5);

	glColor4f(0.5, 0.5, 0.5, 1);

	//front:
	glBegin(GL_TRIANGLE_STRIP);
	glNormal3f(0, 0, 1);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(links, unten, vorne);
	glTexCoord2f(0.0, length / 2.0);
	glVertex3f(links, oben, vorne);
	glTexCoord2f(width / 2.0, 0.0);
	glVertex3f(rechts, unten, vorne);
	glTexCoord2f(width / 2.0, length / 2.0);
	glVertex3f(rechts, oben, vorne);
	glEnd();


	//hinten:
	glBegin(GL_TRIANGLE_STRIP);
	glNormal3f(0, 0, -1);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(rechts, unten, hinten);
	glTexCoord2f(0.0, length / 2.0);
	glVertex3f(rechts, oben, hinten);
	glTexCoord2f(width / 2.0, 0.0);
	glVertex3f(links, unten, hinten);
	glTexCoord2f(width / 2.0, length / 2.0);
	glVertex3f(links, oben, hinten);
	glEnd();


	//rechts
	glBegin(GL_TRIANGLE_STRIP);
	glNormal3f(-1, 0, 0);
	glTexCoord2f(depth / 2.0, 0.0);
	glVertex3f(rechts, unten, hinten);
	glTexCoord2f(depth / 2.0, length / 2.0);
	glVertex3f(rechts, oben, hinten);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(rechts, unten, vorne);
	glTexCoord2f(0.0, length / 2.0);
	glVertex3f(rechts, oben, vorne);
	glEnd();

	//links:
	glBegin(GL_TRIANGLE_STRIP);
	glNormal3f(1, 0, 0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(links, unten, hinten);
	glTexCoord2f(0.0, length / 2.0);
	glVertex3f(links, oben, hinten);
	glTexCoord2f(depth / 2.0, 0.0);
	glVertex3f(links, unten, vorne);
	glTexCoord2f(depth / 2.0, length / 2.0);
	glVertex3f(links, oben, vorne);
	glEnd();

	tex3->unbind();
	tex1->bind(5);
	//unten:
	glBegin(GL_TRIANGLE_STRIP);
	glNormal3f(0, 1, 0);
	glTexCoord2f(0.0, depth / 2.0);
	glVertex3f(links, unten, hinten);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(links, unten, vorne);
	glTexCoord2f(width / 2.0, depth / 2.0);
	glVertex3f(rechts, unten, hinten);
	glTexCoord2f(width / 2.0, 0.0);
	glVertex3f(rechts, unten, vorne);
	glEnd();
	tex1->unbind();


	//disable enabled features again
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_MATERIAL);

	glPopMatrix();
}

void setMapMaterial() {

	float zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	float ambient[] = { .2f, 0.2, 0.2, 1.0f };
	float diffuse[] = { 0.65, 0.65, 0.65, 1.0f };
	float specular[] = { 0.15, 0.15, 0.15, 1.0f };
	float emission[] = { .0f, .0f, .0f, 1.0f };

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 155.0);
}

void setSlingMaterial() {

	float zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	float ambient[] = { 1.0f, 1.0, 1.0, 1.0f };
	float diffuse[] = { 0.85, 0.85, 0.85, 1.0f };
	float specular[] = { 0.03, 0.03, 0.03, 1.0f };
	float emission[] = { .0f, .0f, .0f, 1.0f };

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 155.0);
}

void rendersling(){
	glPushMatrix();
	glTranslatef(0.3, -0.1, 0.1);
	glRotatef(90, 0, 1, 0);
	glScalef(0.02,0.02,0.02);

	renderCellStrip(3, 1, 1, tex4);

	glPushMatrix();
	glRotatef(45,0,0,1);
	glTranslatef(2,2,0);
	renderCellStrip(2, 1, 1, tex4);
	glPopMatrix();

	glPushMatrix();
	glRotatef(315, 0, 0, 1);
	glTranslatef(-2, 2, 0);
	renderCellStrip(2, 1, 1, tex4);
	glPopMatrix();

	glPopMatrix();
}
int addRMaterial(float x, float z, int count){
		int i;
		if (rMaterialCount < rMaterialMax)
		for (i = 0; i < rMaterialMax; i++){
			if (rMaterials[i] == NULL){
				ReplicatorBasicMaterial* p=new ReplicatorBasicMaterial();
				p->x=x;
				p->z=z;
				p->count=count;
				rMaterials[i] = p;
				rMaterialCount++;
				return i;
			}
		}
}
void RemoveRMaterial(int index){
	if (rMaterials[index] != NULL){
		delete(rMaterials[index]);
		rMaterials[index] = NULL;
		rMaterialCount--;
	}
}
int createRandomRMaterial(){
	return addRMaterial(rand()%63+1,rand()%63+1,rand()%8);
}
int findNearestRMaterial(float x, float z){
int i=0,bestIndex=-1;
float distance=64*64,newDistance;
		for (i = 0; i < rMaterialMax; i++){
			if (rMaterials[i] != NULL){
				newDistance=(x-rMaterials[i]->x)*(x-rMaterials[i]->x)+(z-rMaterials[i]->z)*(z-rMaterials[i]->z);
				if(newDistance<distance){
					bestIndex=i;
					distance=newDistance;
				}
			}
		}
		if(rMaterialCount<rMaterialMax){
			if(bestIndex==-1)
				return createRandomRMaterial();
		}
		return bestIndex;
}
//PROJECTILE STUFF
void addProjectile(float x, float y, float z, float tilt, float rot){
	int i;
	if (ProjectileCount < PROJECTILE_MAX)
		for (i = 0; i < PROJECTILE_MAX; i++){
			if (Projectiles[i] == NULL){
				Projectile* p=new Projectile();
				p->x=x;
				p->y=y;
				p->z=z;
				p->tilt=tilt;
				p->rot=rot;
				p->index=i;
				Projectiles[i] = p;
				ProjectileCount++;
				break;
			}
		}
}

void RemoveProjectile(int index){
	if (Projectiles[index] != NULL){
		delete(Projectiles[index]);
		Projectiles[index] = NULL;
		ProjectileCount--;
	}
}

void moveProjectiles(){
	int i;
	for (i = 0; i < PROJECTILE_MAX; i++){
		if (Projectiles[i] != NULL){
			//@TODO
			Projectiles[i]->x += PROJECTILE_SPEED*cosf(Projectiles[i]->rot*M_PI / 180.0);
			Projectiles[i]->z += PROJECTILE_SPEED*sinf(Projectiles[i]->rot*M_PI / 180.0);
			Projectiles[i]->y -= PROJECTILE_SPEED*sinf(Projectiles[i]->tilt*M_PI / 180.0);
		}
	}
}
void removeInvalidProjectiles(){
	//@TODO
	int i;
	for (i = 0; i < PROJECTILE_MAX; i++){
		if (Projectiles[i] != NULL){

			if(Projectiles[i]->x>MAP_RIGHT_BACK_X||Projectiles[i]->x<MAP_LEFT_FRONT_X||
				Projectiles[i]->z> MAP_RIGHT_BACK_Z||Projectiles[i]->z<MAP_LEFT_FRONT_Z||
				Projectiles[i]->y>MAP_TOP||Projectiles[i]->y<MAP_BOTTOM){
					RemoveProjectile(i);
			}
		}
	}
}

void renderProjectiles(){
	int i;

	for (i = 0; i < PROJECTILE_MAX; i++){
		if (Projectiles[i] != NULL){
			glPushMatrix();
			glTranslatef(Projectiles[i]->x, Projectiles[i]->y, Projectiles[i]->z);
			glColor4f(0, 0, 0, 1);
			glutSolidSphere(.03, 10, 10);
			glPopMatrix();
		}
	}
}

//REPLICATOR STUFF
void addReplicator(float x, float y, int rot){
	int i;
	if (ReplicatorCount < REPLICATOR_MAX)
		for (i = 0; i < REPLICATOR_MAX; i++){
			if (Replicators[i] == NULL){
				Replicator* r = new Replicator();
				r->x=x;
				r->z=y;
				r->rot=rot;
				r->index=i;
				Replicators[i]=r;
				ReplicatorCount++;
				break;
			}
		}
}

void removeReplicator(int index){
	if (Replicators[index] != NULL){
		delete(Replicators[index]);
		Replicators[index] = NULL;
		ReplicatorCount--;
	}
}

void replicatorLogic(){
	int i;
	float newx, newz;
	float dist;
	for (i = 0; i < REPLICATOR_MAX; i++){
		if (Replicators[i] != NULL){
			newx = -eyeX - Replicators[i]->x;
			newz = -eyeZ - Replicators[i]->z;
			dist = sqrtf(newx*newx+newz*newz);

			if (!(dist > 0.5 && dist <= 10)){
				int rMaterialIndex=findNearestRMaterial(Replicators[i]->x,Replicators[i]->z);
				ReplicatorBasicMaterial* rMat=rMaterials[rMaterialIndex];
				if(abs(rMat->x-Replicators[i]->x)<0.4&&abs(rMat->z-Replicators[i]->z)<0.4){
					Replicators[i]->gathered+=rMat->count;
					if(Replicators[i]->gathered>=MATERIAL_NEEDED_FOR_CLONING){
						Replicators[i]->gathered-=MATERIAL_NEEDED_FOR_CLONING;
						addReplicator(Replicators[i]->z,Replicators[i]->x,Replicators[i]->rot);

					}
					RemoveRMaterial(rMaterialIndex);
					rMaterialIndex=findNearestRMaterial(Replicators[i]->x,Replicators[i]->z);
					rMat=rMaterials[rMaterialIndex];
				}

				newx = rMat->x- Replicators[i]->x;
				newz = rMat->z - Replicators[i]->z;
				dist = sqrtf(newx*newx+newz*newz);
			}
			if (newz>0){
				Replicators[i]->rot = asinf(newx / dist)*180.0 / M_PI;
			}
			else{
				Replicators[i]->rot = 180 - asinf(newx / dist)*180.0 / M_PI;
			}
			if (dist < 0.2 && repl_ani_dir_up){
				healthpoints--;
				if (healthpoints == 0){
					gameover = true;
					isinmenu = true;
				}
			}

			Replicators[i]->x += REPLICATOR_SPEED*sinf(Replicators[i]->rot*M_PI / 180.0);
			Replicators[i]->z += REPLICATOR_SPEED*cosf(Replicators[i]->rot*M_PI / 180.0);

			if (Replicators[i]->x < 0)Replicators[i]->x = 0;
			else if (Replicators[i]->x > 64) Replicators[i]->x = 64;
			if (Replicators[i]->z < 0)Replicators[i]->z = 0;
			else if (Replicators[i]->z > 64) Replicators[i]->z = 64;

		}
	}
}
void detectCollisions(){
	int i,j;

	for (i = 0; i < REPLICATOR_MAX; i++){
		if (Replicators[i] != NULL){
			Replicator* cr=Replicators[i];
			for (j = 0; j < PROJECTILE_MAX; j++){
				if (Projectiles[j] != NULL){
					Projectile* cp=Projectiles[j];
					if(abs(cp->x-cr->x)<0.5&&abs(cp->z-cr->z)<0.5){
						RemoveProjectile(j);
						removeReplicator(i);
						score++;
						if (ReplicatorCount == 0){
							gameover = true;
							isinmenu = true;
						}
					}
				}
			}
		}
	}
}

void setReplMaterial() {

	float zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	float ambient[] = { .2f, 0.2, 0.2, 1.0f };
	float diffuse[] = { 0.65, 0.65, 0.65, 1.0f };
	float specular[] = { 0.65, 0.65, 0.65, 1.0f };
	float emission[] = { .5f, .5f, .5f, 1.0f };

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32.0);
}

void renderCellStrip(int length, float width, float depth, oogl::Texture* tex){

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

	tex->bind(5);

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

	tex->unbind();

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
	renderCellStrip(3, 1, 1, tex2);

	glRotatef(alpha, 0, 0, 1);
	glTranslatef(0, 0, 0);
	renderCellStrip(2, 1, 1, tex2);

	glTranslatef(0, 2, 0);
	glRotatef(180 - beta, 0, 0, 1);
	renderCellStrip(6, 1, 1, tex2);

	glTranslatef(0, 6, 0);
	glRotatef(180 - gamma, 0, 0, 1);
	renderCellStrip(3, 1, 1, tex2);

	//disable enabled features again

	glPopMatrix();
}

void renderReplicator(Replicator * repl){
	glPushMatrix();
	setReplMaterial();
	glTranslatef(repl->x, 0.25, repl->z);
	glRotatef(repl->rot + 180, 0, 1, 0);
	glScalef(.05, .05f, .05f);
	renderCellStrip(3, 5, 7, tex2);

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
	glRotatef(-90 - 10, 0, 1, 0);
	renderReplicatorLeg(110 + 10 * -repl_ani_state, 70 + 15 * -repl_ani_state, 130 + 20 * -repl_ani_state);
	glPopMatrix();


	//front right
	glPushMatrix();
	glTranslatef(-1, 0, -5);
	glRotatef(-90 + 10, 0, 1, 0);
	renderReplicatorLeg(110 + 10 * repl_ani_state, 70 + 15 * repl_ani_state, 130 + 20 * repl_ani_state);
	glPopMatrix();

	glPopMatrix();
}

void renderReplicators(){
	int i;

	for (i = 0; i < REPLICATOR_MAX; i++){
		if (Replicators[i] != NULL){
			renderReplicator(Replicators[i]);
		}
	}
}
void renderRMaterial(ReplicatorBasicMaterial* rMat){
	glPushMatrix();
	glTranslatef(rMat->x,0.25,rMat->z);
	glScalef(.1, .1f, .1f);
	renderCellStrip(1,1,1, tex2);
	glPopMatrix();
}
void renderRMaterials(){
	int i;

	for (i = 0; i < rMaterialMax; i++){
		if (rMaterials[i] != NULL){
			renderRMaterial(rMaterials[i]);
		}
	}
}
//DISPLAY
/**
* called when a frame should be rendered
*/
void display() {
	if (!isinmenu){
		glutSetCursor(GLUT_CURSOR_CROSSHAIR);
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH)*0.5, glutGet(GLUT_WINDOW_HEIGHT)*0.5);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		// set vantage point
		gluLookAt(0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 1.0, 0.0);

		glUseProgram(phongshaderprogram);
		glUniform4f(glGetUniformLocation(phongshaderprogram, "mycolor"), 1.0, 1.0, 1.0, 1.0);
		glUniform1i(glGetUniformLocation(phongshaderprogram, "mytexture"), 5);
		glUniform1i(glGetUniformLocation(phongshaderprogram, "enablespot"), enableflashlight);

		setSlingMaterial();
		rendersling();

		// add rotation
		//rotation z(neigung)
		glRotatef(rotationX, 0.0f, 0.0f, 1.0f);
		//rotation y(drehung)
		glRotatef(rotationY, 0.0f, 1.0f, 0.0f);

		//translate scene
		glTranslatef(eyeX, -1.5, eyeZ);

		// set specific light properties
		glUseProgram(0);
		setLights();
		glUseProgram(phongshaderprogram);

		//renders all axisting replicators
		renderReplicators();
		//renders replicator basic material
		renderRMaterials();

		//sets material for map
		setMapMaterial();
		//renders map
		renderFloor();
		glUseProgram(0);
		//renders all projectiles
		renderProjectiles();
		//renders hud
		drawHUD();
	}
	else{
		//renders menu
		glutSetCursor(GLUT_CURSOR_INHERIT);
		drawMenu();
	}

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
	gluPerspective(60, ((float)w) / h, 0.25, 300);

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

//INPUT
//Spiel pausiert bei p (toggelt gamerunning boolean)
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27: //27=esc
		isinmenu = !isinmenu;
		break;
	case 'p':
		gamerunning = !gamerunning;
		break;
	case 'l':
		enableflashlight = !enableflashlight;
		break;
	}
	glutPostRedisplay();
}

void special(int key, int x, int y) {
	if (!isinmenu)
		switch (key) {
		case GLUT_KEY_UP:
			eyeZ -= PLAYER_SPEED*sinf(rotationY*M_PI / 180.0);
			eyeX -= PLAYER_SPEED*cosf(rotationY*M_PI / 180.0);
			break;
		case GLUT_KEY_DOWN:
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
	if (eyeX > 0)eyeX = 0;
	else if (eyeX < -64) eyeX = -64;
	if (eyeZ > 0)eyeZ = 0;
	else if (eyeZ < -64) eyeZ = 64;
}

/**
* called when the user pressed or released a mouse key
* @param button which mouse button was pressed, one of GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON and GLUT_RIGHT_BUTTON
* @param state button pressed (GLUT_DOWN) or released(GLUT_UP)
* @param x mouse x position in pixel relative to the window, when the mouse button was pressed
* @param y mouse y position in pixel relative to the window, when the mouse button was pressed
*/
void mouse(int button, int state, int x, int y) {

	if (leftMouseButtonDown && button == GLUT_LEFT_BUTTON && state == GLUT_UP){
		if (!isinmenu){
			addProjectile(-eyeX, 1.5, -eyeZ, rotationX, rotationY);
			glutPostRedisplay();
		}
		else{
			if (x <= glutGet(GLUT_WINDOW_WIDTH)*0.75 && x >= glutGet(GLUT_WINDOW_WIDTH)*0.25){
				if (y <= glutGet(GLUT_WINDOW_HEIGHT)*0.375 && y >= glutGet(GLUT_WINDOW_HEIGHT)*0.25 && !gameover){
					glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH)*0.5, glutGet(GLUT_WINDOW_HEIGHT)*0.5);
					isinmenu = false;
					glutPostRedisplay();
				}
				else if (y <= glutGet(GLUT_WINDOW_HEIGHT)*0.5625 && y >= glutGet(GLUT_WINDOW_HEIGHT)*0.4375){
					initgame();
					glutPostRedisplay();
				}
				else if (y <= glutGet(GLUT_WINDOW_HEIGHT)*0.75 && y >= glutGet(GLUT_WINDOW_HEIGHT)*0.625){
					exit(0);
				}
			}
		}
	}
	leftMouseButtonDown = (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN);
}

/**
* called when the mouse moves (and no buttons pressed)
* @param x mouse x position in pixel relative to the window
* @param y mouse x position in pixel relative to the window
*/
void passiveMouseMotion(int x, int y) {
	if (!isinmenu) {
		rotationY -= glutGet(GLUT_WINDOW_WIDTH)*0.5 - x;
		rotationX -= glutGet(GLUT_WINDOW_HEIGHT)*0.5 - y;
		if (rotationX >= 90) rotationX = 90;
		else if (rotationX <= -90) rotationX = -90;
	}
}

//MAIN

void update(int value) {
	if (!isinmenu){
		if (moveLight)
		{
			angle += 1.0f;
			if (angle > 360)
				angle -= 360;
		}
		wobbletime += 0.1f;
		if (gamerunning){
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
			repl_side_ud_left = 10 * (!repl_ani_dir_up ? abs(repl_ani_state) : 1) - 10;
			repl_side_ud_right = 10 * (repl_ani_dir_up ? abs(repl_ani_state) : 1) - 10;
			
			moveProjectiles();
			removeInvalidProjectiles();
			detectCollisions();
			replicatorLogic();
		}
	}

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
	glutMotionFunc(passiveMouseMotion);
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
