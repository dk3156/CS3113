#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f
#define LOG(argument) std::cout << argument << '\n'

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
#include "LevelC.h"
#include "Intro.h"
#include <chrono>
/**
 CONSTANTS
 */
const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;

/**
 VARIABLES
 */
Scene *current_scene;
Scene *levels[3];
LevelA *level_a;
LevelB *level_b;
LevelC *level_c;
Intro *start_menu;

SDL_Window* display_window;
bool game_is_running = true;

ShaderProgram program;
glm::mat4 view_matrix, projection_matrix;

float previous_ticks = 0.0f;
float accumulator = 0.0f;

auto t_start = std::chrono::high_resolution_clock::now();
auto t_end = std::chrono::high_resolution_clock::now();

void switch_to_scene(Scene *scene)
{
    current_scene = scene;
    current_scene->initialise();
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    display_window = SDL_CreateWindow("Hello, Scenes!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    view_matrix = glm::mat4(1.0f);
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);
    
    glUseProgram(program.programID);
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    level_a = new LevelA();
    level_b = new LevelB();
    level_c = new LevelC();
    
    levels[0] = level_a;
    levels[1] = level_b;
    levels[2] = level_c;
    
    start_menu = new Intro();
    switch_to_scene(start_menu);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    current_scene->state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:{
                        // Quit the game with a keystroke
                        game_is_running = false;
                        break;
                    }
                    case SDLK_SPACE:{
                        // Jump
                        if (current_scene->state.player->jumping_count < 1)
                        {
                            current_scene->state.player->is_jumping = true;
                            Mix_PlayChannel(-1, current_scene->state.jump_sfx, 0);
                            current_scene->state.player->jumping_count ++;
                        }
                        break;
                    }
                    case SDLK_RETURN:{
                        if(current_scene == start_menu){
                            switch_to_scene(levels[0]);
                        }
                    }
                    case SDLK_z:{
                        t_start = std::chrono::high_resolution_clock::now();
                        if(!current_scene->state.player->collided_bottom){
                            current_scene->state.player->is_dashing = true;
                        }
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
        current_scene->state.player->movement.x = -1.0f;
        current_scene->state.player->animation_indices = current_scene->state.player->walking[current_scene->state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        current_scene->state.player->movement.x = 1.0f;
        current_scene->state.player->animation_indices = current_scene->state.player->walking[current_scene->state.player->RIGHT];
    }
    
    if (glm::length(current_scene->state.player->movement) > 1.0f)
    {
        current_scene->state.player->movement = glm::normalize(current_scene->state.player->movement);
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;
    
    delta_time += accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        accumulator = delta_time;
        return;
    }
    
    while (delta_time >= FIXED_TIMESTEP) {
        current_scene->update(FIXED_TIMESTEP);
        
        delta_time -= FIXED_TIMESTEP;
    }
    
    accumulator = delta_time;
    
    if(current_scene->state.player->is_dashing){
        t_end = std::chrono::high_resolution_clock::now();
        current_scene->state.player->speed = 6.0f;
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start);
        if(elapsed.count() > 300){
            current_scene->state.player->is_dashing = false;
            current_scene->state.player->speed = current_scene->state.player->original_speed;
        }
    }
    
    
    // Prevent the camera from showing anything outside of the "edge" of the level
    view_matrix = glm::mat4(1.0f);
    
    if (current_scene->state.player->get_position().x > LEVEL1_LEFT_EDGE) {
        view_matrix = glm::translate(view_matrix, glm::vec3(-current_scene->state.player->get_position().x, 3.75, 0));
    } else {
        view_matrix = glm::translate(view_matrix, glm::vec3(-5, 3.75, 0));
    }
}

void render()
{
    program.SetViewMatrix(view_matrix);
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    current_scene->render(&program);
    
    SDL_GL_SwapWindow(display_window);
}

void shutdown()
{
    SDL_Quit();
    
    delete start_menu;
    delete level_a;
    delete level_b;
    delete level_c;
}

/**
 DRIVER GAME LOOP
 */
int main(int argc, char* argv[])
{
    initialise();
    
    while (game_is_running)
    {
        process_input();
        update();
        
        if (current_scene->state.next_scene_id >= 0) switch_to_scene(levels[current_scene->state.next_scene_id]);
        
        render();
    }
    
    shutdown();
    return 0;
}
