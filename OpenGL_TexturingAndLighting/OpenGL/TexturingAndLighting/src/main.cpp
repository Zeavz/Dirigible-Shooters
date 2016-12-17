#include <TextureAndLightingPCH.h>
/*******************************************************************
Multi-Part Model Construction and Manipulation
********************************************************************/

#include <windows.h>
#include <GL/glew.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <utility>
#include <vector>
#include "VECTOR3D.h"
#include "CubeMesh.h"
#include "QuadMesh.h"
using namespace std;

void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void drawDirigible();
void drawBalloon();
void drawDeck();
void drawFins();
void drawPropeller();
void drawFans();
void drawPeriscope();
void drawTorpedo();
void moveForward(int);
bool hitsTarget();
void LoadGLTexture(int x);

VECTOR3D ScreenToWorld(int x, int y);

#define PI 3.141592654
#define CAMX 0.661736
#define CAMY 48.9326
#define CAMZ 34.7155
#define LOOKX 25.0
#define LOOKY 0.0
#define LOOKZ -25.0

static int currentButton;
static unsigned char currentKey;

GLfloat theta = 90.0;
GLfloat fans = 0.0;
GLfloat speed = 10.0;
GLfloat x = 0.0;
GLfloat yvalue = 20.0;
GLfloat z = 10.0;
GLfloat acceleration = 0.00;
GLfloat forwardint = 0.01, aiforwardint = 0.05;
GLfloat spin = 10.0;
GLfloat perry;

GLfloat aix = 15, aiy = yvalue, aiz = -10, aitheta = 90.0;
GLfloat camerax = CAMX, cameray = CAMY, cameraz = CAMZ, lookatx = LOOKX, lookaty = LOOKY, lookatz = LOOKZ;
GLfloat periscopex = x, periscopey = yvalue, periscopez = z, periscopelookx = 100, periscopelookz = 100;
GLfloat misslex, misslez, missley, missletheta;

int caseNumber;
GLuint texture[4];

string fileNames[4] = {"cabin.bmp","balloonTexture.bmp","grass.bmp", "Sandstone.bmp"};

bool pressed, shooting, periscope;

VECTOR3D missle;

GLfloat light_position0[] = { -6.0,  12.0, 0.0,1.0 };
GLfloat light_position1[] = { 6.0,  12.0, 0.0,1.0 };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1.0 };



// Set up lighting/shading and material properties for submarine - upcoming lecture - just copy for now
GLfloat submarine_mat_ambient[] = { 0.4, 0.2, 0.0, 1.0 };
GLfloat submarine_mat_specular[] = { 0.1, 0.1, 0.0, 1.0 };
GLfloat submarine_mat_diffuse[] = { 0.9, 0.5, 0.0, 1.0 };
GLfloat submarine_mat_shininess[] = { 0.0 };

QuadMesh *groundMesh = NULL;
CubeMesh *cubeMesh;
GLUquadricObj *mySphere;


struct BoundingBox
{
	VECTOR3D min;
	VECTOR3D max;
} BBox;

Blobby firstBlob;

char message[19] = "Press 'h' for help";
char message1[56] = "Arrow keys to move, 'q' to accelerate 'e' to decelerate";
char message2[84] = "'d' to toggle periscope view 'w' and 's' to move periscope up and down respectivley";
char message4[71] = "'space' to shoot and 'i' to go into walk through terrain mode on floor";
char message3[18] = "press 'h' to hide";

// Default Mesh Size
int meshSize = 64;

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1000, 1000);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Scene Modeller");	

	initOpenGL(1000, 1000);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotionHandler);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);
	glutTimerFunc(100, moveForward, 0);
	glutMainLoop();
	return 0;
}



