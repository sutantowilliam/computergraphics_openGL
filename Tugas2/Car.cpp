// Include standard headers
#include <stdio.h>
#include <stdlib.h>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
// Include GLM
#include <glm/glm.hpp>
using namespace std;
using namespace glm;
GLFWwindow* window;
GLuint programID;
GLint positionHandle;
GLint colorHandle;

const GLchar *vs_source = R"glsl(
#version 120
attribute vec2 position;
attribute vec3 color;
varying vec3 vColor;
void main(void) { 
	vColor = color;
	gl_Position = vec4(position, 0.0, 1.0); 
}
)glsl";

const char *fs_source =
"#version 120\n"
"varying vec3 vColor;"
"void main(void) { "
" gl_FragColor = vec4(vColor,1.0);"
"}";


int init_resources(void)
{
	GLint compile_ok = GL_FALSE;
	GLint link_ok = GL_FALSE;

	GLuint vertexHandle = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexHandle, 1, &vs_source, NULL);
	glCompileShader(vertexHandle);
	glGetShaderiv(vertexHandle, GL_COMPILE_STATUS, &compile_ok);
	if (0 == compile_ok)
	{
		fprintf(stderr, "Error in vertex shader\n");
		return 0;
	}

	GLuint fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragHandle, 1, &fs_source, NULL);
	glCompileShader(fragHandle);
	glGetShaderiv(fragHandle, GL_COMPILE_STATUS, &compile_ok);
	if (!compile_ok) {
		fprintf(stderr, "Error in fragment shader\n");
		return 0;
	}

	programID = glCreateProgram();
	glAttachShader(programID, vertexHandle);
	glAttachShader(programID, fragHandle);
	glBindFragDataLocation(programID, 0, "gl_FragColor");
	glLinkProgram(programID);
	glGetProgramiv(programID, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		fprintf(stderr, "glLinkProgram:");
		return 0;
	}
	return 1;
}



int main() {
	glfwInit();
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}
	//buat object window
	GLFWwindow* window = glfwCreateWindow(600, 600, "Hello", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Dark blue background
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

	// Create Vertex Array Object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create a Vertex Buffer Object and copy the vertex data to it
	GLuint vbo;
	glGenBuffers(1, &vbo);

	GLfloat car_data[] = {
		255.0f,370.0f,0.2f,0.85f,0.6f,
		625.0f,370.0f,0.2f,0.85f,0.6f,
		740.0f,579.0f,0.2f,0.85f,0.6f,
		859.0f,610.0f,0.2f,0.85f,0.6f,
		875.0f,807.0f,0.2f,0.65f,0.4f,
		155.0f,807.0f,0.2f,0.85f,0.6f,
		155.0f,646.0f,0.2f,0.85f,0.6f,
		269.0f,382.0f,0.8f,0.8f,0.8f,
		444.0f,382.0f,1.0f,1.0f,1.0f,
		444.0f,581.0f,1.0f,1.0f,1.0f,
		197.0f,581.0f,1.0f,1.0f,1.0f,
		478.0f,382.0f,0.8f,0.8f,0.8f,
		613.0f,382.0f,1.0f,1.0f,1.0f,
		716.0f,581.0f,1.0f,1.0f,1.0f,
		477.0f,581.0f,1.0f,1.0f,1.0f,
	};
	for (int i = 0; i < 75; i++) {
		if ((i % 5 == 0)||(i%5==1)) {
			if (i % 5 == 1) {
				car_data[i] = 1000.0f - car_data[i];
			}
			car_data[i] -= 500.0f;
			car_data[i] = car_data[i] / 500.0f;
		}
		
		
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(car_data), car_data, GL_STATIC_DRAW);

	GLuint ebo;
	glGenBuffers(1, &ebo);
	GLuint elements[] = {
		0,1,2,
		0,2,6,
		2,3,4,
		2,4,6,
		4,5,6,
		7,8,9,
		7,9,10,
		11,12,13,
		11,13,14,
	};
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(elements), elements, GL_STATIC_DRAW);

	do {
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);
		init_resources();
		// Use our shader
		glUseProgram(programID);

		GLint posAttrib = glGetAttribLocation(programID, "position");
		glEnableVertexAttribArray(posAttrib);
		glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

		GLint colAttrib = glGetAttribLocation(programID, "color");
		glEnableVertexAttribArray(colAttrib);
		glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

		glDrawElements(GL_TRIANGLES, 27, GL_UNSIGNED_INT, 0);
	
		glDisableVertexAttribArray(posAttrib);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	//glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}
