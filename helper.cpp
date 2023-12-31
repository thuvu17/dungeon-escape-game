//
//  helper.cpp
//  SDLProject
//
//  Created by Sebastián Romero Cruz on 5/31/22.
//  Copyright © 2022 ctg. All rights reserved.
//
#define GL_SILENCE_DEPRECATION
#include <iostream>

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

// We might not need all these libraries yet, but I'll leave them there just in case
#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Scene.h"
#include "Utility.h"
#include <stdio.h>

const char MAT_SEP = '\t';
const int FONTBANK_SIZE = 16;

/**
 * Prints a formatted representation of a size x size matrix onto
 * the user's console. The \t character is used for horizontal
 * separation.
 *
 * For example, printing a 4 x 4 identity matrix
 *
 *      print_matrix(glm::mat4(1.0f), 4);
 *
 * Prints:
 *
 *     1    0    0    0
 *     0    1    0    0
 *     0    0    1    0
 *     0    0    0    1
 *
 * @param matrix Reference to the matrix.
 * @param size The size of the matrix
 */

void DrawText(ShaderProgram *program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for each character
    // Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their position
        //    relative to the whole sentence)
        int spritesheet_index = (int) text[i];  // ascii value of character
        float offset = (screen_size + spacing) * i;
        
        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float) (spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float) (spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
        });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
        });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
    
    program->SetModelMatrix(model_matrix);
    glUseProgram(program->programID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int) (text.size() * 6));
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void print_matrix(glm::mat4 &matrix, int size)
{
    for (auto row = 0 ; row < size ; row++)
    {
        for (auto col = 0 ; col < size ; col++)
        {
            // Print row
            std::cout << matrix[row][col] << MAT_SEP;
        }
        
        // Visually "move on" to the next row
        std::cout << "\n";
    }
}

void draw_background(GameState &m_state, ShaderProgram *program)
{
    // Initialise
    m_state.background = new Entity();
    m_state.background->set_entity_type(BACKGROUND);
    m_state.background->set_width(14.0f);
    m_state.background->set_height(10.0f);
    m_state.background->m_texture_id = Utility::load_texture("assets/tileset/bg.png");
    
    // Update
    float width = m_state.background->get_width();
    float height = m_state.background->get_height();
    m_state.background->m_model_matrix = glm::translate(m_state.background->m_model_matrix, glm::vec3(7.0f, -4.5f, 0.0f));
    m_state.background->m_model_matrix = glm::scale(m_state.background->m_model_matrix, glm::vec3(width, height, 1.0f));
    
    // Render
    m_state.background->render(program);
}

