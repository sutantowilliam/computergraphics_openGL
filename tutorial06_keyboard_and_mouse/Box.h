#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>

class Box
{
public:
	Box(GLfloat x, GLfloat y, GLfloat z);
	~Box();
	std::vector<GLfloat> getVertexBufferData();
	std::vector<GLuint> getItemElements();
	glm::vec3 getMaxPosition();
	glm::vec3 getMinPosition();
	void addMaxPositionX(float);
	void addMinPositionX(float);
	void addMaxPositionZ(float);
	void addMinPositionZ(float);
	void translateX(float);
	void changeVertexBufferData(int, GLfloat);
	void resetPosition();
private:
	GLfloat size;
	std::vector<GLfloat> vertex_buffer_data;
	std::vector<GLuint> itemElements;
	glm::vec3 minPosition;
	glm::vec3 maxPosition;
	glm::vec3 initPosition;
};