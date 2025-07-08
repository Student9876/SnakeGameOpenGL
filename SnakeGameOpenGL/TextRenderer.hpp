#pragma once

#include <map>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>
#include <glad/glad.h>

struct Character {
    GLuint textureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    GLuint advance;
};

class TextRenderer {
public:
    std::map<char, Character> characters;
    GLuint VAO, VBO;
    GLuint shaderProgram;

    bool init(const char* fontPath, int fontSize);
    void drawText(const std::string& text, float x, float y, float scale, glm::vec3 color);
    void clearText();
    GLuint createShaderProgram(const char* vert, const char* frag);
	std::string loadShaderSource(const char* path);
};
