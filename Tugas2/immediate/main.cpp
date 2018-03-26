#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
using namespace std;
using namespace glm;


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

float rotate_angle = 0;

void drawCircle(GLfloat x, GLfloat y, GLfloat radius, GLfloat red, GLfloat green, GLfloat blue, int triangleAmount){
	int i;
	
	//GLfloat radius = 0.8f; //radius
	GLfloat twicePi = 2.0f * 3.14;
	glColor3f (red, green, blue);

	glTranslatef(x, y,0);
	glRotatef(rotate_angle, 0, 0, 1);
	glTranslatef(-x, -y,0);

	glBegin(GL_TRIANGLE_FAN);
		glVertex3f(x, y, 0); // center of circle
		for(i = 0; i <= triangleAmount;i++) {
			glVertex3f(
					x + (radius * cos(i *  twicePi / triangleAmount)), 
				y + (radius * sin(i * twicePi / triangleAmount)), 0
			);
		}
	glEnd();

	glTranslatef(x, y,0);
	glRotatef(-rotate_angle, 0, 0, 1);
	glTranslatef(-x, -y,0);

}

void drawCar() {
	//Car Outline
	glColor3f(0.2f, 0.85f, 0.6f);
	glBegin(GL_POLYGON);
		glVertex3f(convertPositionX(255), convertPositionY(370), 0.2f);
		glVertex3f(convertPositionX(625), convertPositionY(370), 0.2f);
		glVertex3f(convertPositionX(740), convertPositionY(579), 0.2f);
		glVertex3f(convertPositionX(859), convertPositionY(610), 0.2f);
		glVertex3f(convertPositionX(875), convertPositionY(807), 0.2f);
		glVertex3f(convertPositionX(155), convertPositionY(807), 0.2f);
		glVertex3f(convertPositionX(155), convertPositionY(646), 0.2f);
	glEnd();
	
	//Back Window
	glBegin(GL_POLYGON);	
		glColor3f(0.8f, 0.8f, 0.8f);
		glVertex3f(convertPositionX(269), convertPositionY(382), 1);

		glColor3f(1, 1, 1);
		glVertex3f(convertPositionX(444), convertPositionY(382), 1);
		glVertex3f(convertPositionX(444), convertPositionY(581), 1);
		glVertex3f(convertPositionX(197), convertPositionY(581), 1);
	glEnd();

	//Front Window
	glBegin(GL_POLYGON);	
		glColor3f(0.8f, 0.8f, 0.8f);
		glVertex3f(convertPositionX(478), convertPositionY(382), 1);

		glColor3f(1, 1, 1);
		glVertex3f(convertPositionX(613), convertPositionY(382), 1);
		glVertex3f(convertPositionX(716), convertPositionY(581), 1);
		glVertex3f(convertPositionX(477), convertPositionY(581), 1);
	glEnd();
}

void drawWheel() {
	drawCircle(convertPositionX (317), convertPositionY (800), 0.15f, 0, 0, 0, 1000);
	drawCircle(convertPositionX (717), convertPositionY (800), 0.15f, 0, 0, 0, 1000);

	drawCircle(velgBackX, velgBackY, 0.1f, 0.5f, 0.5f, 0.5f, 5);
	drawCircle(velgFrontX, velgFrontY, 0.1f, 0.5f, 0.5f, 0.5f, 5);
}

void rotate(){
	rotate_angle -= 1.5;
}

int main() {
    if (!glfwInit()) {
            fprintf(stderr, "Failed to initialize GLFW\n");
            getchar();
            return -1;
    }
	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tugas 2", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

    do{
		glClear( GL_COLOR_BUFFER_BIT );

		drawCar();
		drawWheel();
		rotate();

		glfwSwapBuffers(window);
		glfwPollEvents();

	}
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

    return 0;
}