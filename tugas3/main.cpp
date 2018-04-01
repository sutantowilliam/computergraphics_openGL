#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
 
bool fullscreen = false;
bool mouseDown = false;
 
float xrot = 0.0f;
float yrot = 0.0f;
 
float xdiff = 0.0f;
float ydiff = 0.0f;


#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 1000



float convertPositionX(int X) {
	static float zeroPosition = SCREEN_WIDTH / 2;
	return (X - zeroPosition) / zeroPosition;
}

float convertPositionY(int Y) {
	static float zeroPosition = SCREEN_HEIGHT / 2;
	return ((SCREEN_HEIGHT - Y) - zeroPosition) / zeroPosition;
}


float velgBackX = convertPositionX(317);
float velgBackY = convertPositionY(800);

float velgFrontX = convertPositionX(717);
float velgFrontY = convertPositionY(800);


void drawCircle(GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLfloat red, GLfloat green, GLfloat blue, int triangleAmount, bool clockwise){
	int i;
	
	//GLfloat radius = 0.8f; //radius
	GLfloat twicePi = 2.0f * 3.14;
	if (clockwise) {
		twicePi *= -1;
	}
	glColor3f (red+0.3f, green+0.3f, blue+0.3f);
	glBegin(GL_TRIANGLE_FAN);
		glVertex3f(x, y, z); // center of circle
        glColor3f (red, green, blue);
		for(i = 0; i <= triangleAmount+1;i++) {
			glVertex3f(
					x + (radius * cos(i *  twicePi / triangleAmount)), 
				y + (radius * sin(i * twicePi / triangleAmount)), z
			);
		}
	glEnd();
}

void drawWheelWidth(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat radius, GLfloat red, GLfloat green, GLfloat blue, int triangleAmount, bool clockwise){
	int i;
	
	//GLfloat radius = 0.8f; //radius
	GLfloat twicePi = 2.0f * 3.14;
	if (clockwise) {
		twicePi *= -1;
	}
	glColor3f (red, green, blue);
    for(i = 0; i <= triangleAmount+1;i++){
        glBegin(GL_POLYGON);
            glVertex3f(x + (radius * cos(i *  twicePi / triangleAmount)), y + (radius * sin(i * twicePi / triangleAmount)), z);
            glVertex3f(x + (radius * cos((i+1) *  twicePi / triangleAmount)), y + (radius * sin((i+1) * twicePi / triangleAmount)), z);
            glVertex3f(x + (radius * cos((i+1) *  twicePi / triangleAmount)), y + (radius * sin((i+1) * twicePi / triangleAmount)), z-width);
            glVertex3f(x + (radius * cos(i *  twicePi / triangleAmount)), y + (radius * sin(i * twicePi / triangleAmount)), z-width);
	    glEnd();
    }
    
}

