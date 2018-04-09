// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>

#include <iostream>

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 1000
#define WHEEL_TRIANGLE_AMOUNT 1000
#define VELG_TRIANGLE_AMOUNT 6
#define ROTATE_ANGLE 10

float convertPositionX(int X) {
	static float zeroPosition = SCREEN_WIDTH / 2;
	return (X - zeroPosition) / zeroPosition;
}

float convertPositionY(int Y) {
	static float zeroPosition = SCREEN_HEIGHT / 2;
	return ((SCREEN_HEIGHT - Y) - zeroPosition) / zeroPosition;
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
	} else {
		width = -width;
	}

	int indexOffset = (triangleAmount + 1) * 3;

	circleData[0] = x;
	circleData[1]= y;
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

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( SCREEN_WIDTH, SCREEN_HEIGHT, "Tutorial 0 - Keyboard and Mouse", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
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
    glfwSetCursorPos(window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	// glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Load the texturef
	GLuint sideTexture = loadBMP_custom("Side Body.bmp");
	GLuint frontBumperTexture = loadBMP_custom("Front Bumper.bmp");
	GLuint hoodTexture = loadBMP_custom("Hood.bmp");
	GLuint rearBumperTexture = loadBMP_custom("Rear Bumper.bmp");
	GLuint windowTexture = loadBMP_custom("Window.bmp");
	GLuint blackTexture = loadBMP_custom("Black.bmp");
	GLuint wheelTexture = loadBMP_custom("Wheel.bmp");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
	// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
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
		} else if (modedIndex == 1) {
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
		if( i % 2 == 0) {
			g_uv_buffer_data[i] = g_uv_buffer_data[i] / SCREEN_HEIGHT;
		} else {
			g_uv_buffer_data[i] = (SCREEN_WIDTH - g_uv_buffer_data[i]) / SCREEN_WIDTH;
		}
		
		// int modedIndex = i % 3;
		// if (modedIndex == 0) {
		// 	g_uv_buffer_data[i] = convertPositionX(g_uv_buffer_data[i]);
		// } else if (modedIndex == 1) {
		// 	g_uv_buffer_data[i] = convertPositionY(g_uv_buffer_data[i]);
		// }
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
		29, 26, 27,

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

	int wheelDataSize = (WHEEL_TRIANGLE_AMOUNT + 1) * 3 * 2;
	int wheelElementSize = WHEEL_TRIANGLE_AMOUNT * 3 * 2 * 2;
	int wheelUVSize = (WHEEL_TRIANGLE_AMOUNT + 1) * 2 * 2;

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
	generateWheelUV(rightWheelUV, 0.5f, WHEEL_TRIANGLE_AMOUNT);
	generateWheelUV(leftWheelUV, 0.5f, WHEEL_TRIANGLE_AMOUNT);

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

	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
	
	int indexOffset = (WHEEL_TRIANGLE_AMOUNT + 1) * 2;
	do{	
		for (int i = 1; i <= WHEEL_TRIANGLE_AMOUNT; ++i) {
			rotate(rightWheelUV[0], rightWheelUV[1], &rightWheelUV[i * 2], &rightWheelUV[i * 2 + 1], ROTATE_ANGLE);
			rotate(rightWheelUV[0], rightWheelUV[1], &rightWheelUV[i * 2 + indexOffset], &rightWheelUV[i * 2 + 1 + indexOffset], ROTATE_ANGLE);
			rotate(rightWheelUV[0], rightWheelUV[1], &leftWheelUV[i * 2], &leftWheelUV[i * 2 + 1], -ROTATE_ANGLE);
			rotate(rightWheelUV[0], rightWheelUV[1], &leftWheelUV[i * 2 + indexOffset], &leftWheelUV[i * 2 + 1 + indexOffset], -ROTATE_ANGLE);
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

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

		glBindTexture(GL_TEXTURE_2D, wheelTexture);

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(frontLeftWheelData), frontLeftWheelData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(leftWheelUV), leftWheelUV, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(frontLeftWheelElement), frontLeftWheelElement, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, sizeof(frontLeftWheelElement) / sizeof(GLuint), GL_UNSIGNED_INT, 0);

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(frontLeftWheelData), rearLeftWheelData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(leftWheelUV), leftWheelUV, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(frontLeftWheelElement), rearLeftWheelElement, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, sizeof(frontLeftWheelElement) / sizeof(GLuint), GL_UNSIGNED_INT, 0);

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(frontLeftWheelData), frontRightWheelData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(rightWheelUV), rightWheelUV, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(frontLeftWheelElement), frontRightWheelElement, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, sizeof(frontLeftWheelElement) / sizeof(GLuint), GL_UNSIGNED_INT, 0);

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(frontLeftWheelData), rearRightWheelData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(rightWheelUV), rightWheelUV, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(frontLeftWheelElement), rearRightWheelElement, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, sizeof(frontLeftWheelElement) / sizeof(GLuint), GL_UNSIGNED_INT, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &TextureID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

