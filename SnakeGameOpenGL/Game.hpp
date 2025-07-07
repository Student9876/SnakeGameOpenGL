#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include "Renderer.hpp"
#include "TextRenderer.hpp"
#include <deque>
#include <random>

enum class Direction { UP, DOWN, LEFT, RIGHT };


class Game {
public:
	Game(int width, int height, const std::string& title);
	~Game();

	void run();
	int score = 0;

private:
	GLFWwindow* window;
	int width, height;
	std::string title;
	Renderer* renderer;
	TextRenderer* textRenderer;

	/*glm::vec2 playerPosition = glm::vec2(0.0f, 0.0f);
	float playerSpeed = 0.01f;*/

	void init();
	void update();
	void render();
	
	double lastFrameTime = 0.0;
	double deltaTime = 0.0;

	
	glm::ivec2 snakeHead = { 10, 10 };
	Direction snakeDirection = Direction::RIGHT;
	
	std::deque<glm::ivec2> snake;  // body: [head, ..., tail]
	glm::ivec2 foodPosition;

	int snakeLength = 1; // initial length
	float moveTimer = 0.0f;
	float moveDelay = 0.2f; // seconds between moves

	const int gridWidth = 20;
	const int gridHeight = 20;
	
	// Methods
	void updateDirection(); // input
	void updateSnake();     // logic
	void drawGrid(); // rendering
	void spawnFood(); // random food position
	void displayScore(); // display score on screen
};