void drawCar() {
	// Left Side
	glColor3f(-0.4f, 0.85f, 0.6f);
	glBegin(GL_POLYGON);
		glVertex3f(convertPositionX(155), convertPositionY(646), -0.4f);
		glVertex3f(convertPositionX(155), convertPositionY(807), -0.4f);
		glVertex3f(convertPositionX(875), convertPositionY(807), -0.4f);
		glVertex3f(convertPositionX(859), convertPositionY(610), -0.4f);
		glVertex3f(convertPositionX(740), convertPositionY(579), -0.4f);
		glVertex3f(convertPositionX(625), convertPositionY(370), -0.4f);
		glVertex3f(convertPositionX(255), convertPositionY(370), -0.4f);
		
	glEnd();
	
	// Right Side
	glBegin(GL_POLYGON);
		glVertex3f(convertPositionX(255), convertPositionY(370), 0.4f);
		glVertex3f(convertPositionX(625), convertPositionY(370), 0.4f);
		glVertex3f(convertPositionX(740), convertPositionY(579), 0.4f);
		glVertex3f(convertPositionX(859), convertPositionY(610), 0.4f);
		glVertex3f(convertPositionX(875), convertPositionY(807), 0.4f);
		glVertex3f(convertPositionX(155), convertPositionY(807), 0.4f);
		glVertex3f(convertPositionX(155), convertPositionY(646), 0.4f);
	glEnd();

	// Roof
	glBegin(GL_POLYGON);
		glVertex3f(convertPositionX(255), convertPositionY(370), 0.4f);
		glVertex3f(convertPositionX(255), convertPositionY(370), -0.4f);
		glVertex3f(convertPositionX(625), convertPositionY(370), -0.4f);
		glVertex3f(convertPositionX(625), convertPositionY(370), 0.4f);
	glEnd();

	// Front Window Body
	glBegin(GL_POLYGON);
		glVertex3f(convertPositionX(625), convertPositionY(370), 0.4f);
		glVertex3f(convertPositionX(625), convertPositionY(370), -0.4f);
		glVertex3f(convertPositionX(740), convertPositionY(579), -0.4f);
		glVertex3f(convertPositionX(740), convertPositionY(579), 0.4f);
	glEnd();

	// Hood
	glBegin(GL_POLYGON);
		glVertex3f(convertPositionX(740), convertPositionY(579), 0.4f);
		glVertex3f(convertPositionX(740), convertPositionY(579), -0.4f);
		glVertex3f(convertPositionX(859), convertPositionY(610), -0.4f);
		glVertex3f(convertPositionX(859), convertPositionY(610), 0.4f);
	glEnd();

	// Front Bumper
	glBegin(GL_POLYGON);
		glVertex3f(convertPositionX(859), convertPositionY(610), 0.4f);
		glVertex3f(convertPositionX(859), convertPositionY(610), -0.4f);
		glVertex3f(convertPositionX(875), convertPositionY(807), -0.4f);
		glVertex3f(convertPositionX(875), convertPositionY(807), 0.4f);
	glEnd();


	// Back Window Body
	glBegin(GL_POLYGON);
		glVertex3f(convertPositionX(255), convertPositionY(370), -0.4f);
		glVertex3f(convertPositionX(255), convertPositionY(370), 0.4f);
		glVertex3f(convertPositionX(155), convertPositionY(646), 0.4f);
		glVertex3f(convertPositionX(155), convertPositionY(646), -0.4f);
	glEnd();

	//Back Bumper
	glBegin(GL_POLYGON);
		glVertex3f(convertPositionX(155), convertPositionY(646), -0.4f);
		glVertex3f(convertPositionX(155), convertPositionY(646), 0.4f);
		glVertex3f(convertPositionX(155), convertPositionY(807), 0.4f);
		glVertex3f(convertPositionX(155), convertPositionY(807), -0.4f);
	glEnd();

	// Underbody
	glBegin(GL_POLYGON);
        glColor3f(0.2f, 0.2f, 0.2f);
		glVertex3f(convertPositionX(155), convertPositionY(807), -0.4f);
		glVertex3f(convertPositionX(155), convertPositionY(807), 0.4f);
		glVertex3f(convertPositionX(875), convertPositionY(807), 0.4f);
		glVertex3f(convertPositionX(875), convertPositionY(807), -0.4f);
	glEnd();

    
	// Front Windows
	glBegin(GL_POLYGON);
		glColor3f(0.8f, 0.8f, 0.8f);
		glVertex3f(convertPositionX(632), convertPositionY(380), 0.375f);

		glColor3f(1, 1, 1);
		glVertex3f(convertPositionX(632), convertPositionY(380), -0.375f);
		glVertex3f(convertPositionX(735), convertPositionY(562), -0.375f);
		glVertex3f(convertPositionX(735), convertPositionY(562), 0.375f);
	glEnd();

	//Back Right Window
	glBegin(GL_POLYGON);	
		glColor3f(0.8f, 0.8f, 0.8f);
		glVertex3f(convertPositionX(269), convertPositionY(382), 0.4001f);

		glColor3f(1, 1, 1);
		glVertex3f(convertPositionX(444), convertPositionY(382), 0.4001f);
		glVertex3f(convertPositionX(444), convertPositionY(562), 0.4001f);
		glVertex3f(convertPositionX(197), convertPositionY(562), 0.4001f);
	glEnd();

	//Front Right Window
	glBegin(GL_POLYGON);	
        glColor3f(1, 1, 1);
		glVertex3f(convertPositionX(478), convertPositionY(382), 0.4001f);
		glVertex3f(convertPositionX(613), convertPositionY(382), 0.4001f);
		glVertex3f(convertPositionX(716), convertPositionY(562), 0.4001f);
		
        glColor3f(0.8f, 0.8f, 0.8f);
        glVertex3f(convertPositionX(477), convertPositionY(562), 0.4001f);
	glEnd();

	//Back Left Window
	glBegin(GL_POLYGON);	
		glColor3f(1, 1, 1);
        glVertex3f(convertPositionX(197), convertPositionY(562), -0.4001f);
		glVertex3f(convertPositionX(444), convertPositionY(562), -0.4001f);
		glVertex3f(convertPositionX(444), convertPositionY(382), -0.4001f);
        
        glColor3f(0.8f, 0.8f, 0.8f);
		glVertex3f(convertPositionX(269), convertPositionY(382), -0.4001f);
	glEnd();

	//Front Left Window
	glBegin(GL_POLYGON);	
		glColor3f(0.8f, 0.8f, 0.8f);
		glVertex3f(convertPositionX(477), convertPositionY(562), -0.4001f);

		glColor3f(1, 1, 1);
		glVertex3f(convertPositionX(716), convertPositionY(562), -0.4001f);
		glVertex3f(convertPositionX(613), convertPositionY(382), -0.4001f);
		glVertex3f(convertPositionX(478), convertPositionY(382), -0.4001f);
	glEnd();

	//Back Window
	glBegin(GL_POLYGON);
		glColor3f(0.8f, 0.8f, 0.8f);
		glVertex3f(convertPositionX(250), convertPositionY(380), -0.375f);

		glColor3f(1, 1, 1);
		glVertex3f(convertPositionX(250), convertPositionY(380), 0.375f);
		glVertex3f(convertPositionX(182), convertPositionY(562), 0.375f);
		glVertex3f(convertPositionX(182), convertPositionY(562), -0.375f);
	glEnd();

    //Left Headlight
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(1, 1, 1);
        glVertex3f(convertPositionX(861), convertPositionY(625), 0.3f);
        
        glColor3f(0.8f, 0.8f, 0.8f);
        glVertex3f(convertPositionX(860), convertPositionY(610), 0.4f);
        glVertex3f(convertPositionX(860), convertPositionY(610), 0.2f);
        glVertex3f(convertPositionX(862), convertPositionY(640), 0.2f);
        glVertex3f(convertPositionX(862), convertPositionY(640), 0.4f);
        glVertex3f(convertPositionX(860), convertPositionY(610), 0.4f);
    glEnd();

    //Right Headlight
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(1, 1, 1);
        glVertex3f(convertPositionX(861), convertPositionY(625), -0.3f);
        
        glColor3f(0.8f, 0.8f, 0.8f);
        glVertex3f(convertPositionX(860), convertPositionY(610), -0.4f);
        glVertex3f(convertPositionX(860), convertPositionY(610), -0.2f);
        glVertex3f(convertPositionX(862), convertPositionY(640), -0.2f);
        glVertex3f(convertPositionX(862), convertPositionY(640), -0.4f);
        glVertex3f(convertPositionX(860), convertPositionY(610), -0.4f);
    glEnd();

    //Left Taillight
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(1, 0, 0);
        glVertex3f(convertPositionX(154.999), convertPositionY(661), -0.3f);

        glColor3f(0.8f, 0, 0);
        glVertex3f(convertPositionX(154.999), convertPositionY(646), -0.4f);
        glVertex3f(convertPositionX(154.999), convertPositionY(646), -0.2f);
        glVertex3f(convertPositionX(154.999), convertPositionY(676), -0.2f);
        glVertex3f(convertPositionX(154.999), convertPositionY(676), -0.4f);
        glVertex3f(convertPositionX(154.999), convertPositionY(646), -0.4f);
    glEnd();

    //Right Taillight
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(1, 0, 0);
        glVertex3f(convertPositionX(154.999), convertPositionY(661), 0.3f);

        glColor3f(0.8f, 0, 0);
        glVertex3f(convertPositionX(154.999), convertPositionY(646), 0.4f);
        glVertex3f(convertPositionX(154.999), convertPositionY(646), 0.2f);
        glVertex3f(convertPositionX(154.999), convertPositionY(676), 0.2f);
        glVertex3f(convertPositionX(154.999), convertPositionY(676), 0.4f);
        glVertex3f(convertPositionX(154.999), convertPositionY(646), 0.4f);
    glEnd();

    //Front Grill
    glBegin(GL_POLYGON);
        glColor3f(0.2f, 0.2f, 0.2f);
        glVertex3f(convertPositionX(862), convertPositionY(650), -0.3f);
		glVertex3f(convertPositionX(862), convertPositionY(650), 0.3f);
		glVertex3f(convertPositionX(874), convertPositionY(790), 0.25f);
		glVertex3f(convertPositionX(874), convertPositionY(790), -0.25f);
    glEnd();
}

