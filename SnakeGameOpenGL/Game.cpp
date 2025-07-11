﻿#include <iostream>
#include "Game.hpp"
#include <glad/glad.h>

Game::Game(int width, int height, const std::string& title):
	width(width), height(height), title(title) {
	init();
}

Game::~Game() {
	glfwDestroyWindow(window);
	glfwTerminate();
	if (renderer) {
		delete renderer;
		renderer = nullptr;
	}
}

void Game::init() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" <<std::endl;
        exit(EXIT_FAILURE);
    }
    else {
		std::cout << "GLFW initialized successfully" << std::endl;
    }

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    else {
		std::cout << "GLFW window created successfully" << std::endl;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable VSync

    // Load GLAD after context
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
		std::cout << "GLAD initialized successfully" << std::endl;
    }

    // Set OpenGL viewport
    glViewport(0, 0, width, height);


    renderer = new Renderer(width, height);
	if (!renderer) {
		std::cerr << "Failed to create Renderer" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(EXIT_FAILURE);
    }
    else {
		std::cout << "Renderer created successfully" << std::endl;
    }
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);


	textRenderer = new TextRenderer();
    textRenderer->init("BitcountGridDouble-VariableFont_CRSV,ELSH,ELXP,slnt,wght.ttf", 30);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    snake.clear();
    for (int i = 0; i < snakeLength; ++i) {
        snake.push_back(glm::ivec2(10 - i, 10)); // horizontal right
    }

	spawnFood();
}


void Game::run() {
    lastFrameTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

		update();
		render();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void Game::update() {
	// TODO: input and logic
    if (state == GameState::GameOver) {
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            restartGame();
        }
        return;
    }

    updateDirection();

    moveTimer += static_cast<float>(deltaTime); // assume ~60 FPS for now, use deltaTime later

    if (moveTimer >= moveDelay) {
        moveTimer = 0.0f;
        updateSnake();
    }

}

void Game::restartGame() {
	snake.clear();
	snakeLength = 1;
	for (int i = 0; i < snakeLength; ++i) {
		snake.push_back(glm::ivec2(10 - i, 10)); // horizontal right
	}
	spawnFood();
	score = 0;
	state = GameState::Playing;
	snakeDirection = Direction::RIGHT;
	moveTimer = 0.0f;
	std::cout << "Game restarted!" << std::endl;
	glfwSetWindowShouldClose(window, false);
    render();
}

void Game::updateSnake() {
    glm::ivec2 newHead = snake.front();

    switch (snakeDirection) {
        case Direction::UP:    newHead.y += 1; break;
        case Direction::DOWN:  newHead.y -= 1; break;
        case Direction::LEFT:  newHead.x -= 1; break;
        case Direction::RIGHT: newHead.x += 1; break;
    }

    // Wrap around
    newHead.x = (newHead.x + gridWidth) % gridWidth;
    newHead.y = (newHead.y + gridHeight) % gridHeight;

    // Check collision with self
    if (std::find(snake.begin(), snake.end(), newHead) != snake.end()) {
        std::cout << "Game Over! Final Score: " << score << std::endl;
		state = GameState::GameOver;
    }

    // Insert new head
    snake.push_front(newHead);

    // Check if food eaten
    if (newHead == foodPosition) {
        snakeLength++;
        score += 5;
        std::cout << "Food eaten! Score: " << score << std::endl;
        spawnFood();
    }

    // Trim tail
    while (snake.size() > snakeLength) {
        snake.pop_back();
    }
}

void Game::updateDirection() {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && snakeDirection != Direction::DOWN)
        snakeDirection = Direction::UP;
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && snakeDirection != Direction::UP)
        snakeDirection = Direction::DOWN;
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && snakeDirection != Direction::RIGHT)
        snakeDirection = Direction::LEFT;
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && snakeDirection != Direction::LEFT)
        snakeDirection = Direction::RIGHT;
}

void Game::drawGrid() {
    float cellWidth = 2.0f / gridWidth;
    float cellHeight = 2.0f / gridHeight;

    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            float screenX = -1.0f + x * cellWidth;
            float screenY = -1.0f + y * cellHeight;

            glm::vec3 cellColor = glm::vec3(0.15f);  // dark gray
            renderer->drawRectangle(screenX, screenY, cellWidth, cellHeight, cellColor);
        }
    }
}

void Game::spawnFood() {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> xDist(0, gridWidth - 1);
    std::uniform_int_distribution<int> yDist(0, gridHeight - 1);

    do {
        foodPosition = glm::ivec2(xDist(rng), yDist(rng));
    } while (std::find(snake.begin(), snake.end(), foodPosition) != snake.end());
}


void Game::render() {
    //Set background color 
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);


    //render the grid 
    drawGrid();

    if (state == GameState::Playing) {
        // Display score
        textRenderer->drawText("Score: " + std::to_string(score), -0.95f, 0.9f, 0.002f, glm::vec3(1.0f));


        //render snake head 
        float cw = 2.0f / gridWidth;
        float ch = 2.0f / gridHeight;

        // Draw food
        float fx = -1.0f + foodPosition.x * cw;
        float fy = -1.0f + foodPosition.y * ch;
        renderer->drawRectangle(fx, fy, cw, ch, glm::vec3(1.0f, 0.0f, 0.0f));

        // Draw snake
        for (const auto& segment : snake) {
            float sx = -1.0f + segment.x * cw;
            float sy = -1.0f + segment.y * ch;
            renderer->drawRectangle(sx, sy, cw, ch, glm::vec3(0.0f, 1.0f, 0.0f));
        }
    }
    else if (state == GameState::GameOver) {
        textRenderer->drawText("Game Over!", -0.2f, 0.1f, 0.002f, glm::vec3(1, 0, 0));
        textRenderer->drawText("Press R to Restart", -0.3f, -0.1f, 0.002f, glm::vec3(1, 1, 1));
        textRenderer->drawText("Score: " + std::to_string(score), -0.2f, -0.3f, 0.002f, glm::vec3(1, 1, 0));
    }
}
