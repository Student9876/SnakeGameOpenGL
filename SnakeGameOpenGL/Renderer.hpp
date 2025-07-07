#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>


class Renderer {
public:
	Renderer(int screenWidth, int screenHeight);
	~Renderer();


	//void drawSquare(float x, float y, float size, const glm::vec3& color) const;
	void drawRectangle(float x, float y, float width, float height, const glm::vec3& color) const;
	void drawCircle(float cx, float cy, float radius, const glm::vec3& color) const;

private:
	int width, height;

	GLuint VAO, VBO, shaderProgram;

	GLuint loadShader(const char* vertextPath, const char* fragmentPath);
};