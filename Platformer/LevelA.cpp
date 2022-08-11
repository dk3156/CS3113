#include "LevelA.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8
#define LOG(argument) std::cout << argument << '\n'

unsigned int LEVELA_DATA[] =
{
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    3, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

LevelA::~LevelA()
{
    delete [] this->state.enemies;
    delete    this->state.player;
    delete    this->state.map;
    Mix_FreeChunk(this->state.jump_sfx);
    Mix_FreeMusic(this->state.bgm);
}

void LevelA::initialise()
{
    state.next_scene_id = -1;
    state.num_of_lives = 3;
    state.mission_failed = false;
    
    GLuint map_texture_id = Utility::load_texture("assets/tileset.png");
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELA_DATA, map_texture_id, 1.0f, 4, 1);
    
    state.font_texture_id = Utility::load_texture("assets/font1.png");
    
    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
    // Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 2.5f;
    state.player->set_acceleration(glm::vec3(0.0f, -7.81f, 0.0f));
    state.player->texture_id = Utility::load_texture("assets/fireboy.png");
    
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
    GLuint enemy_texture_id = Utility::load_texture("assets/monster.png");
    
    state.enemies = new Entity[this->ENEMY_COUNT];
    state.enemies[0].set_entity_type(ENEMY);
    state.enemies[0].set_ai_type(WALKER);
    state.enemies[0].set_ai_state(WALKING);
    state.enemies[0].texture_id = enemy_texture_id;
    state.enemies[0].set_position(glm::vec3(8.0f, -5.0f, 0.0f));
    state.enemies[0].set_movement(glm::vec3(0.0f));
    state.enemies[0].speed = 1.0f;
    state.enemies[0].set_acceleration(glm::vec3(0.0f, -5.81f, 0.0f));
    
    state.enemies[1].set_entity_type(ENEMY);
    state.enemies[1].set_ai_type(WALKER);
    state.enemies[1].set_ai_state(WALKING);
    state.enemies[1].texture_id = enemy_texture_id;
    state.enemies[1].set_position(glm::vec3(11.0f, -4.0f, 0.0f));
    state.enemies[1].set_movement(glm::vec3(0.0f));
    state.enemies[1].speed = 1.0f;
    state.enemies[1].set_acceleration(glm::vec3(0.0f, -5.81f, 0.0f));
    
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    state.bgm = Mix_LoadMUS("assets/Pleasant Porridge.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0f);
    
    state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
}

void LevelA::update(float delta_time)
{
    for(int i=0; i<this->ENEMY_COUNT; i++) state.enemies[i].update(delta_time, state.player, NULL, 0, this->state.map);
    this->state.player->update(delta_time, state.player, state.enemies, this->ENEMY_COUNT, this->state.map);
    
    if (this->state.player->get_position().y < -10.0f) state.next_scene_id = 1;
    
    
    //collision detection and dying
    for (int i = 0; i < this->ENEMY_COUNT; i++){
        if(state.enemies[i].collided_top)
        {
            state.enemies[i].deactivate();
        }
        else if ((state.enemies[i].collided_left && state.player->collided_right) || (state.enemies[i].collided_right && state.player->collided_left))
        {
            Mix_PlayChannel(-1, this->state.jump_sfx, 0);
            state.num_of_lives--;
            this->reset = true;
            LOG("died once");
        }
    }
    
    if(state.num_of_lives == 0)
    {
        LOG("you died");
        state.player->deactivate();
        LOG(state.player->get_position().x);
        LOG(state.player->get_position().y);
        state.mission_failed = true;
    }
}

void LevelA::render(ShaderProgram *program)
{
    if(state.mission_failed)
    {
        Utility::draw_text(program, this->state.font_texture_id, "MISSION FAILED!", 0.5f, 0.25f, glm::vec3(state.player->get_position().x-2.0f, -3.0f, 0.0f));
    }
    this->state.map->render(program);
    this->state.player->render(program);
    for (int i = 0; i < ENEMY_COUNT; i++) state.enemies[i].render(program);
}