void drawWheel(float zPosition, float width) {
	drawCircle(convertPositionX (317), convertPositionY (800), zPosition, 0.15f, 0, 0, 0, 1000, true);
	drawCircle(convertPositionX (717), convertPositionY (800), zPosition, 0.15f, 0, 0, 0, 1000, true);
	drawCircle(velgBackX, velgBackY, zPosition+0.001f, 0.1f, 0.4f, 0.4f, 0.4f, 6, true);
	drawCircle(velgFrontX, velgFrontY, zPosition+0.001f, 0.1f, 0.4f, 0.4f, 0.4f, 6, true);

    //Wheel Inside 
    drawCircle(convertPositionX (317), convertPositionY (800), zPosition-width, 0.15f, 0, 0, 0, 1000, true);
	drawCircle(convertPositionX (717), convertPositionY (800), zPosition-width, 0.15f, 0, 0, 0, 1000, true);
	drawCircle(velgBackX, velgBackY, zPosition-width-0.001f, 0.1f, 0.4f, 0.4f, 0.4f, 6, true);
	drawCircle(velgFrontX, velgFrontY, zPosition-width-0.001f, 0.1f, 0.4f, 0.4f, 0.4f, 6, true);

    drawWheelWidth(convertPositionX (317), convertPositionY (800), zPosition, width, 0.15f, 0, 0, 0, 1000, true);
    drawWheelWidth(convertPositionX (717), convertPositionY (800), zPosition, width, 0.15f, 0, 0, 0, 1000, true);

    drawCircle(convertPositionX (317), convertPositionY (800), -zPosition, 0.15f, 0, 0, 0, 1000, true);
	drawCircle(convertPositionX (717), convertPositionY (800), -zPosition, 0.15f, 0, 0, 0, 1000, true);
	drawCircle(velgBackX, velgBackY, -zPosition-0.001f, 0.1f, 0.4f, 0.4f, 0.4f, 6, true);
	drawCircle(velgFrontX, velgFrontY, -zPosition-0.001f, 0.1f, 0.4f, 0.4f, 0.4f, 6, true);

    //Wheel Inside
    drawCircle(convertPositionX (317), convertPositionY (800), -zPosition+width, 0.15f, 0, 0, 0, 1000, true);
	drawCircle(convertPositionX (717), convertPositionY (800), -zPosition+width, 0.15f, 0, 0, 0, 1000, true);
	drawCircle(velgBackX, velgBackY, -zPosition+width+0.001f, 0.1f, 0.4f, 0.4f, 0.4f, 6, true);
	drawCircle(velgFrontX, velgFrontY, -zPosition+width+0.001f, 0.1f, 0.4f, 0.4f, 0.4f, 6, true);

    drawWheelWidth(convertPositionX (317), convertPositionY (800), -zPosition, -width, 0.15f, 0, 0, 0, 1000, true);
    drawWheelWidth(convertPositionX (717), convertPositionY (800), -zPosition, -width, 0.15f, 0, 0, 0, 1000, true);
}
 
