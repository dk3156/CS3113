#include "LevelB.h"
#include "Utility.h"

#define LEVEL_WIDTH 42
#define LEVEL_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f
#define LOG(argument) std::cout << argument << '\n'

const float BG_RED     = 0.0f,
            BG_BLUE    = 0.0f,
            BG_GREEN   = 0.0f,
            BG_OPACITY = 1.0f;

unsigned int LEVELB_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 0, 1, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

LevelB::~LevelB()
{
    delete [] this->state.enemies;
    delete    this->state.player;
    delete    this->state.map;
    delete    this->state.weapon;
    delete    this->state.item;
    Mix_FreeChunk(this->state.jump_sfx);
    Mix_FreeMusic(this->state.bgm);
}

void LevelB::initialise()
{
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    state.next_scene_id = -1;
    state.mission_failed = false;
    
    GLuint map_texture_id = Utility::load_texture("assets/texture/greenzone_tileset.png");
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELB_DATA, map_texture_id, 1.0f, 4, 1);
    
    state.font_texture_id = Utility::load_texture("assets/texture/font1.png");
    
    // Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
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
    
    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture("assets/texture/monster.png");
    
    state.enemies = new Entity[this->ENEMY_COUNT];
    state.enemies[0].set_entity_type(ENEMY);
    state.enemies[0].set_ai_type(WALKER);
    state.enemies[0].set_ai_state(WALKING);
    state.enemies[0].texture_id = enemy_texture_id;
    state.enemies[0].set_position(glm::vec3(6.0f, -4.0f, 0.0f));
    state.enemies[0].set_movement(glm::vec3(0.0f));
    state.enemies[0].speed = 1.0f;
    state.enemies[0].set_acceleration(glm::vec3(0.0f, -5.81f, 0.0f));
    
    state.enemies[1].set_entity_type(ENEMY);
    state.enemies[1].set_ai_type(WALKER);
    state.enemies[1].set_ai_state(WALKING);
    state.enemies[1].texture_id = enemy_texture_id;
    state.enemies[1].set_position(glm::vec3(4.0f, -4.0f, 0.0f));
    state.enemies[1].set_movement(glm::vec3(0.0f));
    state.enemies[1].speed = 3.0f;
    state.enemies[1].set_acceleration(glm::vec3(0.0f, -5.81f, 0.0f));
    
    state.enemies[2].set_entity_type(ENEMY);
    state.enemies[2].set_ai_type(WALKER);
    state.enemies[2].set_ai_state(WALKING);
    state.enemies[2].texture_id = enemy_texture_id;
    state.enemies[2].set_position(glm::vec3(5.0f, -1.0f, 0.0f));
    state.enemies[2].set_movement(glm::vec3(0.0f));
    state.enemies[2].speed = 2.0f;
    state.enemies[2].set_acceleration(glm::vec3(0.0f, -5.81f, 0.0f));
    
    state.enemies[3].set_entity_type(ENEMY);
    state.enemies[3].set_ai_type(WALKER);
    state.enemies[3].set_ai_state(WALKING);
    state.enemies[3].texture_id = enemy_texture_id;
    state.enemies[3].set_position(glm::vec3(14.0f, -1.0f, 0.0f));
    state.enemies[3].set_movement(glm::vec3(-1.0f));
    state.enemies[3].speed = 1.0f;
    state.enemies[3].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    
    state.enemies[4].set_entity_type(ENEMY);
    state.enemies[4].set_ai_type(WALKER);
    state.enemies[4].set_ai_state(WALKING);
    state.enemies[4].texture_id = enemy_texture_id;
    state.enemies[4].set_position(glm::vec3(16.0f, -3.0f, 0.0f));
    state.enemies[4].set_movement(glm::vec3(0.0f));
    state.enemies[4].speed = 1.5f;
    state.enemies[4].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    
    state.enemies[5].set_entity_type(ENEMY);
    state.enemies[5].set_ai_type(WALKER);
    state.enemies[5].set_ai_state(WALKING);
    state.enemies[5].texture_id = enemy_texture_id;
    state.enemies[5].set_position(glm::vec3(12.0f, -1.0f, 0.0f));
    state.enemies[5].set_movement(glm::vec3(0.0f));
    state.enemies[5].speed = 1.0f;
    state.enemies[5].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[6].set_entity_type(ENEMY);
    state.enemies[6].set_ai_type(GUARD);
    state.enemies[6].set_ai_state(IDLE);
    state.enemies[6].texture_id = enemy_texture_id;
    state.enemies[6].set_position(glm::vec3(18.0f, -1.0f, 0.0f));
    state.enemies[6].set_movement(glm::vec3(0.0f));
    state.enemies[6].speed = 2.0f;
    state.enemies[6].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[7].set_entity_type(ENEMY);
    state.enemies[7].set_ai_type(GUARD);
    state.enemies[7].set_ai_state(IDLE);
    state.enemies[7].texture_id = enemy_texture_id;
    state.enemies[7].set_position(glm::vec3(20.0f, -1.0f, 0.0f));
    state.enemies[7].set_movement(glm::vec3(0.0f));
    state.enemies[7].speed = 3.0f;
    state.enemies[7].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[8].set_entity_type(ENEMY);
    state.enemies[8].set_ai_type(WALKER);
    state.enemies[8].set_ai_state(IDLE);
    state.enemies[8].texture_id = enemy_texture_id;
    state.enemies[8].set_position(glm::vec3(21.0f, -1.0f, 0.0f));
    state.enemies[8].set_movement(glm::vec3(0.0f));
    state.enemies[8].speed = 1.5f;
    state.enemies[8].set_acceleration(glm::vec3(0.0f, -3.3f, 0.0f));
    
    state.enemies[9].set_entity_type(ENEMY);
    state.enemies[9].set_ai_type(WALKER);
    state.enemies[9].set_ai_state(IDLE);
    state.enemies[9].texture_id = enemy_texture_id;
    state.enemies[9].set_position(glm::vec3(22.0f, -1.0f, 0.0f));
    state.enemies[9].set_movement(glm::vec3(0.0f));
    state.enemies[9].speed = 1.0f;
    state.enemies[9].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[10].set_entity_type(ENEMY);
    state.enemies[10].set_ai_type(WALKER);
    state.enemies[10].set_ai_state(IDLE);
    state.enemies[10].texture_id = enemy_texture_id;
    state.enemies[10].set_position(glm::vec3(23.0f, -5.0f, 0.0f));
    state.enemies[10].set_movement(glm::vec3(0.0f));
    state.enemies[10].speed = 1.0f;
    state.enemies[10].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[11].set_entity_type(ENEMY);
    state.enemies[11].set_ai_type(WALKER);
    state.enemies[11].set_ai_state(IDLE);
    state.enemies[11].texture_id = enemy_texture_id;
    state.enemies[11].set_position(glm::vec3(24.0f, -5.0f, 0.0f));
    state.enemies[11].set_movement(glm::vec3(0.0f));
    state.enemies[11].speed = 1.0f;
    state.enemies[11].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[12].set_entity_type(ENEMY);
    state.enemies[12].set_ai_type(WALKER);
    state.enemies[12].set_ai_state(IDLE);
    state.enemies[12].texture_id = enemy_texture_id;
    state.enemies[12].set_position(glm::vec3(25.0f, -5.0f, 0.0f));
    state.enemies[12].set_movement(glm::vec3(0.0f));
    state.enemies[12].speed = 2.0f;
    state.enemies[12].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[13].set_entity_type(ENEMY);
    state.enemies[13].set_ai_type(WALKER);
    state.enemies[13].set_ai_state(IDLE);
    state.enemies[13].texture_id = enemy_texture_id;
    state.enemies[13].set_position(glm::vec3(26.0f, -5.0f, 0.0f));
    state.enemies[13].set_movement(glm::vec3(0.0f));
    state.enemies[13].speed = 2.0f;
    state.enemies[13].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[14].set_entity_type(ENEMY);
    state.enemies[14].set_ai_type(WALKER);
    state.enemies[14].set_ai_state(IDLE);
    state.enemies[14].texture_id = enemy_texture_id;
    state.enemies[14].set_position(glm::vec3(27.0f, -5.0f, 0.0f));
    state.enemies[14].set_movement(glm::vec3(0.0f));
    state.enemies[14].speed = 2.0f;
    state.enemies[14].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[15].set_entity_type(ENEMY);
    state.enemies[15].set_ai_type(WALKER);
    state.enemies[15].set_ai_state(IDLE);
    state.enemies[15].texture_id = enemy_texture_id;
    state.enemies[15].set_position(glm::vec3(28.0f, -5.0f, 0.0f));
    state.enemies[15].set_movement(glm::vec3(0.0f));
    state.enemies[15].speed = 2.0f;
    state.enemies[15].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[16].set_entity_type(ENEMY);
    state.enemies[16].set_ai_type(WALKER);
    state.enemies[16].set_ai_state(IDLE);
    state.enemies[16].texture_id = enemy_texture_id;
    state.enemies[16].set_position(glm::vec3(29.0f, -5.0f, 0.0f));
    state.enemies[16].set_movement(glm::vec3(0.0f));
    state.enemies[16].speed = 2.0f;
    state.enemies[16].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[17].set_entity_type(ENEMY);
    state.enemies[17].set_ai_type(WALKER);
    state.enemies[17].set_ai_state(IDLE);
    state.enemies[17].texture_id = enemy_texture_id;
    state.enemies[17].set_position(glm::vec3(30.0f, -5.0f, 0.0f));
    state.enemies[17].set_movement(glm::vec3(0.0f));
    state.enemies[17].speed = 2.0f;
    state.enemies[17].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[18].set_entity_type(ENEMY);
    state.enemies[18].set_ai_type(WALKER);
    state.enemies[18].set_ai_state(IDLE);
    state.enemies[18].texture_id = enemy_texture_id;
    state.enemies[18].set_position(glm::vec3(31.0f, -5.0f, 0.0f));
    state.enemies[18].set_movement(glm::vec3(0.0f));
    state.enemies[18].speed = 2.0f;
    state.enemies[18].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[19].set_entity_type(ENEMY);
    state.enemies[19].set_ai_type(WALKER);
    state.enemies[19].set_ai_state(IDLE);
    state.enemies[19].texture_id = enemy_texture_id;
    state.enemies[19].set_position(glm::vec3(32.0f, -5.0f, 0.0f));
    state.enemies[19].set_movement(glm::vec3(0.0f));
    state.enemies[19].speed = 2.0f;
    state.enemies[19].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[20].set_entity_type(ENEMY);
    state.enemies[20].set_ai_type(WALKER);
    state.enemies[20].set_ai_state(IDLE);
    state.enemies[20].texture_id = enemy_texture_id;
    state.enemies[20].set_position(glm::vec3(33.0f, -5.0f, 0.0f));
    state.enemies[20].set_movement(glm::vec3(0.0f));
    state.enemies[20].speed = 2.0f;
    state.enemies[20].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    state.enemies[21].set_entity_type(ENEMY);
    state.enemies[21].set_ai_type(WALKER);
    state.enemies[21].set_ai_state(IDLE);
    state.enemies[21].texture_id = enemy_texture_id;
    state.enemies[21].set_position(glm::vec3(34.0f, -5.0f, 0.0f));
    state.enemies[21].set_movement(glm::vec3(0.0f));
    state.enemies[21].speed = 2.0f;
    state.enemies[21].set_acceleration(glm::vec3(0.0f, -7.3f, 0.0f));
    
    //jumper stuff
    GLuint item_texture_id = Utility::load_texture("assets/texture/item.png");
    state.item = new Entity();
    state.item->set_entity_type(ITEM);
    state.item->set_position(glm::vec3(18.0f, -5.0f, 0.0f));
    state.item->set_size(glm::vec3(0.8f, 0.8f, 1.0f));
    state.item->set_movement(glm::vec3(0.0f));
    state.item->texture_id = item_texture_id;
    
    state.weapon = new Entity();
    state.weapon->set_entity_type(WEAPON);
    state.weapon->texture_id = Utility::load_texture("assets/texture/fire.png");
    state.weapon->set_position(glm::vec3(6.0f, -3.0f, 0.0f));
    state.weapon->set_movement(glm::vec3(-1.0f, 0.0f, 0.0f));
    state.weapon->speed = 10.0f;
    state.weapon->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    state.weapon->deactivate();
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    state.bgm = Mix_LoadMUS("assets/music/Ethernight Club.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0f);
    
    state.jump_sfx = Mix_LoadWAV("assets/sfx/bounce.wav");
    state.kill_sfx =Mix_LoadWAV("assets/music/Maple Leaf Rag.mp3");
}

