#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Util.h"
#include "Entity.h"
#include "Map.h"

struct GameState
{
    Map *map;
    Entity *player;
    Entity *enemies;
    Entity *breakable;
    Entity *jumper;
    Entity *weapon;
    Entity *background;
    Entity *item;
    
    Mix_Music *bgm;
    Mix_Chunk *jump_sfx;
    Mix_Chunk *kill_sfx;
    
    int next_scene_id;
    bool mission_failed = false;
    bool mission_success = false;
    
    GLuint font_texture_id;
};

class Scene {
public:
    int number_of_enemies = 1;
    int num_of_lives = 3;
    
    GameState state;
    
    virtual void initialise() = 0;
    virtual void update(float delta_time) = 0;
    virtual void render(ShaderProgram *program) = 0;
    
    GameState const get_state() const { return this->state; }
};
