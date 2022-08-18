#include "LevelA.h"
#include "Utility.h"
#include <string>

#define LEVEL_WIDTH 62
#define LEVEL_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f
#define LOG(argument) std::cout << argument << '\n'

glm::vec3 view_position;

unsigned int LEVELA_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

LevelA::~LevelA()
{
    delete [] this->state.enemies;
    delete [] this->state.breakable;
    delete    this->state.player;
    delete    this->state.map;
    delete [] this->state.jumper;
    delete    this->state.weapon;
    delete    this->state.background;
    Mix_FreeChunk(this->state.jump_sfx);
    Mix_FreeMusic(this->state.bgm);
}

void LevelA::initialise()
{
    state.next_scene_id = -1;
    state.mission_failed = false;
    view_position = glm::vec3(0.0f);
    
    GLuint map_texture_id = Utility::load_texture("assets/texture/tileset.png");
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELA_DATA, map_texture_id, 1.0f, 4, 1);
    
    state.font_texture_id = Utility::load_texture("assets/texture/font1.png");
    
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
    state.player->original_speed = 2.5f;
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
    state.player->jumping_power = 3.5f;
    
//    breakable
    GLuint breakable_texture_id = Utility::load_texture("assets/texture/breakable.png");
    state.breakable = new Entity[BREAK_COUNT];
    state.breakable[0].set_entity_type(BREAKABLE);
    state.breakable[0].set_position(glm::vec3(14.54f, -2.3f, 0.0f));
    state.breakable[0].set_size(glm::vec3(0.8f, 0.8f, 1.0f));
    state.breakable[0].set_movement(glm::vec3(0.0f));
    state.breakable[0].texture_id = breakable_texture_id;
    
    state.breakable[1].set_entity_type(BREAKABLE);
    state.breakable[1].set_position(glm::vec3(16.54f, -2.3f, 0.0f));
    state.breakable[1].set_size(glm::vec3(0.8f, 0.8f, 1.0f));
    state.breakable[1].set_movement(glm::vec3(0.0f));
    state.breakable[1].texture_id = breakable_texture_id;
    
    state.breakable[2].set_entity_type(BREAKABLE);
    state.breakable[2].set_position(glm::vec3(18.54f, -2.3f, 0.0f));
    state.breakable[2].set_size(glm::vec3(0.8f, 0.8f, 1.0f));
    state.breakable[2].set_movement(glm::vec3(0.0f));
    state.breakable[2].texture_id = breakable_texture_id;
    
    state.breakable[3].set_entity_type(BREAKABLE);
    state.breakable[3].set_position(glm::vec3(18.54f, -1.97f, 0.0f));
    state.breakable[3].set_movement(glm::vec3(0.0f));
    state.breakable[3].set_size(glm::vec3(0.8f, 0.8f, 1.0f));
    state.breakable[3].texture_id = Utility::load_texture("assets/texture/heart.png");
    state.breakable[3].deactivate();
    
    state.breakable[4].set_entity_type(BREAKABLE);
    state.breakable[4].set_position(glm::vec3(47.0f, -4.0f, 0.0f));
    state.breakable[4].set_movement(glm::vec3(0.0f));
    state.breakable[4].texture_id = breakable_texture_id;
    
    state.breakable[5].set_entity_type(BREAKABLE);
    state.breakable[5].set_position(glm::vec3(48.0f, -4.0f, 0.0f));
    state.breakable[5].set_movement(glm::vec3(0.0f));
    state.breakable[5].texture_id = breakable_texture_id;

    state.breakable[6].set_entity_type(BREAKABLE);
    state.breakable[6].set_position(glm::vec3(49.0f, -4.0f, 0.0f));
    state.breakable[6].set_movement(glm::vec3(0.0f));
    state.breakable[6].texture_id = breakable_texture_id;
    
    state.breakable[7].set_entity_type(BREAKABLE);
    state.breakable[7].set_position(glm::vec3(50.0f, -4.0f, 0.0f));
    state.breakable[7].set_movement(glm::vec3(0.0f));
    state.breakable[7].texture_id = breakable_texture_id;
    
    state.breakable[8].set_entity_type(BREAKABLE);
    state.breakable[8].set_position(glm::vec3(51.0f, -4.0f, 0.0f));
    state.breakable[8].set_movement(glm::vec3(0.0f));
    state.breakable[8].texture_id = breakable_texture_id;
    
    state.breakable[9].set_entity_type(BREAKABLE);
    state.breakable[9].set_position(glm::vec3(52.0f, -4.0f, 0.0f));
    state.breakable[9].set_movement(glm::vec3(0.0f));
    state.breakable[9].texture_id = breakable_texture_id;
    
    state.breakable[10].set_entity_type(BREAKABLE);
    state.breakable[10].set_position(glm::vec3(53.0f, -4.0f, 0.0f));
    state.breakable[10].set_movement(glm::vec3(0.0f));
    state.breakable[10].texture_id = breakable_texture_id;
    
    state.breakable[11].set_entity_type(BREAKABLE);
    state.breakable[11].set_position(glm::vec3(54.0f, -4.0f, 0.0f));
    state.breakable[11].set_movement(glm::vec3(0.0f));
    state.breakable[11].texture_id = breakable_texture_id;
    
    state.breakable[12].set_entity_type(BREAKABLE);
    state.breakable[12].set_position(glm::vec3(55.0f, -4.0f, 0.0f));
    state.breakable[12].set_movement(glm::vec3(0.0f));
    state.breakable[12].texture_id = breakable_texture_id;
    
    state.breakable[13].set_entity_type(BREAKABLE);
    state.breakable[13].set_position(glm::vec3(56.0f, -4.0f, 0.0f));
    state.breakable[13].set_movement(glm::vec3(0.0f));
    state.breakable[13].texture_id = breakable_texture_id;
    
    state.breakable[14].set_entity_type(BREAKABLE);
    state.breakable[14].set_position(glm::vec3(57.0f, -4.0f, 0.0f));
    state.breakable[14].set_movement(glm::vec3(0.0f));
    state.breakable[14].texture_id = breakable_texture_id;
    
    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture("assets/texture/monster.png");
    
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
    state.enemies[1].set_ai_type(GUARD);
    state.enemies[1].set_ai_state(IDLE);
    state.enemies[1].texture_id = enemy_texture_id;
    state.enemies[1].set_position(glm::vec3(11.0f, -4.0f, 0.0f));
    state.enemies[1].set_movement(glm::vec3(0.0f));
    state.enemies[1].speed = 1.0f;
    state.enemies[1].set_acceleration(glm::vec3(0.0f, -5.81f, 0.0f));
    
    state.enemies[2].set_entity_type(ENEMY);
    state.enemies[2].set_ai_type(GUARD);
    state.enemies[2].set_ai_state(IDLE);
    state.enemies[2].texture_id = enemy_texture_id;
    state.enemies[2].set_position(glm::vec3(14.54f, -1.0f, 0.0f));
    state.enemies[2].set_movement(glm::vec3(0.0f));
    state.enemies[2].speed = 1.0f;
    state.enemies[2].set_acceleration(glm::vec3(0.0f, -5.81f, 0.0f));
    state.enemies[2].deactivate();
    
    
    state.enemies[3].set_entity_type(ENEMY);
    state.enemies[3].set_ai_type(FLYER);
    state.enemies[3].set_ai_state(WALKING);
    state.enemies[3].texture_id = enemy_texture_id;
    state.enemies[3].set_position(glm::vec3(26.0f, -1.0f, 0.0f));
    state.enemies[3].set_movement(glm::vec3(0.0f));
    state.enemies[3].speed = 1.0f;
    state.enemies[3].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    
    state.enemies[4].set_entity_type(ENEMY);
    state.enemies[4].set_ai_type(FLYER);
    state.enemies[4].set_ai_state(WALKING);
    state.enemies[4].texture_id = enemy_texture_id;
    state.enemies[4].set_position(glm::vec3(30.0f, -3.0f, 0.0f));
    state.enemies[4].set_movement(glm::vec3(0.0f));
    state.enemies[4].speed = 1.0f;
    state.enemies[4].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    
    state.enemies[5].set_entity_type(ENEMY);
    state.enemies[5].set_ai_type(ATTACKER);
    state.enemies[5].set_ai_state(IDLE);
    state.enemies[5].texture_id = enemy_texture_id;
    state.enemies[5].set_position(glm::vec3(44.0f, -5.0f, 0.0f));
    state.enemies[5].set_movement(glm::vec3(0.0f));
    state.enemies[5].speed = 1.0f;
    state.enemies[5].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[6].set_entity_type(ENEMY);
    state.enemies[6].set_ai_type(GUARD);
    state.enemies[6].set_ai_state(IDLE);
    state.enemies[6].texture_id = enemy_texture_id;
    state.enemies[6].set_position(glm::vec3(57.0f, -5.0f, 0.0f));
    state.enemies[6].set_movement(glm::vec3(0.0f));
    state.enemies[6].speed = 1.0f;
    state.enemies[6].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[7].set_entity_type(ENEMY);
    state.enemies[7].set_ai_type(GUARD);
    state.enemies[7].set_ai_state(IDLE);
    state.enemies[7].texture_id = enemy_texture_id;
    state.enemies[7].set_position(glm::vec3(49.0f, -5.0f, 0.0f));
    state.enemies[7].set_movement(glm::vec3(0.0f));
    state.enemies[7].speed = 1.0f;
    state.enemies[7].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[8].set_entity_type(ENEMY);
    state.enemies[8].set_ai_type(GUARD);
    state.enemies[8].set_ai_state(IDLE);
    state.enemies[8].texture_id = enemy_texture_id;
    state.enemies[8].set_position(glm::vec3(50.0f, -5.0f, 0.0f));
    state.enemies[8].set_movement(glm::vec3(0.0f));
    state.enemies[8].speed = 1.0f;
    state.enemies[8].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[9].set_entity_type(ENEMY);
    state.enemies[9].set_ai_type(GUARD);
    state.enemies[9].set_ai_state(IDLE);
    state.enemies[9].texture_id = enemy_texture_id;
    state.enemies[9].set_position(glm::vec3(51.0f, -5.0f, 0.0f));
    state.enemies[9].set_movement(glm::vec3(0.0f));
    state.enemies[9].speed = 1.0f;
    state.enemies[9].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[10].set_entity_type(ENEMY);
    state.enemies[10].set_ai_type(GUARD);
    state.enemies[10].set_ai_state(IDLE);
    state.enemies[10].texture_id = enemy_texture_id;
    state.enemies[10].set_position(glm::vec3(52.0f, -5.0f, 0.0f));
    state.enemies[10].set_movement(glm::vec3(0.0f));
    state.enemies[10].speed = 1.0f;
    state.enemies[10].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[11].set_entity_type(ENEMY);
    state.enemies[11].set_ai_type(GUARD);
    state.enemies[11].set_ai_state(IDLE);
    state.enemies[11].texture_id = enemy_texture_id;
    state.enemies[11].set_position(glm::vec3(53.0f, -5.0f, 0.0f));
    state.enemies[11].set_movement(glm::vec3(0.0f));
    state.enemies[11].speed = 1.0f;
    state.enemies[11].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[12].set_entity_type(ENEMY);
    state.enemies[12].set_ai_type(GUARD);
    state.enemies[12].set_ai_state(IDLE);
    state.enemies[12].texture_id = enemy_texture_id;
    state.enemies[12].set_position(glm::vec3(54.0f, -5.0f, 0.0f));
    state.enemies[12].set_movement(glm::vec3(0.0f));
    state.enemies[12].speed = 1.0f;
    state.enemies[12].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[13].set_entity_type(ENEMY);
    state.enemies[13].set_ai_type(GUARD);
    state.enemies[13].set_ai_state(IDLE);
    state.enemies[13].texture_id = enemy_texture_id;
    state.enemies[13].set_position(glm::vec3(55.0f, -5.0f, 0.0f));
    state.enemies[13].set_movement(glm::vec3(0.0f));
    state.enemies[13].speed = 1.0f;
    state.enemies[13].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[14].set_entity_type(ENEMY);
    state.enemies[14].set_ai_type(GUARD);
    state.enemies[14].set_ai_state(IDLE);
    state.enemies[14].texture_id = enemy_texture_id;
    state.enemies[14].set_position(glm::vec3(56.0f, -5.0f, 0.0f));
    state.enemies[14].set_movement(glm::vec3(0.0f));
    state.enemies[14].speed = 1.0f;
    state.enemies[14].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    //jumper stuff
    state.jumper = new Entity[this->JUMPER_COUNT];
    state.jumper[0].set_entity_type(JUMPER);
    state.jumper[0].set_position(glm::vec3(31.5f, -6.5f, 0.0f));
    state.jumper[0].set_movement(glm::vec3(0.0f));
    state.jumper[0].set_size(glm::vec3(0.8f, 0.8f, 1.0f));
    state.jumper[0].texture_id = Utility::load_texture("assets/texture/jump.png");
    
    state.jumper[1].set_entity_type(JUMPER);
    state.jumper[1].set_position(glm::vec3(22.5f, -3.0f, 0.0f));
    state.jumper[1].set_movement(glm::vec3(0.0f));
    state.jumper[1].set_size(glm::vec3(0.8f, 0.8f, 1.0f));
    state.jumper[1].texture_id = Utility::load_texture("assets/texture/jump.png");
    
    state.jumper[2].set_entity_type(JUMPER);
    state.jumper[2].set_position(glm::vec3(45.5f, -4.0f, 0.0f));
    state.jumper[2].set_movement(glm::vec3(0.0f));
    state.jumper[2].set_size(glm::vec3(0.8f, 0.8f, 1.0f));
    state.jumper[2].texture_id = Utility::load_texture("assets/texture/jump.png");
    
    state.weapon = new Entity();
    state.weapon->set_entity_type(WEAPON);
    state.weapon->texture_id = Utility::load_texture("assets/texture/fire.png");
    state.weapon->set_position(glm::vec3(43.0f, -6.0f, 0.0f));
    state.weapon->set_movement(glm::vec3(-1.0f, 0.0f, 0.0f));
    state.weapon->speed = 10.0f;
    state.weapon->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    state.weapon->deactivate();
    
