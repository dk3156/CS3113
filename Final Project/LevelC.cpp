#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

const float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

unsigned int LEVELC_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 0, 1, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2
};

LevelC::~LevelC()
{
    delete [] this->state.enemies;
    delete    this->state.player;
    delete    this->state.map;
    Mix_FreeChunk(this->state.jump_sfx);
    Mix_FreeMusic(this->state.bgm);
}

void LevelC::initialise()
{
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    Mix_HaltChannel(-1);
    
    state.next_scene_id = -1;
    state.mission_failed = false;
    
    GLuint map_texture_id = Utility::load_texture("assets/texture/tileset.png");
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELC_DATA, map_texture_id, 1.0f, 4, 1);
    
    state.font_texture_id = Utility::load_texture("assets/texture/font1.png");
    
    // Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 2.5f;
    state.player->set_acceleration(glm::vec3(0.0f, -7.81f, 0.0f));
    state.player->texture_id = Utility::load_texture("assets/texture/fireboy.png");
    
    // Walking
    state.player->walking[state.player->LEFT]  = new int[4] { 1, 5, 9,  13 };
    state.player->walking[state.player->RIGHT] = new int[4] { 3, 7, 11, 15 };
    state.player->walking[state.player->UP]    = new int[4] { 2, 6, 10, 14 };
    state.player->walking[state.player->DOWN]  = new int[4] { 0, 4, 8,  12 };

    state.player->animation_indices = state.player->walking[state.player->RIGHT];  // start George looking left
    state.player->animation_frames = 4;
    state.player->animation_index  = 0;
    state.player->animation_time   = 0.0f;
    state.player->animation_cols   = 4;
    state.player->animation_rows   = 4;
    state.player->set_height(0.8f);
    state.player->set_width(0.8f);
    
    // Jumping
    state.player->jumping_power = 5.0f;
    
    /**
     Enemies' stuff */
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    state.bgm = Mix_LoadMUS("assets/music/tenno_edited.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0f);
    
    state.jump_sfx = Mix_LoadWAV("assets/sfx/bounce.wav");
}

void LevelC::update(float delta_time) {
    this->state.player->update(delta_time, state.player, state.enemies, this->ENEMY_COUNT, this->state.map);
    
    if (this->state.player->get_position().x > 15.0f) {
        state.mission_success = true;
    }
}

void LevelC::render(ShaderProgram *program)
{
    if(state.mission_failed)
    {
        Utility::draw_text(program, this->state.font_texture_id, "MISSION FAILED!", 0.5f, 0.25f, glm::vec3(this->state.player->get_position().x - 2.0f, -3.0f, 1.0f));
    }
    if(state.mission_success)
    {
        Utility::draw_text(program, this->state.font_texture_id, "MISSION SUCCESSFUL!", 0.5f, 0.25f, glm::vec3(state.player->get_position().x - 2.0f, -3.0f, 0.0f));
    }
    this->state.map->render(program);
    this->state.player->render(program);
}
