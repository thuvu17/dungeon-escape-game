#include "LevelB.h"
#include "Utility.h"
#include "helper.hpp"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8
#define OBJECT_COUNT 2

unsigned int LEVELB_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    3, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    8, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    8, 9, 0, 0, 1, 2, 2, 3, 0, 0, 0, 0, 0, 0,
    8, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8
};

LevelB::~LevelB()
{
    delete    m_state.enemies;
    delete    m_state.player;
    delete    m_state.map;
    Mix_FreeChunk(m_state.jump_sfx);
    Mix_FreeMusic(m_state.bgm);
}

void LevelB::initialise(int lives)
{
    m_state.next_scene_id = -1;
    m_number_of_lives = lives;
    
    GLuint map_texture_id = Utility::load_texture("assets/tileset/tilemap.png");
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELB_DATA, map_texture_id, 1.0f, 5, 2);
    
    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
    // Existing
    m_state.player = new Entity();
    m_state.player->set_entity_type(PLAYER);
    m_state.player->set_position(glm::vec3(1.0f, -2.5f, 0.0f));
    m_state.player->set_movement(glm::vec3(0.0f));
    m_state.player->m_speed = 2.5f;
    m_state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    m_state.player->m_texture_id = Utility::load_texture("assets/entities/player.png");
    
    // Player animation
    m_state.player->m_walking[m_state.player->LEFT]  = new int[4] { 4, 5, 6, 7 };
    m_state.player->m_walking[m_state.player->RIGHT] = new int[4] { 0, 1, 2, 3 };

    m_state.player->m_animation_indices = m_state.player->m_walking[m_state.player->RIGHT];  // start George looking left
    m_state.player->m_animation_frames = 4;
    m_state.player->m_animation_index  = 0;
    m_state.player->m_animation_time   = 0.0f;
    m_state.player->m_animation_cols   = 8;
    m_state.player->m_animation_rows   = 1;
    
    // Jumping
    m_state.player->m_jumping_power = 5.0f;
    
    /**
     Enemies' stuff */
    m_state.enemies = new Entity();
    
    GLuint enemy_texture_id = Utility::load_texture("assets/entities/big_knight.png");
    
    m_state.enemies->m_walking[m_state.enemies[1].LEFT]  = new int[4] { 0, 1, 2, 3 };
    m_state.enemies->m_walking[m_state.enemies[1].RIGHT] = new int[4] { 4, 5, 6, 7 };
    
    // Enemy animation
    m_state.enemies->m_animation_indices = m_state.enemies->m_walking[m_state.enemies->LEFT];
    m_state.enemies->m_animation_frames = 4;
    m_state.enemies->m_animation_cols   = 8;
    m_state.enemies->m_animation_rows   = 1;
    
    m_state.enemies->set_entity_type(ENEMY);
    m_state.enemies->set_ai_type(GUARD);
    m_state.enemies->set_ai_state(IDLE);
    m_state.enemies->m_texture_id = enemy_texture_id;
    m_state.enemies->set_position(glm::vec3(6.0f, -4.0f, 0.0f));
    m_state.enemies->set_movement(glm::vec3(0.0f));
    m_state.enemies->m_speed = 1.0f;
    m_state.enemies->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    m_state.enemies->set_width(1.25f);
    m_state.enemies->set_height(1.25f);
    
    // Objects
    m_state.objects = new Entity[OBJECT_COUNT];
    for (int i = 0; i < OBJECT_COUNT; i++)
    {
        m_state.objects[i].set_entity_type(OBJECT);
        m_state.objects[i].m_texture_id = Utility::load_texture("assets/objects/door_closed.png");
        m_state.objects[i].set_width(1.25f);
        m_state.objects[i].set_height(1.25f);
    }
    m_state.objects[0].set_position(glm::vec3(0.8f, -1.9f, 0.0f));
    m_state.objects[1].set_position(glm::vec3(5.0f, -3.9f, 0.0f));
    
    /**
     BGM and SFX
     */
//    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
//    
//    m_state.bgm = Mix_LoadMUS("assets/sounds/bgm.mp3");
//    Mix_PlayMusic(m_state.bgm, -1);
//    Mix_VolumeMusic(4.0f);
    
    m_state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
}

void LevelB::update(float delta_time)
{
    m_state.player->update(delta_time, m_state.player, m_state.enemies, ENEMY_COUNT, m_state.map);
    m_state.enemies->update(delta_time, m_state.player, NULL, 0, m_state.map);
    
    for (int i = 0; i < OBJECT_COUNT + m_number_of_lives; i++)
    {
        m_state.objects[i].update(delta_time, m_state.player, NULL, 0, m_state.map);
    }
    
    // Check if player died
    if (!m_state.player->is_active())
    {
        m_number_of_lives -= 1;
        if (m_number_of_lives <= 0)
        {
            m_state.next_scene_id = 5;
        }
        else
        {
            m_state.player->activate();
            m_state.player->set_position(glm::vec3(1.0f, -2.5f, 0.0f));
        }
        return;
    }
    
    if (door_opened == true)
    {
        if (start_timer <= 0.75f) start_timer += delta_time;
        else m_state.next_scene_id = 3;
    }
    
    if (m_state.player->get_position().x >= 4.5f && m_state.player->get_position().x <= 5.5f
        && m_state.player->get_position().y <= -3.0f && door_opened == false)
    {
        m_state.objects[1].m_texture_id = Utility::load_texture("assets/objects/door_open.png");
        door_opened = true;
        start_timer = delta_time;
    }
}

void LevelB::render(ShaderProgram *program)
{
    draw_background(m_state, program);
    m_state.map->render(program);
    for (int i = 0; i < OBJECT_COUNT; i++)
    {
        m_state.objects[i].render(program);
    }
    m_state.player->render(program);
    m_state.enemies->render(program);
}