//    state.background = new Entity();
//    state.background->texture_id = Utility::load_texture("assets/texture/space.jpg");
//    state.background->set_position(glm::vec3(10.0f, -5.0f, -1.0f));
//    state.background->set_size(glm::vec3(30.0f, 10.0f, 1.0f));
    
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    state.bgm = Mix_LoadMUS("assets/music/tenno_edited.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0f);
    
    state.jump_sfx = Mix_LoadWAV("assets/sfx/bounce.wav");
}

void LevelA::update(float delta_time)
{
//    this->state.background->update(delta_time, state.player, NULL, 0, this->state.map);
    for(int i=0; i<this->ENEMY_COUNT; i++) state.enemies[i].update(delta_time, state.player, NULL, 0, this->state.map);
    for(int i=0; i<BREAK_COUNT; i++) {state.breakable[i].update(delta_time, state.player, NULL, 0, this->state.map);}
    for(int i=0; i<JUMPER_COUNT; i++) {state.jumper[i].update(delta_time, state.player, NULL, 0, this->state.map);}
    
    if(state.breakable[0].open){
        state.enemies[2].activate();
    }
    
    if(state.breakable[2].open){
        state.breakable[3].activate();
    }
    
    if(state.breakable[3].open){
        this->num_of_lives ++;
    }
    
    if(state.enemies[5].weapon_enabled){
        state.weapon->activate();
    }
    
    if(state.weapon->get_position().x < 33.0f){
        if(state.enemies[5].died){
            state.weapon->deactivate();
        } else {
            state.weapon->set_position(glm::vec3(43.0f, -6.0f, 0.0f));
        }
    }
    
    this->state.player->update(delta_time, state.player, state.enemies, this->ENEMY_COUNT, this->state.map);
    this->state.weapon->update(delta_time, state.player, NULL, 0, this->state.map);
   
    if(this->num_of_lives == 0)
    {
        LOG("you died");
        state.player->deactivate();
        state.mission_failed = true;
    }
    
    if(state.player->died)
    {
        Mix_PlayChannel(-1, this->state.jump_sfx, 0);
        this->num_of_lives--;
        state.next_scene_id = 0;
    }
    
    //collision detection and dying
    for (int i = 0; i < this->ENEMY_COUNT; i++){
        if(state.enemies[i].died)
        {
            state.enemies[i].deactivate();
        }
       
    }

    
    if(state.player->get_position().x > 60.0f){
        state.next_scene_id = 1;
    }
}

void LevelA::render(ShaderProgram *program)
{
//    this->state.background->render(program);
    if(state.mission_failed)
    {
        Utility::draw_text(program, this->state.font_texture_id, "MISSION FAILED!", 0.5f, 0.25f, glm::vec3(state.player->get_position().x-2.0f, -3.0f, 0.0f));
    }
    
    if(state.player->get_position().x > LEVEL1_LEFT_EDGE){
        Utility::draw_text(program, this->state.font_texture_id, "lives: " + std::to_string(num_of_lives), 0.5f, 0.25f, glm::vec3(state.player->get_position().x - 4.0f, -1.0f, 0.0f));
    } else {
        Utility::draw_text(program, this->state.font_texture_id, "lives: " + std::to_string(num_of_lives), 0.5f, 0.25f, glm::vec3(1.0f, -1.0f, 0.0f));
    }
    
    this->state.weapon->render(program);
    this->state.map->render(program);
    this->state.player->render(program);
    for (int i = 0; i < ENEMY_COUNT; i++) state.enemies[i].render(program);
    for (int i = 0; i < BREAK_COUNT; i++) state.breakable[i].render(program);
    for (int i = 0; i < JUMPER_COUNT; i++) state.jumper[i].render(program);
}