// Setup openGL */
void initOpenGL(int w, int h)
{
	LoadGLTexture(0);
	LoadGLTexture(1);
	LoadGLTexture(2);
	LoadGLTexture(3);
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and modeling transforms
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 1.0, 0.2, 500.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.6, 0.6, 0.6, 0.0);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	// This one is important - renormalize normal vectors 
	glEnable(GL_NORMALIZE);

	mySphere = gluNewQuadric();
	gluQuadricDrawStyle(mySphere, GLU_FILL);
	gluQuadricNormals(mySphere, GLU_SMOOTH);
	gluQuadricTexture(mySphere, GLU_TRUE);

	//Nice perspective.
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Set up ground quad mesh
	VECTOR3D origin = VECTOR3D(-8.0f, 0.0f, 8.0f);
	VECTOR3D dir1v = VECTOR3D(1.0f, 0.0f, 0.0f);
	VECTOR3D dir2v = VECTOR3D(0.0f, 0.0f, -1.0f);
	groundMesh = new QuadMesh(meshSize, 16.0);
	groundMesh->InitMesh(meshSize, origin, 64.0, 64.0, dir1v, dir2v);

	VECTOR3D ambient = VECTOR3D(0.0f, 0.05f, 0.0f);
	VECTOR3D diffuse = VECTOR3D(0.4f, 0.8f, 0.4f);
	VECTOR3D specular = VECTOR3D(0.04f, 0.04f, 0.04f);
	float shininess = 0.2;

	groundMesh->SetMaterial(ambient, diffuse, specular, shininess);


	// Set up the bounding box of the scene
	// Currently unused. You could set up bounding boxes for your objects eventually.
	BBox.min.Set(-8.0f, 0.0, -8.0);
	BBox.max.Set(8.0f, 6.0, 8.0);
}

void LoadGLTexture(int x) {
	texture[x] = SOIL_load_OGL_texture(
		fileNames[x].c_str(),
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y);

	glEnable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, texture[x]);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//return texture;
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	// Set up the camera
	gluLookAt(camerax, cameray, cameraz, lookatx, lookaty, lookatz, 0.0, 1.0, 0.0);

	// Draw Submarine


	// Set submarine material properties
	glMaterialfv(GL_FRONT, GL_AMBIENT, submarine_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, submarine_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, submarine_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, submarine_mat_shininess);

	// Apply transformations to move submarine
	// ...

	// Apply transformations to construct submarine

	
	//glEnable(GL_TEXTURE_2D);

	
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	if (!shooting) {
		misslex = x;
		missley = yvalue;
		misslez = z;
		missletheta = theta;
	}
	glTranslatef(misslex, missley, misslez);
	glRotatef(90.0 + missletheta, 0.0, missley, 0.0);
	drawTorpedo();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(x, yvalue, z);
	glRotatef(90.0 + theta, 0.0, yvalue, 0.0);
	drawPeriscope();
	drawDirigible();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(aix, aiy, aiz);
	glRotatef(90.0 + aitheta, 0.0, aiy, 0.0);
	glEnable(GL_TEXTURE_2D);
	drawDirigible();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	// Draw ground
	groundMesh->DrawMesh(meshSize, true);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	

	glRasterPos3f(-5.0, 10.0, -25.0);

	for (int i = 0; i < 19; i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, message[i]);

	if (pressed) {
		glRasterPos3f(-5.0, 8.0, -24.0);

		for (int i = 0; i < 56; i++)
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, message1[i]);

		glRasterPos3f(-5.0, 6.0, -23.0);

		for (int j = 0; j < 84; j++)
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, message2[j]);

		glRasterPos3f(-5.0, 4.0, -23.0);

		for (int l = 0; l < 71; l++)
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, message4[l]);

		glRasterPos3f(-5.0, 2.0, -22.0);

		for (int k = 0; k < 18; k++)
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, message3[k]);
	}

	glutSwapBuffers();
}