void drawBox()
{
	glBegin(GL_QUADS);
	
	glColor3f(1.0f, 0.0f, 0.0f);
	// FRONT
	glVertex3f(-0.4f, -0.4f, 0.4f);
	glVertex3f( 0.4f, -0.4f, 0.4f);
	glVertex3f( 0.4f, 0.4f, 0.4f);
	glVertex3f(-0.4f, 0.4f, 0.4f);
	// BACK
	glVertex3f(-0.4f, -0.4f, -0.4f);
	glVertex3f(-0.4f, 0.4f, -0.4f);
	glVertex3f( 0.4f, 0.4f, -0.4f);
	glVertex3f( 0.4f, -0.4f, -0.4f);
	
	glColor3f(0.0f, 1.0f, 0.0f);
	// LEFT
	glVertex3f(-0.4f, -0.4f, 0.4f);
	glVertex3f(-0.4f, 0.4f, 0.4f);
	glVertex3f(-0.4f, 0.4f, -0.4f);
	glVertex3f(-0.4f, -0.4f, -0.4f);
	// RIGHT
	glVertex3f( 0.4f, -0.4f, -0.4f);
	glVertex3f( 0.4f, 0.4f, -0.4f);
	glVertex3f( 0.4f, 0.4f, 0.4f);
	glVertex3f( 0.4f, -0.4f, 0.4f);
	
	glColor3f(0.0f, 0.0f, 1.0f);
	// TOP
	glVertex3f(-0.4f, 0.4f, 0.4f);
	glVertex3f( 0.4f, 0.4f, 0.4f);
	glVertex3f( 0.4f, 0.4f, -0.4f);
	glVertex3f(-0.4f, 0.4f, -0.4f);
	// BOTTOM
	glVertex3f(-0.4f, -0.4f, 0.4f);
	glVertex3f(-0.4f, -0.4f, -0.4f);
	glVertex3f( 0.4f, -0.4f, -0.4f);
	glVertex3f( 0.4f, -0.4f, 0.4f);
	glEnd();
}
 
