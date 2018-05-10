#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;


#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>

#include <iostream>


#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 1000
#define WHEEL_TRIANGLE_AMOUNT 1000
#define VELG_TRIANGLE_AMOUNT 6

// CPU representation of a particle
struct Particle {
	glm::vec3 pos, speed;
	unsigned char r, g, b, a; // Color
	float size, angle, weight;
	float life; // Remaining life of the particle. if <0 : dead and unused.
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};

//for collision detection
struct AABB {
	glm::vec3 maxPosition;
	glm::vec3 minPosition;
};

bool checkCollision(AABB obj1, AABB obj2){
	bool checkX = (obj1.maxPosition.x > obj2.minPosition.x) && (obj1.minPosition.x < obj2.maxPosition.x);
	bool checkY = (obj1.maxPosition.y > obj2.minPosition.y) && (obj1.minPosition.x < obj2.maxPosition.y);
	bool checkZ = (obj1.maxPosition.z > obj2.minPosition.z) && (obj1.minPosition.x < obj2.maxPosition.z);
	return checkX && checkY && checkZ;
}

bool checkRainCollision(AABB obj1, vec3 point) {
	bool checkX = point.x > obj1.minPosition.x && point.x < obj1.maxPosition.x;
	bool checkY = point.y > obj1.minPosition.y && point.y < obj1.maxPosition.y;
	bool checkZ = point.z > obj1.minPosition.z && point.z < obj1.maxPosition.z;
	return checkX && checkY && checkZ;
}

const int MaxParticles = 1000;
Particle ParticlesContainer[MaxParticles];
int LastUsedParticle = 0;

// Finds a Particle in ParticlesContainer which isn't used yet.
// (i.e. life < 0);
int FindUnusedParticle() {

	for (int i = LastUsedParticle; i<MaxParticles; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = i;
			return i;
		}
	}

	for (int i = 0; i<LastUsedParticle; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = i;
			return i;
		}
	}

	return 0; // All particles are taken, override the first one
}