void drawDirigible() {
	drawBalloon();
	drawDeck();
	drawFins();
	drawPropeller();
	drawFans();
}
void drawBalloon() {
	glPushMatrix();	
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glScalef(1.0, 1.0, 3.0);
	//gluSphere(mySphere, 1.0, 20.0, 20.0);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glutSolidSphere(1.0, 20.0, 20.0);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	
	glPopMatrix();
}
void drawDeck() {
	glPushMatrix();
	glTranslatef(0.0, -1.0, 0.0);
	glScalef(2.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	glutSolidCube(1.0);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	//glutSolidCube(1.0);
	//drawCube(cubeMesh);
	glPopMatrix();
}
void drawFins() {
	glPushMatrix();
	glTranslatef(1.45, 0.05, 0.0);
	glRotatef(45.0, 1.0, 0.0, 0.0);
	glScalef(1.3, 1.3, 1.3);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glutSolidCube(1.0);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glPopMatrix();
}
void drawPropeller() {
	glPushMatrix();
	glTranslatef(1.3, -1.2, 0.0);
	glScalef(0.1, 0.1, 0.1);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glutSolidSphere(1.0, 20.0, 20.0);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.3, -1.2, 0.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glScalef(0.3, 0.3, 0.9);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glutSolidTorus(0.1, 1.0, 20.0, 20.0);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glPopMatrix();
}

void drawFans() {
	glPushMatrix();
	glTranslatef(1.3, -1.2, 0.0);
	glRotatef(fans, 1.0, 0.0, 0.0);
	glScalef(0.2, 1.0, 0.1);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glutSolidCube(0.5);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.3, -1.2, 0.0);
	glRotatef(90.0 + fans, 1.0, 0.0, 0.0);
	glScalef(0.2, 1.0, 0.1);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glutSolidCube(0.5);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glPopMatrix();
}
void drawPeriscope() {
	glPushMatrix();
	glTranslatef(-0.5, 1.5 + perry, 0.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glScalef(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glutSolidCone(0.25, 0.5, 20, 20);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, 1.0 + perry, 0.0);
	glScalef(0.5, 3.0, 0.5);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glutSolidCube(0.5);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glPopMatrix();
}
void drawTorpedo() {
	glPushMatrix();
	glTranslatef(0.0, -1.0, 0.750);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glScalef(0.50, 0.50, 1.5);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glutSolidSphere(0.50, 20.0, 20.0);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glPopMatrix();
}

// Called at initialization and whenever user resizes the window */
void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 1.0, 0.2, 500.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

VECTOR3D pos = VECTOR3D(0, 0, 0);

// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
	currentButton = button;

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
			;
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;
		}
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

// moving forward code
static void moveForward(int) {
	caseNumber = rand() % 25;
	switch (caseNumber) {
	case 1:
		aitheta += 15.0;
		if (aitheta > 360) {
			aitheta -= 360;
		}
		break;
	case 23:
		aitheta -= 15.0;
		if (aitheta < 0) {
			aitheta += 360;
		}
		break;
	}

	forwardint += acceleration;
	if (forwardint > 0.05) {
		forwardint = 0.05;
	}
	if (forwardint < -0.05) {
		forwardint = -0.05;
	}

	aiforwardint += aiforwardint;
	if (aiforwardint > 0.08) {
		aiforwardint = 0.08;
	}

	x += forwardint * sin((theta / 180)*PI);
	z += forwardint * cos((theta / 180)*PI);

	aix += aiforwardint * sin((aitheta / 180)*PI);
	aiz += aiforwardint * cos((aitheta / 180)*PI);

	periscopex = x;
	periscopez = z;

	periscopelookx = 100 * sin((theta / 180)*PI);
	periscopelookz = 100 * cos((theta / 180)*PI);

	if (periscope) {
		camerax = periscopex;
		cameray = periscopey + perry + yvalue;
		cameraz = periscopez;
		lookatx = periscopelookx;
		lookaty = periscopey + perry + yvalue;
		lookatz = periscopelookz;
	}
	else {
		camerax = CAMX;
		cameray = CAMY;
		cameraz = CAMZ;
		lookatx = LOOKX;
		lookaty = LOOKY;
		lookatz = LOOKZ;
	}

	if (x > 48.0) {
		x = 48.0;
	}
	if (x < -8.0) {
		x = -8.0;
	}
	if (z > 8.0) {
		z = 8.0;
	}
	if (z < -48.0) {
		z = -48.0;
	}

	if (aix > 48.0) {
		aix = 48.0;
		aitheta += 180;
	}
	if (aix < -8.0) {
		aix = -8.0;
		aitheta += 180;
	}
	if (aiz > 8.0) {
		aiz = 8.0;
		aitheta += 180;
	}
	if (aiz < -48.0) {
		aiz = -48.0;
		aitheta += 180;
	}

	fans += forwardint * 90.0;
	if (fans > 360) {
		fans -= 360;
	}
	if (fans < -360) {
		fans += 360;
	}

	if (shooting) {
		misslex += 5.0 * sin((missletheta / 180)*PI);
		misslez += 5.0 * cos((missletheta / 180)*PI);
		if (misslex > 48.0 || misslex < -8.0 || misslez > 8.0 || misslez < -48.0 || hitsTarget()) {
			shooting = false;
		}
	}

	glutTimerFunc(100, moveForward, 0);
	glutPostRedisplay();
}

