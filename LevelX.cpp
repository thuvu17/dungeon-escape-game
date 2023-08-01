#include "LevelX.hpp"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

LevelX::~LevelX()
{
    delete [] m_state.enemies;
    delete    m_state.player;
    delete    m_state.map;
    Mix_FreeChunk(m_state.jump_sfx);
    Mix_FreeMusic(m_state.bgm);
}

void LevelX::initialise(int lives)
{
    m_state.next_scene_id = -1;
    
    /**
     BGM and SFX
     */
//    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
//    
//    m_state.bgm = Mix_LoadMUS("assets/sounds/bgm.mp3");
//    Mix_PlayMusic(m_state.bgm, -1);
//    Mix_VolumeMusic(2.0f);
}

void LevelX::update(float delta_time)
{
}

void LevelX::render(ShaderProgram *program)
{
    Utility::draw_text(program, Utility::load_texture("assets/font1.png"), std::string("FIND PRINCESS BUBBLEGUM"), 0.3f, 0.0f, glm::vec3(-2.5f, 2.0f, 0.0f));
    Utility::draw_text(program, Utility::load_texture("assets/font1.png"), std::string("Press enter to start"), 0.3f, 0.0f, glm::vec3(-3.0f, 1.0f, 0.0f));
}
