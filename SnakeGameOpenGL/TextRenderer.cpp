#include "TextRenderer.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>

bool TextRenderer::init(const char* fontPath, int fontSize) {
    // Init FreeType
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "ERROR: Could not init FreeType Library" << std::endl;
        return false;
    }
    else {
		std::cout << "FreeType initialized successfully" << std::endl;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath, 0, &face)) {
        std::cerr << "ERROR: Failed to load font" << std::endl;
        return false;
	}
	else {
		std::cout << "Font loaded successfully: " << fontPath << std::endl;
	}
    
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
            continue;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x)
        };
        characters.insert(std::pair<char, Character>(c, character));
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // Shader loading (simple)
    std::string vertCode = loadShaderSource("text_vertex.glsl");
    std::string fragCode = loadShaderSource("text_fragment.glsl");
    shaderProgram = createShaderProgram(vertCode.c_str(), fragCode.c_str());

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

void TextRenderer::drawText(const std::string& text, float x, float y, float scale, glm::vec3 color) {
    glUseProgram(shaderProgram);
    glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), color.x, color.y, color.z);
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    for (auto c : text) {
        Character ch = characters[c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextRenderer::clearText() {
	for (auto& pair : characters) {
		glDeleteTextures(1, &pair.second.textureID);
	}
	characters.clear();
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);
	std::cout << "TextRenderer cleared" << std::endl;
}


std::string TextRenderer::loadShaderSource(const char* path) {
    std::ifstream file(path);
    std::stringstream buffer;
    if (!file.is_open()) {
        std::cerr << "Failed to load shader: " << path << std::endl;
        return "";
    }
    buffer << file.rdbuf();
	std::cout << "Shader loaded successfully: " << path << std::endl;
    return buffer.str();
}

GLuint TextRenderer::createShaderProgram(const char* vert, const char* frag) {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vert, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &frag, nullptr);
    glCompileShader(fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}
