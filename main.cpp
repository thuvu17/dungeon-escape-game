#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f
#define LEVEL1_RIGHT_EDGE 8.0f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.hpp"
#include "LevelX.hpp"
#include "Effects.h"

// ––––– CONSTANTS ––––– //
const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED     = 0.0f,
            BG_BLUE    = 0.0f,
            BG_GREEN   = 0.0f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;


// ––––– GLOBAL VARIABLES ––––– //
Scene  *g_current_scene;
Entity *g_lives;
LevelX *g_levelX;
LevelA *g_levelA;
LevelB *g_levelB;
LevelC *g_levelC;

Effects *g_effects;
Scene   *g_levels[4];

SDL_Window* g_display_window;
bool g_game_is_running = true;
bool g_player_win      = false;
bool g_player_lose     = false;

ShaderProgram g_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;
int g_number_of_lives = 3;

bool g_is_colliding_bottom = false;

// ––––– GENERAL FUNCTIONS ––––– //
void switch_to_scene(Scene *scene, int lives)
{    
    g_current_scene = scene;
    g_current_scene->initialise(lives); // DON'T FORGET THIS STEP!
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Dungeon Escape",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_program.SetProjectionMatrix(g_projection_matrix);
    g_program.SetViewMatrix(g_view_matrix);
    
    glUseProgram(g_program.programID);
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    g_levelX = new LevelX();
    g_levelA = new LevelA();
    g_levelB = new LevelB();
    g_levelC = new LevelC();
    
    g_levels[0] = g_levelX;
    g_levels[1] = g_levelA;
    g_levels[2] = g_levelB;
    g_levels[3] = g_levelC;
    
    // Start at level X
    switch_to_scene(g_levels[0], 3);
    
    // Initialise the hearts to represent number of lives
    g_lives = new Entity[g_number_of_lives];
    for (int i = 0; i < g_number_of_lives; i++)
    {
        g_lives[i].set_entity_type(OBJECT);
        g_lives[i].m_texture_id = Utility::load_texture("assets/objects/heart.png");
        g_lives[i].set_width(0.5f);
        g_lives[i].set_height(0.5f);
    }
    
//    g_effects = new Effects(g_projection_matrix, g_view_matrix);
//    g_effects->start(SHRINK, 2.0f);
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_PlayMusic(Mix_LoadMUS("assets/sounds/bgm.mp3"), -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 3.0f);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    if (g_current_scene != g_levelX)
        g_current_scene->m_state.player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_game_is_running = false;
                        break;
                        
                    case SDLK_SPACE:
                        // Jump
                        if (g_current_scene->m_state.player->m_collided_bottom)
                        {
                            g_current_scene->m_state.player->m_is_jumping = true;
                        }
                        break;
                    
                    case SDLK_RETURN:
                        if (g_current_scene == g_levelX)
                        {
                            switch_to_scene(g_levelA, g_number_of_lives);
                        }
                    default:

                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        g_current_scene->m_state.player->m_movement.x = -1.0f;
        g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        g_current_scene->m_state.player->m_movement.x = 1.0f;
        g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->RIGHT];
    }

    if (g_current_scene != g_levelX && glm::length(g_current_scene->m_state.player->m_movement) > 1.0f)
    {
        g_current_scene->m_state.player->m_movement = glm::normalize(g_current_scene->m_state.player->m_movement);
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    delta_time += g_accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }
    
    while (delta_time >= FIXED_TIMESTEP) {
        g_current_scene->update(FIXED_TIMESTEP);
//        g_effects->update(FIXED_TIMESTEP);
        
//        if (g_current_scene != g_levelX)
//        {
//            if (g_is_colliding_bottom == false && g_current_scene->m_state.player->m_collided_bottom) g_effects->start(SHAKE, 1.0f);
//
//            g_is_colliding_bottom = g_current_scene->m_state.player->m_collided_bottom;
//        }
        
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;
    
    // Prevent the camera from showing anything outside of the "edge" of the level
    g_view_matrix = glm::mat4(1.0f);
    for (int i = 0; i < g_number_of_lives; i++) g_lives[i].m_model_matrix = glm::mat4(1.0f);
    
    
    if (g_current_scene != g_levelX)
    {
        if (g_current_scene->m_state.player->get_position().x >= LEVEL1_LEFT_EDGE
            and g_current_scene->m_state.player->get_position().x <= LEVEL1_RIGHT_EDGE) {
            g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->m_state.player->get_position().x, 3.75, 0));
            for (int i = 0; i < g_number_of_lives; i++)
            {
                g_lives[i].m_model_matrix = glm::translate(g_lives[i].m_model_matrix,
                    glm::vec3(g_current_scene->m_state.player->get_position().x + 3.0f + float(i)/2, -0.5f, 0));
            }
            
        } else {
            if (g_current_scene->m_state.player->get_position().x < LEVEL1_LEFT_EDGE)
            {
                g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5, 3.75, 0));
                for (int i = 0; i < g_number_of_lives; i++)
                {
                    g_lives[i].m_model_matrix = glm::translate(g_lives[i].m_model_matrix, glm::vec3(8.0f + float(i)/2, -0.5f, 0));
                }
            }
            else if (g_current_scene->m_state.player->get_position().x > LEVEL1_RIGHT_EDGE)
            {
                g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-8, 3.75, 0));
                for (int i = 0; i < g_number_of_lives; i++)
                {
                    g_lives[i].m_model_matrix = glm::translate(g_lives[i].m_model_matrix, glm::vec3(11.0f + float(i)/2, -0.5f, 0));
                }
            }
            
        }
        
        for (int i = 0; i < g_number_of_lives; i++)
        {
            g_lives[i].m_model_matrix = glm::scale(g_lives[i].m_model_matrix, glm::vec3(0.35f, 0.35f, 1.0f));
        }
        
        // Update the hearts to follow the camera
