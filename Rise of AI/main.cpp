#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define ENEMY_COUNT 3
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 5

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include <SDL_mixer.h>
#include <string>

/**
 STRUCTS AND ENUMS
 */
struct GameState
{
    Entity* player;
    Entity* enemies;
    Entity* weapon;
    
    Map *map;
    
    Mix_Music  *bgm;
    Mix_Chunk *jump_sfx;
};

/**
 CONSTANTS
 */

const float PLATFORM_OFFSET = 5.0f;

const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

const int FONTBANK_SIZE = 16;

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
const char SPRITESHEET_FILEPATH[] = "assets/fireboy.png";
const char MAP_FILEPATH[] = "assets/greenzone_tileset.png";

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL  = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER   = 0;   // this value MUST be zero

const std::string text_1 = "MISSION FAILED!";
const std::string text_2 = "MISSION SUCCESSFUL!";

unsigned int LEVEL_1_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    2, 2, 1, 1, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2
};

/**
 VARIABLES
 */
GameState state;

SDL_Window* display_window;
bool game_is_running = true;
bool text_bool_1 = false;
bool text_bool_2 = false;
//bool success = false;
//bool fail = false;

ShaderProgram program;
glm::mat4 view_matrix, projection_matrix;

float previous_ticks = 0.0f;
float accumulator = 0.0f;
float spacing= 0.1f;

GLuint font_texture_id;

int num_dead_enemies = 0;

/**
 GENERAL FUNCTIONS
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
        float offset = (spacing) * i;
        
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

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Setting our texture wrapping modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // the last argument can change depending on what you are looking for
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // STEP 5: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    display_window = SDL_CreateWindow("Hello, Entities!",
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
    
    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.
    
    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);
    
    glUseProgram(program.programID);
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    //BGM and SFX
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    state.bgm = Mix_LoadMUS("assets/tenno_edited.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0f);
    
    state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
    /**
     Map Stuff
     */
    GLuint map_texture_id = load_texture(MAP_FILEPATH);
    state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, LEVEL_1_DATA, map_texture_id, 1.0f, 4, 1);
     
    /**
     Enemy Stuff
     */
    GLuint enemy_texture_id = load_texture("assets/monster.png");
    GLuint bullet_texture_id = load_texture("assets/fire.png");
    state.enemies = new Entity[ENEMY_COUNT];
    state.enemies[0].set_entity_type(ENEMY);
    state.enemies[0].set_ai_type(WALKER);
    state.enemies[0].texture_id = enemy_texture_id;
    state.enemies[0].set_position(glm::vec3(5.0f, 0.0f, 0.0f));
    state.enemies[0].set_movement(glm::vec3(0.0f));
    state.enemies[0].speed = 0.5f;
    state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
//    state.enemies[0].deactivate();
    
    state.enemies[1].set_entity_type(ENEMY);
    state.enemies[1].set_ai_type(GUARD);
    state.enemies[1].set_ai_state(IDLE);
    state.enemies[1].texture_id = enemy_texture_id;
    state.enemies[1].set_position(glm::vec3(7.0f, 0.0f, 0.0f));
    state.enemies[1].set_movement(glm::vec3(0.0f));
    state.enemies[1].speed = 0.5f;
    state.enemies[1].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
//    state.enemies[1].deactivate();
    
    state.enemies[2].set_entity_type(ENEMY);
    state.enemies[2].set_ai_type(ATTACKER);
    state.enemies[2].set_ai_state(IDLE);
    state.enemies[2].texture_id = enemy_texture_id;
    state.enemies[2].set_position(glm::vec3(12.0f, 0.0f, 0.0f));
    state.enemies[2].set_movement(glm::vec3(0.0f));
    state.enemies[2].speed = 0.5f;
    state.enemies[2].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    
    state.weapon = new Entity();
    state.weapon->set_entity_type(ENEMY);
    state.weapon->set_ai_type(WEAPON);
    state.weapon->texture_id = bullet_texture_id;
    state.weapon->set_position(glm::vec3(11.0f, -1.0f, 0.0f));
    state.weapon->set_movement(glm::vec3(-1.0f, 0.0f, 0.0f));
    state.weapon->speed = 1.0f;
    state.weapon->deactivate();
    
    // Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(0.0f));
    state.player->set_movement(glm::vec3(0.0f));
