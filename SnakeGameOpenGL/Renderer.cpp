#include "Renderer.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

Renderer::Renderer(int screenWidth, int screenHeight):width(screenWidth), height(screenHeight) {
	float vertices[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0, 1.0f,

		0.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};


	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//  Position attribute
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Load shaders 
	shaderProgram = loadShader("vertex.glsl", "fragment.glsl");
}

Renderer::~Renderer() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);
}


void Renderer::drawRectangle(float x, float y, float widthRect, float heightRect, const glm::vec3& color) const {
    glUseProgram(shaderProgram);

    // Fix for aspect ratio
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / aspect, 1.0f, 1.0f));

    // Apply translation and non-uniform scaling
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(widthRect, heightRect, 1.0f));

    // Send matrix to shader
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

    // Send color
    GLuint colorLoc = glGetUniformLocation(shaderProgram, "color");
    glUniform3fv(colorLoc, 1, &color[0]);

    // Bind and draw
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}


std::vector<float> generateCircleVertices(float cx, float cy, float radius, int segments) {
    std::vector<float> vertices;

    // Center of the circle
    vertices.push_back(cx);
    vertices.push_back(cy);

    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * 3.1415926f * float(i) / float(segments);
        float x = cx + radius * cosf(theta);
        float y = cy + radius * sinf(theta);
        vertices.push_back(x);
        vertices.push_back(y);
    }

    return vertices;
}


void Renderer::drawCircle(float cx, float cy, float radius, const glm::vec3& color) const {
    std::vector<float> vertices = generateCircleVertices(cx, cy, radius, 50);

    GLuint circleVAO, circleVBO;
    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);

    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(shaderProgram);

    // Identity model (no transformation, already positioned)
    float aspect = (float)width / (float)height;
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / aspect, 1.0f, 1.0f));
    
    model = glm::translate(model, glm::vec3(cx, cy, 0.0f));
    model = glm::scale(model, glm::vec3(radius, radius, 1.0f));

    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

    GLuint colorLoc = glGetUniformLocation(shaderProgram, "color");
    glUniform3fv(colorLoc, 1, &color[0]);

    glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2);

    glDeleteVertexArrays(1, &circleVAO);
    glDeleteBuffers(1, &circleVBO);
}


GLuint Renderer::loadShader(const char* vertexPath, const char* fragmentPath) {
    auto readFile = [](const char* path) -> std::string {
        std::ifstream file(path);
        std::stringstream buffer;
        if (!file.is_open()) {
            std::cerr << "Failed to open shader file: " << path << std::endl;
            return "";
        }
        buffer << file.rdbuf();
		std::cout << "Shader file loaded successfully: " << path << std::endl;
        return buffer.str();
        };

    std::string vertexCode = readFile(vertexPath);
    std::string fragmentCode = readFile(fragmentPath);

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    GLuint vertex, fragment;
    GLint success;
    char infoLog[512];

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        std::cerr << "Vertex Shader Compilation Failed:" << std::endl << infoLog << std::endl;
    }

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        std::cerr << "Fragment Shader Compilation Failed:" << std::endl << infoLog <<std::endl;
    }

    // Shader Program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Failed:"<< std::endl << infoLog << std::endl;
    }
    else {
		std::cout << "Shader Program linked successfully" << std::endl;
    }

    // Cleanup
    glDeleteShader(vertex);
    glDeleteShader(fragment);


    return program;
}