bool init() {
	glClearColor(0.93f, 0.93f, 0.93f, 0.0f);
	
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);
	
	return true;
}
 
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	gluLookAt(
	0.0f, 0.0f, 3.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f);
	
	glRotatef(xrot, 1.0f, 0.0f, 0.0f);
	glRotatef(yrot, 0.0f, 1.0f, 0.0f);
	
	drawCar();
	drawWheel(0.401f,0.075f);
	

	glFlush();
	glutSwapBuffers();
}
 
void resize(int w, int h){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glViewport(0, 0, w, h);
	
	gluPerspective(45.0f, 1.0f * w / h, 1.0f, 100.0f);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
	
void idle(){
	if (!mouseDown) {
		xrot += 0.3f;
		yrot += 0.4f;
	}
	
	glutPostRedisplay();
}
 
void keyboard(unsigned char key, int x, int y){
	switch(key)	{
		case 27 : 
			exit(0.4f); 
			break;
	}
}
 
void specialKeyboard(int key, int x, int y)
{
	if (key == GLUT_KEY_F1)	{
		fullscreen = !fullscreen;
		
		if (fullscreen) {
			glutFullScreen();
		} else {
			glutReshapeWindow(500, 500);
			glutPositionWindow(50, 50);
		}
	}
}
 
void mouse(int button, int state, int x, int y){
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		mouseDown = true;
		
		xdiff = x - yrot;
		ydiff = -y + xrot;
	} else
		mouseDown = false;
}
 
void mouseMotion(int x, int y) {
	if (mouseDown) {
		yrot = x - xdiff;
		xrot = y + ydiff;
		
		glutPostRedisplay();
	}
}
 
int main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	
	glutCreateWindow("Tugas 3");
	
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(resize);
	//glutIdleFunc(idle);
	
	if (!init())
	return 1;
	
	glutMainLoop();
	
	return 0;
}