void LevelB::update(float delta_time) {
    
    if(state.player->get_position().x > 40.0f){
        state.next_scene_id = 2;
    }
    
    if(state.player->killed){
        Mix_PlayChannel(-1, state.kill_sfx, 0);
    }
    
    LOG(state.player->get_position().x);
    for(int i=0; i<this->ENEMY_COUNT; i++) state.enemies[i].update(delta_time, state.player, NULL, 0, this->state.map);
    state.item->update(delta_time, state.player, NULL, 0, this->state.map);
    
    if(state.enemies[1].weapon_enabled){
        state.weapon->activate();
    }
    
    if(state.weapon->get_position().x < 0.0f){
        if(state.enemies[1].died){
            state.weapon->deactivate();
        } else {
            state.weapon->set_position(glm::vec3(6.0f, -3.0f, 0.0f));
        }
    }
    
    
    
    this->state.player->update(delta_time, state.player, state.enemies, this->ENEMY_COUNT, this->state.map);
    this->state.weapon->update(delta_time, state.player, NULL, 0, this->state.map);
   
    
    if(this->num_of_lives == 0)
    {
        state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));
        LOG("you died");
        state.player->deactivate();
        state.mission_failed = true;
    }
    
    if(state.player->died)
    {
        Mix_PlayChannel(-1, this->state.jump_sfx, 0);
        this->num_of_lives--;
        state.next_scene_id = 1;
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

void LevelB::render(ShaderProgram *program)
{
    if(state.player->get_position().x > LEVEL1_LEFT_EDGE){
        Utility::draw_text(program, this->state.font_texture_id, "lives: " + std::to_string(num_of_lives), 0.5f, 0.25f, glm::vec3(state.player->get_position().x - 4.0f, -1.0f, 0.0f));
    } else {
        Utility::draw_text(program, this->state.font_texture_id, "lives: " + std::to_string(num_of_lives), 0.5f, 0.25f, glm::vec3(1.0f, -1.0f, 0.0f));
    }
    if(state.mission_failed)
    {
        Utility::draw_text(program, this->state.font_texture_id, "MISSION FAILED!", 0.5f, 0.25f, glm::vec3(state.player->get_position().x-2.0f, -2.0f, 0.0f));
    }
    this->state.map->render(program);
    this->state.weapon->render(program);
    this->state.player->render(program);
    for (int i = 0; i < ENEMY_COUNT; i++) state.enemies[i].render(program);
    this->state.item->render(program);
}