//        for (int i = 1; i < 1 + g_number_of_lives; i++)
//        {
//            m_state.objects[i].set_position(glm::vec3(g_view_matrix.x + 1.0f + float(i), -2.0f, 1.0f));
//        }
        
        int g_next_scene_id = g_current_scene->m_state.next_scene_id;
        if (!g_player_win && !g_player_lose)
        {
            g_number_of_lives = g_current_scene->m_number_of_lives;
            if (g_next_scene_id == 4) g_player_win = true;
            else if (g_next_scene_id == 5) g_player_lose = true;
            else if (g_next_scene_id != -1)
            {
                switch_to_scene(g_levels[g_next_scene_id], g_number_of_lives);
            }
        }
    }
//    g_view_matrix = glm::translate(g_view_matrix, g_effects->m_view_offset);
}

void render()
{
    g_program.SetViewMatrix(g_view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);
 
    glUseProgram(g_program.programID);
    g_current_scene->render(&g_program);
    for (int i = 0; i < g_number_of_lives; i++)
    {
        g_lives[i].render(&g_program);
    }
//    g_effects->render();
    if (g_player_win)
    {
        Utility::draw_text(&g_program, Utility::load_texture("assets/font1.png"),
                           std::string("YOU WIN!"), 0.5f, 0.0f,
                           glm::vec3(g_current_scene->m_state.player->get_position().x - 6.0f, -3.0f, 0.0f));
    }
    else if (g_player_lose)
    {
        Utility::draw_text(&g_program, Utility::load_texture("assets/font1.png"),
                           std::string("YOU LOST!"), 0.5f, 0.0f,
                           glm::vec3(g_current_scene->m_state.player->get_position().x - 0.75f, -3.0f, 0.0f));
    }
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{    
    SDL_Quit();
    
    delete g_levelA;
    delete g_levelB;
    delete g_effects;
}

// ––––– DRIVER GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_game_is_running)
    {
        process_input();
        if (!g_player_win and !g_player_lose) update();
        
        if (g_current_scene->m_state.next_scene_id >= 0 && g_current_scene->m_state.next_scene_id <= 3)
            switch_to_scene(g_levels[g_current_scene->m_state.next_scene_id], g_number_of_lives);
        
        render();
    }
    
    shutdown();
    return 0;
}