//    state.player->set_size(glm::vec3(0.5f, 0.5f, 1.0f));
    state.player->speed = 2.5f;
    state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    state.player->texture_id = load_texture(SPRITESHEET_FILEPATH);
    
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
    
    //
    state.player->jumping_power = 5.0f;
    
    // enable blending
    
    font_texture_id = load_texture("assets/font1.png");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    state.player->set_movement(glm::vec3(0.0f));
//    if (state.player->acceleration.x > 0)
//    {
//        state.player->acceleration.x -= 0.1f;
//    } else if (state.player->acceleration.x < 0)
//    {
//        state.player->acceleration.x += 0.1f;
//    }
    
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
                    case SDLK_q:
                        // Quit the game with a keystroke
                        game_is_running = false;
                        break;
                        
                    case SDLK_SPACE:
                        // Jump
                        if(state.player->collided_bottom)
                        {
                            state.player->is_jumping = true;
                            Mix_PlayChannel(-1, state.jump_sfx, 0);
                        }
                        break;
                        
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
        state.player->movement.x = -1.0f;
//        state.player->acceleration.x += -0.5f;
        state.player->animation_indices = state.player->walking[state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        state.player->movement.x = 1.0f;
//        state.player->acceleration.x += 0.5f;
        state.player->animation_indices = state.player->walking[state.player->RIGHT];
    }
    
    // This makes sure that the player can't move faster diagonally
    if (glm::length(state.player->movement) > 1.0f)
    {
        state.player->movement = glm::normalize(state.player->movement);
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
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        if (!state.player->get_is_active() || num_dead_enemies == ENEMY_COUNT) return;
        state.player->update(FIXED_TIMESTEP, state.player, state.enemies, 3, state.map);
        state.weapon->update(FIXED_TIMESTEP, state.player, NULL, 0, NULL);
        if(state.weapon->collided_right || state.weapon->collided_left)
        {
            state.player->deactivate();
            text_bool_1 = true;
        } 
        for (int i = 0; i < ENEMY_COUNT; i++)
        {
            if(state.enemies[i].collided_top)
            {
                if(state.enemies[i].get_is_active()){
                    num_dead_enemies++;
                }
                state.enemies[i].deactivate();
                
            }
            else if ((state.enemies[i].collided_left && state.player->collided_right) || (state.enemies[i].collided_right && state.player->collided_left))
            {
                state.player->deactivate();
                text_bool_1 = true;
            }
            state.enemies[i].update(FIXED_TIMESTEP, state.player, state.weapon, 1, state.map);
            
        }
        if (num_dead_enemies == ENEMY_COUNT) text_bool_2 = true;
        delta_time -= FIXED_TIMESTEP;
    }
    
    accumulator = delta_time;
    
    view_matrix = glm::mat4(1.0f);
    view_matrix = glm::translate(view_matrix, glm::vec3(-state.player->get_position().x, 0.0f, 0.0f));
}

void render()
{
    program.SetViewMatrix(view_matrix);
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    if(text_bool_1) DrawText(&program, font_texture_id, text_1, 0.5f, 0.25f, glm::vec3(state.player->get_position().x - 1.7f, 2.0f, 0.0f));
    if(text_bool_2) DrawText(&program, font_texture_id, text_2, 0.5f, 0.25f, glm::vec3(state.player->get_position().x - 1.7f, 2.0f, 0.0f));
    state.player->render(&program);
    state.map->render(&program);
    for (int i = 0; i < ENEMY_COUNT; i++) state.enemies[i].render(&program);
    state.weapon->render(&program);
    SDL_GL_SwapWindow(display_window);
}

void shutdown()
{
    SDL_Quit();
    delete[] state.enemies;
    delete state.player;
    delete state.map;
    delete state.weapon;
    Mix_FreeChunk(state.jump_sfx);
    Mix_FreeMusic(state.bgm);
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
        render();
    }
    
    shutdown();
    return 0;
}