void SortParticles() {
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

vec3 normalizeVertex(vec3 v1, vec3 v2, vec3 v3) {
	return cross((v2 - v1), (v3 - v1));
}

float convertPositionX(int X) {
	static float zeroPosition = SCREEN_WIDTH / 2;
	return (X - zeroPosition) / zeroPosition;
}

float convertPositionY(int Y) {
	static float zeroPosition = SCREEN_HEIGHT / 2;
	return ((SCREEN_HEIGHT - Y) - zeroPosition) / zeroPosition;
}

void generateWheelNormal(GLfloat* circleNormal, int triangleAmount, bool clockwise) {
	int i;

	for (i = 0; i <= triangleAmount; ++i) {
		int index = i * 3;
		circleNormal[index] = 0;
		circleNormal[index + 1] = 0;
		circleNormal[index + 2] = !clockwise;
		int offsetedIndex = index + triangleAmount + 1;
		circleNormal[offsetedIndex] = 0;
		circleNormal[offsetedIndex + 1] = 0;
		circleNormal[offsetedIndex + 2] = clockwise;
	}
}

void generateWheelUV(GLfloat* circleUV, GLfloat radius, int triangleAmount) {
	int i;

	GLfloat twicePi = 2.0f * 3.14159265359;

	int indexOffset = (triangleAmount + 1) * 2;

	circleUV[0] = 0.5;
	circleUV[1] = 0.5;
	circleUV[indexOffset] = 0.5;
	circleUV[indexOffset + 1] = 0.5;
	for (i = 0; i < triangleAmount; ++i) {
		int index = (i + 1) * 2;
		circleUV[index] = 0.5 + (radius * cos(i * twicePi / triangleAmount));
		circleUV[index + 1] = 0.5 + (radius * sin(i * twicePi / triangleAmount));
		int offsetedIndex = index + indexOffset;
		circleUV[offsetedIndex] = 0.5 + (radius * cos(i * -twicePi / triangleAmount));
		circleUV[offsetedIndex + 1] = 0.5 + (radius * sin(i * -twicePi / triangleAmount));
	}
}

void generateWheelVertexes(GLfloat* circleData, GLuint* elementData, GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat radius, int triangleAmount, bool clockwise) {
	int i;

	GLfloat twicePi = 2.0f * 3.14159265359;
	if (!clockwise) {
		twicePi = -twicePi;
	}
	else {
		width = -width;
	}

	int indexOffset = (triangleAmount + 1) * 3;

	circleData[0] = x;
	circleData[1] = y;
	circleData[2] = z;
	circleData[indexOffset] = x;
	circleData[indexOffset + 1] = y;
	circleData[indexOffset + 2] = z + width;
	for (i = 0; i < triangleAmount; i++) {
		int index = (i + 1) * 3;
		circleData[index] = x + (radius * cos(i *  twicePi / triangleAmount));
		circleData[index + 1] = y + (radius * sin(i * twicePi / triangleAmount));
		circleData[index + 2] = z;
		int offsetedIndex = index + indexOffset;
		// std::cout << "i " << i << " index " << index << " offset " << offsetedIndex << std::endl;
		circleData[offsetedIndex] = x + (radius * cos(i *  -twicePi / triangleAmount));
		circleData[offsetedIndex + 1] = y + (radius * sin(i * -twicePi / triangleAmount));
		circleData[offsetedIndex + 2] = z + width;
	}

	indexOffset -= 3;

	for (i = 0; i < triangleAmount - 1; i++) {
		int index = i * 3;
		elementData[index] = 0;
		elementData[index + 1] = i + 1;
		elementData[index + 2] = i + 2;
		int offsetedIndex = index + indexOffset;
		// std::cout << "index " << index << " offset " << offsetedIndex << std::endl;
		elementData[offsetedIndex] = triangleAmount + 1;
		elementData[offsetedIndex + 1] = triangleAmount + 1 + i + 1;
		elementData[offsetedIndex + 2] = triangleAmount + 1 + i + 2;
	}

	int index = (triangleAmount - 1) * 3;
	elementData[index] = 0;
	elementData[index + 1] = triangleAmount;
	elementData[index + 2] = 1;
	int offsetedIndex = index + indexOffset;
	elementData[offsetedIndex] = triangleAmount + 1;
	elementData[offsetedIndex + 1] = triangleAmount + 1 + triangleAmount;
	elementData[offsetedIndex + 2] = triangleAmount + 1 + 1;

	int reversedIndex = (triangleAmount + 1) * 2;
	for (i = 1; i < triangleAmount; ++i) {
		elementData[(i - 1) * 6 + offsetedIndex + 3] = reversedIndex - i;
		elementData[(i - 1) * 6 + offsetedIndex + 3 + 1] = i + 1;
		elementData[(i - 1) * 6 + offsetedIndex + 3 + 2] = i;
		elementData[(i - 1) * 6 + offsetedIndex + 3 + 3] = reversedIndex - i - 1;
		elementData[(i - 1) * 6 + offsetedIndex + 3 + 4] = i + 1;
		elementData[(i - 1) * 6 + offsetedIndex + 3 + 5] = reversedIndex - i;
	}
	index = (triangleAmount - 1) * 6 + offsetedIndex + 3;
	elementData[index] = triangleAmount + 2;
	elementData[index + 1] = 1;
	elementData[index + 2] = triangleAmount;
	elementData[index + 3] = reversedIndex - 1;
	elementData[index + 4] = 1;
	elementData[index + 5] = triangleAmount + 2;
}

void rotate(GLfloat x0, GLfloat y0, GLfloat* x1, GLfloat* y1, float angle) {
	angle *= 3.14159265359 / 180;
	GLfloat tempX = (*x1 - x0) * cos(angle) - (*y1 - y0) * sin(angle) + x0;
	GLfloat tempY = (*y1 - y0) * cos(angle) + (*x1 - x0) * sin(angle) + y0;
	*x1 = tempX;
	*y1 = tempY;
}

void moveCarHorizontal(int speed) {
	// std::cout << sizeof(g_vertex_buffer_data) / sizeof(GL_FLOAT) << std::endl;
	// for (int i = 0; i < sizeof(g_vertex_buffer_data) / sizeof(GL_FLOAT); i++ ) {

	// 	g_vertex_buffer_data[i * 3] += speed;
	// }
}


int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}
	/*
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	*/

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Tutorial 18 - Particules", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Dark blue background
	glClearColor(1.0f, 1.0f, 1, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);


	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("Particle.vertexshader", "Particle.fragmentshader");

	// Vertex shader
	GLuint CameraRight_worldspace_ID = glGetUniformLocation(programID, "CameraRight_worldspace");
	GLuint CameraUp_worldspace_ID = glGetUniformLocation(programID, "CameraUp_worldspace");
	GLuint ViewProjMatrixID = glGetUniformLocation(programID, "VP");

	// fragment shader
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Create and compile our GLSL program from the shaders
	GLuint carID = LoadShaders("TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(carID, "MVP");
	GLuint ViewPosID = glGetUniformLocation(carID, "viewPos");

	// Load the texturef
	GLuint sideTexture = loadBMP_custom("Side Body.bmp");
	GLuint frontBumperTexture = loadBMP_custom("Front Bumper.bmp");
	GLuint hoodTexture = loadBMP_custom("Hood.bmp");
	GLuint rearBumperTexture = loadBMP_custom("Rear Bumper.bmp");
	GLuint windowTexture = loadBMP_custom("Window.bmp");
	GLuint blackTexture = loadBMP_custom("Black.bmp");
	GLuint wheelTexture = loadBMP_custom("Wheel.bmp");
	GLuint roadTexture = loadBMP_custom("Road.bmp");
	GLuint grassTexture = loadBMP_custom("Grass.bmp");
	GLuint itemTexture = loadBMP_custom("Item.bmp");


	static GLfloat* g_particule_position_size_data = new GLfloat[MaxParticles * 4];
	static GLubyte* g_particule_color_data = new GLubyte[MaxParticles * 4];

	for (int i = 0; i<MaxParticles; i++) {
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
	}

	GLuint Texture = loadDDS("particle.DDS");

	//TABB for collision detection
	AABB carAABB;
	carAABB.maxPosition = glm::vec3(0.75,0.46,0.4);
	carAABB.minPosition = glm::vec3(-0.69, -0.614, -0.4);
	AABB itemAABB;
	itemAABB.maxPosition = glm::vec3(2.0, 0.0, 0.25);
	itemAABB.minPosition = glm::vec3(1.5, -0.7, -0.25);


	static GLfloat g_vertex_buffer_data[] = {
		255.0f,370.0f,-0.4f,
		625.0f,370.0f,-0.4f,
		740.0f,579.0f,-0.4f,
		859.0f,610.0f,-0.4f,
		875.0f,807.0f,-0.4f,
		155.0f,807.0f,-0.4f,
		155.0f,646.0f,-0.4f,

		255.0f,370.0f,0.4f,
		625.0f,370.0f,0.4f,
		740.0f,579.0f,0.4f,
		859.0f,610.0f,0.4f,
		875.0f,807.0f,0.4f,
		155.0f,807.0f,0.4f,
		155.0f,646.0f,0.4f,

		// Front bumper
		875.0f,807.0f,-0.4f,
		859.0f,610.0f,0.4f,
		875.0f,807.0f,0.4f,
		859.0f,610.0f,-0.4f,

		// Hood

		859.0f,610.0f,-0.4f,
		740.0f,579.0f,0.4f,
		859.0f,610.0f,0.4f,
		740.0f,579.0f,-0.4f,

		// Front window body

		740.0f,579.0f,-0.4f,
		625.0f,370.0f,0.4f,
		740.0f,579.0f,0.4f,
		625.0f,370.0f,-0.4f,

		// Roof

		625.0f,370.0f,-0.4f,
		255.0f,370.0f,0.4f,
		625.0f,370.0f,0.4f,
		255.0f,370.0f,-0.4f,

		// Rear window body
		255.0f,370.0f,0.4f,
		255.0f,370.0f,-0.4f,
		155.0f,646.0f,-0.4f,
		155.0f,646.0f,0.4f,

		// Rear bumper
		155.0f,646.0f,0.4f,
		155.0f,646.0f,-0.4f,
		155.0f,807.0f,-0.4f,
		155.0f,807.0f,0.4f,

		// Under body
		155.0f,807.0f,0.4f,
		155.0f,807.0f,-0.4f,
		875.0f,807.0f,0.4f,
		875.0f,807.0f,-0.4f

	};
	for (int i = 0; i < sizeof(g_vertex_buffer_data) / sizeof(GL_FLOAT); i++) {
		int modedIndex = i % 3;
		if (modedIndex == 0) {
			g_vertex_buffer_data[i] = convertPositionX(g_vertex_buffer_data[i]);
		}
		else if (modedIndex == 1) {
			g_vertex_buffer_data[i] = convertPositionY(g_vertex_buffer_data[i]);
		}
	}

	// Two UV coordinatesfor each vertex. They were created with Blender.
	static  GLfloat g_uv_buffer_data[] = {
		255.0f,370.0f,
		625.0f,370.0f,
		740.0f,579.0f,
		859.0f,610.0f,
		875.0f,807.0f,
		155.0f,807.0f,
		155.0f,646.0f,

		255.0f,370.0f,
		625.0f,370.0f,
		740.0f,579.0f,
		859.0f,610.0f,
		875.0f,807.0f,
		155.0f,807.0f,
		155.0f,646.0f,

		// Front bumper
		300.0f,807.0f,
		700.0f,600.0f,
		700.0f,807.0f,
		300.0f,600.0f,

		// Hood

		300.0f,859.0f,
		700.0f,843.0f,
		700.0f,859.0f,
		300.0f,843.0f,

		// Front window body

		300.0f,857.0f,
		700.0f,667.0f,
		700.0f,857.0f,
		300.0f,667.0f,

		// Roof

		625.0f,370.0f,
		255.0f,370.0f,
		625.0f,370.0f,
		255.0f,370.0f,

		// Rear window body
		700.0f,667.0f,
		300.0f,667.0f,
		300.0f,920.0f,
		700.0f,920.0f,

		// Rear bumper
		700.0f,646.0f,
		300.0f,646.0f,
		300.0f,807.0f,
		700.0f,807.0f,

		// Under body
		155.0f,807.0f,
		155.0f,807.0f,
		875.0f,807.0f,
		875.0f,807.0f
	};

	for (int i = 0; i < sizeof(g_uv_buffer_data) / sizeof(GL_FLOAT); i++) {
		if (i % 2 == 0) {
			g_uv_buffer_data[i] = g_uv_buffer_data[i] / SCREEN_HEIGHT;
		}
		else {
			g_uv_buffer_data[i] = (SCREEN_WIDTH - g_uv_buffer_data[i]) / SCREEN_WIDTH;
		}

	}

	GLuint elements[] = {
		// Left body
		0, 1, 6,
		6, 1, 2,
		6, 2, 3,
		6, 3, 5,
		5, 3, 4,

		// Right body
		8, 7, 9,
		9, 7, 13,
		9, 13, 10,
		10, 13, 12,
		11, 10, 12,

		// Front bumper
		14, 15, 16,
		17, 15, 14,

		// Hood
		18, 19, 20,
		21, 19, 18,

		// Front window body
		22, 23, 24,
		25, 23, 22,

		// Roof
		26, 27, 28,
		29, 27, 26,

		// Rear window body
		30, 31, 32,
		30, 32, 33,

		// Rear bumper
		34, 35, 36,
		34, 36, 37,

		// Under body
		38, 39, 40,
		40, 39, 41
	};

	static GLfloat road_vertex_buffer_data[] = {
		-50.0f, -0.75f, 1.25f,
		50.0f, -0.75f, 1.25f,
		-50.0f, -0.75f, -1.25f,
		50.0f, -0.75f, -1.25f
	};

	static GLfloat grass_vertex_buffer_data[] = {
		-50.0f, -0.8f, 5.0f,
		50.0f, -0.8f, 5.0f,
		-50.0f, -0.8f, -5.0f,
		50.0f, -0.8f, -5.0f
	};

	GLfloat road_uv_buffer_data[] = {
		0.1, -10,
		0.1, 10,
		1, -10,
		1, 10
	};

	GLfloat grass_uv_buffer_data[] = {
		0.1, -10,
		0.1, 10,
		1, -10,
		1, 10
	};

	GLuint roadElements[] = {
		0, 1, 2,
		2, 1, 3
	};

	GLuint grassElements[] = {
		0, 1, 2,
		2, 1, 3
	};

	GLfloat roadNormals[] = {
		0, 1, 0,
		0, 1, 0
	};

	GLfloat grassNormals[] = {
		0, 1, 0,
		0, 1, 0
	};

	static GLfloat item_vertex_buffer_data[] = {
		//upper
		1.5f, 0.0f, -0.25f,
		1.5f, 0.0f, 0.25f,
		2.0f, 0.0f, 0.25f,
		2.0f, 0.0f, -0.25f,
		//lower
		1.5f, -0.7f, -0.25f,
		1.5f, -0.7f, 0.25f,
		2.0f, -0.7f, 0.25f,
		2.0f, -0.7f, -0.25f
	};

	GLuint itemElements[] = {
		//top and bottom
		0,1,2,
		0,2,3,
		4,5,6,
		4,6,7,
		//left and right
		0,1,5,
		0,5,4,
		3,2,6,
		3,6,7,
		//front and back
		0,3,7,
		0,7,4,
		1,2,6,
		1,6,5
	};

	GLfloat normals[sizeof(g_vertex_buffer_data) / sizeof(GLfloat)];

	//Get normal for each vertex
	for (int i = 0; i < sizeof(elements) / sizeof(GLfloat) / 3; i++) {
		vec3 normal = normalizeVertex(
			vec3(g_vertex_buffer_data[elements[i * 3] * 3], g_vertex_buffer_data[elements[i * 3] * 3 + 1], g_vertex_buffer_data[elements[i * 3] * 3 + 2]),
			vec3(g_vertex_buffer_data[elements[i * 3 + 1] * 3], g_vertex_buffer_data[elements[i * 3 + 1] * 3 + 1], g_vertex_buffer_data[elements[i * 3 + 1] * 3 + 2]),
			vec3(g_vertex_buffer_data[elements[i * 3 + 2] * 3], g_vertex_buffer_data[elements[i * 3 + 2] * 3 + 1], g_vertex_buffer_data[elements[i * 3 + 2] * 3 + 2]));

		for (int j = 0; j < 3; ++j) {
			// std::cout << elements[i * 3 + j] << " " << normal.x << " " << normal.y << " " << normal.z << std::endl;
			normals[elements[i * 3 + j] * 3] = normal.x;
			normals[elements[i * 3 + j] * 3 + 1] = normal.y;
			normals[elements[i * 3 + j] * 3 + 2] = normal.z;
		}
	}

	const int wheelDataSize = (WHEEL_TRIANGLE_AMOUNT + 1) * 3 * 2;
	const int wheelElementSize = WHEEL_TRIANGLE_AMOUNT * 3 * 2 * 2;
	const int wheelUVSize = (WHEEL_TRIANGLE_AMOUNT + 1) * 2 * 2;

	GLfloat rearLeftWheelData[wheelDataSize];
	GLuint rearLeftWheelElement[wheelElementSize];
	GLfloat frontLeftWheelData[wheelDataSize];
	GLuint frontLeftWheelElement[wheelElementSize];

	GLfloat rearRightWheelData[wheelDataSize];
	GLuint rearRightWheelElement[wheelElementSize];
	GLfloat frontRightWheelData[wheelDataSize];
	GLuint frontRightWheelElement[wheelElementSize];

	GLfloat rightWheelUV[wheelDataSize];
	GLfloat leftWheelUV[wheelDataSize];

	GLfloat rightWheelNormal[wheelDataSize];
	GLfloat leftWheelNormal[wheelDataSize];

	generateWheelUV(rightWheelUV, 0.5f, WHEEL_TRIANGLE_AMOUNT);
	generateWheelUV(leftWheelUV, 0.5f, WHEEL_TRIANGLE_AMOUNT);

	generateWheelNormal(rightWheelNormal, WHEEL_TRIANGLE_AMOUNT, true);
	generateWheelNormal(leftWheelNormal, WHEEL_TRIANGLE_AMOUNT, false);

	generateWheelVertexes(rearLeftWheelData, rearLeftWheelElement, convertPositionX(317), convertPositionY(800), -0.4001f, 0.1f, 0.15f, WHEEL_TRIANGLE_AMOUNT, false);
	generateWheelVertexes(frontLeftWheelData, frontLeftWheelElement, convertPositionX(717), convertPositionY(800), -0.4001f, 0.1f, 0.15f, WHEEL_TRIANGLE_AMOUNT, false);
	generateWheelVertexes(rearRightWheelData, rearRightWheelElement, convertPositionX(317), convertPositionY(800), 0.4001f, 0.1f, 0.15f, WHEEL_TRIANGLE_AMOUNT, true);
	generateWheelVertexes(frontRightWheelData, frontRightWheelElement, convertPositionX(717), convertPositionY(800), 0.4001f, 0.1f, 0.15f, WHEEL_TRIANGLE_AMOUNT, true);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);


	// The VBO containing the 4 vertices of the particles.
	// Thanks to instancing, they will be shared by all particles.
	static const GLfloat rain_vertex_buffer_data[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f,
		0.5f,  0.5f, 0.0f,
	};
	GLuint billboard_vertex_buffer;
	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rain_vertex_buffer_data), rain_vertex_buffer_data, GL_STATIC_DRAW);

	// The VBO containing the positions and sizes of the particles
	GLuint particles_position_buffer;
	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	GLuint particles_color_buffer;
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	int indexOffset = (WHEEL_TRIANGLE_AMOUNT + 1) * 2;
	double lastTime = glfwGetTime();

	float carSpeed = 10;

	do
	{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		double currentTime = glfwGetTime();
		double delta = currentTime - lastTime;
		lastTime = currentTime;

		for (int i = 1; i <= WHEEL_TRIANGLE_AMOUNT; ++i) {
			rotate(rightWheelUV[0], rightWheelUV[1], &rightWheelUV[i * 2], &rightWheelUV[i * 2 + 1], carSpeed);
			rotate(rightWheelUV[0], rightWheelUV[1], &rightWheelUV[i * 2 + indexOffset], &rightWheelUV[i * 2 + 1 + indexOffset], carSpeed);
			rotate(rightWheelUV[0], rightWheelUV[1], &leftWheelUV[i * 2], &leftWheelUV[i * 2 + 1], -carSpeed);
			rotate(rightWheelUV[0], rightWheelUV[1], &leftWheelUV[i * 2 + indexOffset], &leftWheelUV[i * 2 + 1 + indexOffset], -carSpeed);
		}

		for (int i = 0; i < 4; ++i) {
			road_vertex_buffer_data[i * 3] -= carSpeed / 500;
		}



		computeMatricesFromInputs();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// We will need the camera's position in order to sort the particles
		// w.r.t the camera's distance.
		// There should be a getCameraPosition() function in common/controls.cpp, 
		// but this works too.
		glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);

		glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;


		// Generate 10 new particule each millisecond,
		// but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec),
		// newparticles will be huge and the next frame even longer.
		int newparticles = (int)(delta*500.0);
		if (newparticles > (int)(0.016f*500.0))
			newparticles = (int)(0.016f*500.0);

		for (int i = 0; i<newparticles; i++) {
			int particleIndex = FindUnusedParticle();
			ParticlesContainer[particleIndex].life = 2.0f; // This particle will live 5 seconds.
			ParticlesContainer[particleIndex].pos = glm::vec3((rand() % 2000 - 1000.0f) / 100.0f, 8.0f, (rand() % 2000 - 1000.0f) / 100.0f);

			// float sgror instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
			// // combined with some user-controlled parameters (main direction, spread, etc)
			// glm::vec3 randomdir = glm::vec3(
			// 	(rand()%2000 - 1000.0f)/1000.0f,
			// 	(rand()%2000 - 1000.0f)/1000.0f,
			// 	(rand()%2000 - 1000.0f)/1000.0f
			// );

			ParticlesContainer[particleIndex].speed = glm::vec3(0.0, 0.0f, 0.0);


			// Very bad way to generate a random color
			ParticlesContainer[particleIndex].r = 0;
			ParticlesContainer[particleIndex].g = 0;
			ParticlesContainer[particleIndex].b = 128;
			ParticlesContainer[particleIndex].a = 255;

			ParticlesContainer[particleIndex].size = 0.08f;

		}



		// Simulate all particles
		int ParticlesCount = 0;
		for (int i = 0; i<MaxParticles; i++) {

			Particle& p = ParticlesContainer[i]; // shortcut

			if (p.life > 0.0f) {

				// Decrease life
				p.life -= delta;
				if (p.life > 0.0f) {

					// Simulate simple physics : gravity only, no collisions
					p.speed += glm::vec3(0.0f, -9.81f, 0.0f) * (float)delta * 0.5f;
					p.pos += p.speed * (float)delta;
					p.cameradistance = glm::length2(p.pos - CameraPosition);
					//ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

					// Fill the GPU buffer
					g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
					g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
					g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos.z;

					g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

					g_particule_color_data[4 * ParticlesCount + 0] = p.r;
					g_particule_color_data[4 * ParticlesCount + 1] = p.g;
					g_particule_color_data[4 * ParticlesCount + 2] = p.b;
					g_particule_color_data[4 * ParticlesCount + 3] = p.a;
					if (checkRainCollision(carAABB, p.pos)) {
						//collision with car
						p.life = -1;
						//fprintf(stderr, "rain collision\n");
					}
				}
				else {
					// Particles that just died will be put at the end of the buffer in SortParticles();
					p.cameradistance = -1.0f;
				}

				ParticlesCount++;

			}
		}

		SortParticles();


		//printf("%d ",ParticlesCount);


		// Update the buffers that OpenGL uses for rendering.
		// There are much more sophisticated means to stream data from the CPU to the GPU, 
		// but this is outside the scope of this tutorial.
		// http://www.opengl.org/wiki/Buffer_Object_Streaming


		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Use our shader
		glUseProgram(programID);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// Same as the billboards tutorial
		glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
		glUniform3f(CameraUp_worldspace_ID, ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

		glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : positions of particles' centers
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			4,                                // size : x + y + z + size => 4
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : particles' colors
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
		glVertexAttribPointer(
			2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			4,                                // size : r + g + b + a => 4
			GL_UNSIGNED_BYTE,                 // type
			GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// These functions are specific to glDrawArrays*Instanced*.
		// The first parameter is the attribute buffer we're talking about.
		// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
		// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
		glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
		glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
		glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

									 // Draw the particules !
									 // This draws many times a small triangle_strip (which looks like a quad).
									 // This is equivalent to :
									 // for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
									 // but faster.
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);


		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
		glVertexAttribDivisor(1, 0); // positions : one per quad (its center)                 -> 1
		glVertexAttribDivisor(2, 0);


		glUseProgram(carID);
		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform3fv(ViewPosID, 1, &getCameraPosition()[0]);
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		// glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);


		// Draw the triangle !
		// glDrawArrays(GL_TRIANGLES, 0, 15*3); // 12*3 indices starting at 0 -> 12 triangles

		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

		glBindTexture(GL_TEXTURE_2D, sideTexture);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 30 * sizeof(GLuint), elements, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);

		glBindTexture(GL_TEXTURE_2D, frontBumperTexture);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), elements + 30, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindTexture(GL_TEXTURE_2D, hoodTexture);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), elements + 36, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindTexture(GL_TEXTURE_2D, windowTexture);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), elements + 42, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindTexture(GL_TEXTURE_2D, blackTexture);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), elements + 48, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindTexture(GL_TEXTURE_2D, windowTexture);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), elements + 54, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindTexture(GL_TEXTURE_2D, rearBumperTexture);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), elements + 60, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindTexture(GL_TEXTURE_2D, blackTexture);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), elements + 66, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
		GLuint vertexbuffer;
		glGenBuffers(1, &vertexbuffer);

		GLuint uvbuffer;
		glGenBuffers(1, &uvbuffer);

		GLuint normalbuffer;
		glGenBuffers(1, &normalbuffer);

		GLuint elementbuffer;
		glGenBuffers(1, &elementbuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		glBindTexture(GL_TEXTURE_2D, wheelTexture);

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(frontLeftWheelData), frontLeftWheelData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(leftWheelUV), leftWheelUV, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(leftWheelNormal), leftWheelNormal, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(frontLeftWheelElement), frontLeftWheelElement, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, sizeof(frontLeftWheelElement) / sizeof(GLuint), GL_UNSIGNED_INT, 0);

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(frontLeftWheelData), rearLeftWheelData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(leftWheelUV), leftWheelUV, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(leftWheelNormal), leftWheelNormal, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(frontLeftWheelElement), rearLeftWheelElement, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, sizeof(frontLeftWheelElement) / sizeof(GLuint), GL_UNSIGNED_INT, 0);

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(frontLeftWheelData), frontRightWheelData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(rightWheelUV), rightWheelUV, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(leftWheelNormal), rightWheelNormal, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(frontLeftWheelElement), frontRightWheelElement, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, sizeof(frontLeftWheelElement) / sizeof(GLuint), GL_UNSIGNED_INT, 0);

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(frontLeftWheelData), rearRightWheelData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(rightWheelUV), rightWheelUV, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(leftWheelNormal), rightWheelNormal, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(frontLeftWheelElement), rearRightWheelElement, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, sizeof(frontLeftWheelElement) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
		
		GLuint grassvertexbuffer;
		glGenBuffers(1, &grassvertexbuffer);

		GLuint grassuvbuffer;
		glGenBuffers(1, &grassuvbuffer);

		GLuint grassnormalbuffer;
		glGenBuffers(1, &grassnormalbuffer);

		GLuint grasselementbuffer;
		glGenBuffers(1, &grasselementbuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grasselementbuffer);

		glBindBuffer(GL_ARRAY_BUFFER, grassvertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, grassuvbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, grassnormalbuffer);
		glVertexAttribPointer(
			2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		glBindTexture(GL_TEXTURE_2D, grassTexture);
		glBindBuffer(GL_ARRAY_BUFFER, grassvertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(grass_vertex_buffer_data), grass_vertex_buffer_data, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, grassuvbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(grass_uv_buffer_data), grass_uv_buffer_data, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, grassnormalbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(grassNormals), grassNormals, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(grassElements), grassElements, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, sizeof(grassElements) / sizeof(GLuint), GL_UNSIGNED_INT, 0);

		GLuint itemvertexbuffer;
		glGenBuffers(1, &itemvertexbuffer);
		GLuint itemelementbuffer;
		glGenBuffers(1, &itemelementbuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, itemelementbuffer);

		glBindBuffer(GL_ARRAY_BUFFER, itemvertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		glEnableVertexAttribArray(1);
		glBindTexture(GL_TEXTURE_2D, itemTexture);
		glBindBuffer(GL_ARRAY_BUFFER, itemvertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(item_vertex_buffer_data), item_vertex_buffer_data, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(itemElements), itemElements, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, sizeof(itemElements) / sizeof(GLuint), GL_UNSIGNED_INT, 0);

		GLuint roadvertexbuffer;
		glGenBuffers(1, &roadvertexbuffer);

		GLuint roaduvbuffer;
		glGenBuffers(1, &roaduvbuffer);

		GLuint roadnormalbuffer;
		glGenBuffers(1, &roadnormalbuffer);

		GLuint roadelementbuffer;
		glGenBuffers(1, &roadelementbuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, roadelementbuffer);

		glBindBuffer(GL_ARRAY_BUFFER, roadvertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, roaduvbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, roadnormalbuffer);
		glVertexAttribPointer(
			2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		glBindTexture(GL_TEXTURE_2D, roadTexture);
		glBindBuffer(GL_ARRAY_BUFFER, roadvertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(road_vertex_buffer_data), road_vertex_buffer_data, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, roaduvbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(road_uv_buffer_data), road_uv_buffer_data, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, roadnormalbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(roadNormals), roadNormals, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(roadElements), roadElements, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, sizeof(roadElements) / sizeof(GLuint), GL_UNSIGNED_INT, 0);

	

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			if (carSpeed < 40) {
				carSpeed += 10;
			}
		}
		else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			if (carSpeed < 20) {
				carSpeed += 5;
			}
			for (int i = 0; i < sizeof(item_vertex_buffer_data) / sizeof(GL_FLOAT); i++) {
				if (i % 3 == 0) {
					item_vertex_buffer_data[i] -= 0.02;
				}
			}
			itemAABB.maxPosition.x -= 0.02;
			itemAABB.minPosition.x -= 0.02;
			
			
		}
		else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			if (carSpeed > -10) {
				carSpeed -= 2.5;
			}
			for (int i = 0; i < sizeof(item_vertex_buffer_data) / sizeof(GL_FLOAT); i++) {
				if (i % 3 == 0) {
					item_vertex_buffer_data[i] += 0.02;
				}
			}
			itemAABB.maxPosition.x += 0.02;
			itemAABB.minPosition.x += 0.02;
		}
		else {
			if (carSpeed > 0) {
				carSpeed -= 2.5;
			}
		}

		if (carSpeed > 0 || carSpeed < 0) {
			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
				for (int i = 0; i < sizeof(road_vertex_buffer_data) / sizeof(GL_FLOAT) / 3; i++) {
					if (road_vertex_buffer_data[i * 3 + 2] < 2) {
						road_vertex_buffer_data[i * 3 + 2] += 0.02;
					}
					else {
						break;
					}
				}
			}
			else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
				
				for (int i = sizeof(road_vertex_buffer_data) / sizeof(GL_FLOAT) / 3 - 1; i >= 0; i--) {
					if (road_vertex_buffer_data[i * 3 + 2] > -2.03) {
						road_vertex_buffer_data[i * 3 + 2] -= 0.02;
					}
					else {
						break;
					}
				}
			}
			if (checkCollision(carAABB, itemAABB)) {
				//ada collision
				//fprintf(stderr, "COLLISION\n");

				//change item position
				itemAABB.maxPosition.x += 2;
				itemAABB.minPosition.x += 2;
				for (int i = 0; i < sizeof(item_vertex_buffer_data) / sizeof(GL_FLOAT); i++) {
					if (i % 3 == 0) {
						item_vertex_buffer_data[i] += 2;
					}
				}
			}
		}

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);


	delete[] g_particule_position_size_data;

	// Cleanup VBO and shader
	glDeleteBuffers(1, &particles_color_buffer);
	glDeleteBuffers(1, &particles_position_buffer);
	glDeleteBuffers(1, &billboard_vertex_buffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);


	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