bool hitsTarget() {
	GLfloat subx, suby, subz;
	subx = aix - misslex;
	suby = aiy - missley;
	subz = aiz - misslez;
	if ((subx*subx + suby*suby + subz*subz) < 6.5) {
		aix = (rand() % 57) - 8;
		aiz = -((rand() % 57) - 8);
		return true;
	}
	return false;
}

// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{
	if (currentButton == GLUT_LEFT_BUTTON)
	{
		;
	}
	glutPostRedisplay();
}


VECTOR3D ScreenToWorld(int x, int y)
{
	// you will need this if you use the mouse
	return VECTOR3D(0);
}// ScreenToWorld()

 /* Handles input from the keyboard, non-arrow keys */
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
		acceleration += 0.001;
		if (acceleration > 0.002)
			acceleration = 0.002;
		break;
	case 'e':
		acceleration -= 0.001;
		if (acceleration < -0.002)
			acceleration = -0.002;
		break;

	case 'w':
		perry += 0.01;
		if (perry > 0.75) {
			perry = 0.75;
		}
		break;
	case 'i':
		yvalue = 1.0;
		break;
	case 's':
		perry -= 0.01;
		if (perry < -0.20) {
			perry = -0.20;
		}
		break;
	case 'a':
		break;
	case 'd':
		periscope = !periscope;
		break;

	case 'h':
		pressed = !pressed;
		break;
	case ' ':
		shooting = !shooting;
		break;
	}
	glutPostRedisplay();
}

void functionKeys(int key, int x, int y)
{
	VECTOR3D min, max;

	if (key == GLUT_KEY_LEFT)
	{
		theta += 5.0;
		if (theta > 360.0)
			theta -= 360.0;
		glutPostRedisplay();
	}
	if (key == GLUT_KEY_RIGHT)
	{
		theta -= 5.0;
		if (theta < 0.0)
			theta += 360.0;
		glutPostRedisplay();
	}
	if (key == GLUT_KEY_UP)
	{
		yvalue += 0.35;
		if (yvalue > 25.0) {
			yvalue = 25.0;
		}
		glutPostRedisplay();
	}
	if (key == GLUT_KEY_DOWN)
	{
		yvalue -= 0.35;
		if (yvalue < 19.0) {
			yvalue = 19.0;
		}
		glutPostRedisplay();
	}
	if (key == GLUT_KEY_F1) {
		camerax = CAMX;
		cameray = CAMY;
		cameraz = CAMZ;
		lookatx = LOOKX;
		lookaty = LOOKY;
		lookatz = LOOKZ;
	}
	/*
	// Do transformation code with arrow keys
	// GLUT_KEY_DOWN, GLUT_KEY_UP,GLUT_KEY_RIGHT, GLUT_KEY_LEFT
	else if (...)
	{
	}
	*/
	glutPostRedisplay();
}


