#include "Box.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>


Box::Box(GLfloat x, GLfloat y, GLfloat z){
	size = 0.2f;
	initPosition = glm::vec3(x, y, z);
	minPosition = glm::vec3(-1.0f*size+x, -1.0f*size + y, -1.0f*size + z);
	maxPosition = glm::vec3(1.0f*size + x, 1.0f*size + y, 1.0f*size + z);
	vertex_buffer_data = {
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		//lower
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f
	};
	for (int i = 0; i < vertex_buffer_data.size(); i++) {
		vertex_buffer_data[i] *= size;
	}
	for (int i = 0; i < vertex_buffer_data.size(); i++) {
		if (i % 3 == 0) {
			vertex_buffer_data[i] += x;
		}
		else if (i % 3 == 1) {
			vertex_buffer_data[i] += y;
		}
		else {
			vertex_buffer_data[i] += z;
		}
	}
	itemElements = {
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
}


Box::~Box()
{
}

std::vector<GLfloat> Box::getVertexBufferData() {
	return vertex_buffer_data;
}

std::vector<GLuint> Box::getItemElements() {
	return itemElements;
}

glm::vec3 Box::getMaxPosition() {
	return maxPosition;
}

glm::vec3 Box::getMinPosition() {
	return minPosition;
}

void Box::addMaxPositionX(float change) {
	maxPosition.x += change;
}

void Box::addMinPositionX(float change) {
	minPosition.x += change;
}

void Box::addMaxPositionZ(float change) {
	maxPosition.z += change;
}

void Box::addMinPositionZ(float change) {
	minPosition.z += change;
}

void Box::translateX(float change) {
	for (int i = 0; i < vertex_buffer_data.size(); i++) {
		if (i % 3 == 0) {
			vertex_buffer_data[i] += change;
		}
	}
}

void Box::changeVertexBufferData(int index, GLfloat change) {
	vertex_buffer_data[index] += change;
}

void Box::resetPosition() {
	vertex_buffer_data.clear();
	minPosition = glm::vec3(-1.0f*size + initPosition.x, -1.0f*size + initPosition.y, -1.0f*size + initPosition.z);
	maxPosition = glm::vec3(1.0f*size + initPosition.x, 1.0f*size + initPosition.y, 1.0f*size + initPosition.z);
	vertex_buffer_data = {
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		//lower
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f
	};
	for (int i = 0; i < vertex_buffer_data.size(); i++) {
		vertex_buffer_data[i] *= size;
	}
	for (int i = 0; i < vertex_buffer_data.size(); i++) {
		if (i % 3 == 0) {
			vertex_buffer_data[i] += initPosition.x;
		}
		else if (i % 3 == 1) {
			vertex_buffer_data[i] += initPosition.y;
		}
		else {
			vertex_buffer_data[i] += initPosition.z;
		}
	}
}
