#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 20

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

/**
 STRUCTS AND ENUMS
 */
struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* text_success;
    Entity* text_fail;
};

/**
 CONSTANTS
 */
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
const char SPRITESHEET_FILEPATH[] = "assets/george_0.png";
const char PLATFORM_FILEPATH[]    = "assets/tileset.png";
const char SUCCESS_PATH[] = "assets/mission_success.png";
const char FAIL_PATH[] = "assets/mission_failed.png";

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL  = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER   = 0;   // this value MUST be zero

/**
 VARIABLES
 */
GameState state;

SDL_Window* display_window;
bool game_is_running = true;
bool success = false;
bool fail = false;

ShaderProgram program;
glm::mat4 view_matrix, projection_matrix;

float previous_ticks = 0.0f;
float accumulator = 0.0f;

/**
 GENERAL FUNCTIONS
 */
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
    SDL_Init(SDL_INIT_VIDEO);
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
    
    // Platform stuff
    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);
    
    state.platforms = new Entity[PLATFORM_COUNT];

    state.platforms[PLATFORM_COUNT - 1].texture_id = platform_texture_id;
    state.platforms[PLATFORM_COUNT - 1].set_position(glm::vec3(2.5f, -2.35f, 0.0f));
    state.platforms[PLATFORM_COUNT - 1].set_size(glm::vec3(0.5f, 0.5f, 1.0f));
    state.platforms[PLATFORM_COUNT - 1].update(0.0f, NULL, 0);
    
    state.platforms[PLATFORM_COUNT - 1].walking[0]  = new int[4] { 0, 1, 2, 3 };
    state.platforms[PLATFORM_COUNT - 1].animation_indices = state.platforms[PLATFORM_COUNT - 1].walking[0];
    state.platforms[PLATFORM_COUNT - 1].animation_frames = 4;
    state.platforms[PLATFORM_COUNT - 1].animation_index  = 1;
    state.platforms[PLATFORM_COUNT - 1].animation_time   = 0.0f;
    state.platforms[PLATFORM_COUNT - 1].animation_cols   = 4;
    state.platforms[PLATFORM_COUNT - 1].animation_rows   = 1;
    state.platforms[PLATFORM_COUNT - 1].set_height(0.45f);
    state.platforms[PLATFORM_COUNT - 1].set_width(0.45f);
    
    for (int i=0; i < PLATFORM_COUNT - 8; i++)
    {
        state.platforms[i].texture_id = platform_texture_id;
        state.platforms[i].set_position(glm::vec3(-0.5f * i, -3.0f + (0.5f * i), 0.0f));
        state.platforms[i].set_size(glm::vec3(0.5f, 0.5f, 1.0f));
        state.platforms[i].update(0.0f, NULL, 0);
        
        state.platforms[i].walking[0]  = new int[4] { 0, 1, 2, 3 };
        state.platforms[i].animation_indices = state.platforms[i].walking[0];
        state.platforms[i].animation_frames = 4;
        state.platforms[i].animation_index  = 2;
        state.platforms[i].animation_time   = 0.0f;
        state.platforms[i].animation_cols   = 4;
        state.platforms[i].animation_rows   = 1;
        state.platforms[i].set_height(0.45f);
        state.platforms[i].set_width(0.45f);
    }
    
    for (int i= PLATFORM_COUNT - 8; i < PLATFORM_COUNT - 1; i++)
    {
        state.platforms[i].texture_id = platform_texture_id;
        state.platforms[i].set_position(glm::vec3( 0.5f * (i - 11), -3.0f + 0.5f * (i -11), 0.0f));
        state.platforms[i].set_size(glm::vec3(0.5f, 0.5f, 1.0f));
        state.platforms[i].update(0.0f, NULL, 0);
        
        state.platforms[i].walking[0]  = new int[4] { 0, 1, 2, 3 };
        state.platforms[i].animation_indices = state.platforms[i].walking[0];
        state.platforms[i].animation_frames = 4;
        state.platforms[i].animation_index  = 2;
        state.platforms[i].animation_time   = 0.0f;
        state.platforms[i].animation_cols   = 4;
        state.platforms[i].animation_rows   = 1;
        state.platforms[i].set_height(0.45f);
        state.platforms[i].set_width(0.45f);
    }
    
    // Existing
    state.player = new Entity();
    state.player->set_position(glm::vec3(0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->set_size(glm::vec3(0.5f, 0.5f, 1.0f));
    state.player->speed = 1.0f;
    state.player->set_acceleration(glm::vec3(0.0f, -0.381f, 0.0f));
    state.player->texture_id = load_texture(SPRITESHEET_FILEPATH);
    
    // Walking
    state.player->walking[state.player->LEFT]  = new int[4] { 1, 5, 9,  13 };
    state.player->walking[state.player->RIGHT] = new int[4] { 3, 7, 11, 15 };
    state.player->walking[state.player->UP]    = new int[4] { 2, 6, 10, 14 };
    state.player->walking[state.player->DOWN]  = new int[4] { 0, 4, 8,  12 };

    state.player->animation_indices = state.player->walking[state.player->LEFT];  // start George looking left
    state.player->animation_frames = 4;
    state.player->animation_index  = 0;
    state.player->animation_time   = 0.0f;
    state.player->animation_cols   = 4;
    state.player->animation_rows   = 4;
    state.player->set_height(0.45f);
    state.player->set_width(0.45f);
    
    //
    state.player->jumping_power = 0.5f;
    
    // enable blending
    state.text_success = new Entity();
    state.text_success->texture_id = load_texture(SUCCESS_PATH);
    state.text_success->set_position(glm::vec3(0.0f, 1.0f, 0.0f));
    state.text_success->set_size(glm::vec3(10.0f, 6.0f, 1.0f));
    state.text_success->update(0.0f, NULL, 0);
    
    
    state.text_fail = new Entity();
    state.text_fail->texture_id = load_texture(FAIL_PATH);
    state.text_fail->set_position(glm::vec3(0.0f, 1.0f, 0.0f));
    state.text_fail->set_size(glm::vec3(10.0f, 6.0f, 1.0f));
    state.text_fail->update(0.0f, NULL, 0);
    

    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    state.player->set_movement(glm::vec3(0.0f));
    if (state.player->acceleration.x > 0)
    {
        state.player->acceleration.x -= 0.1f;
    } else if (state.player->acceleration.x < 0)
    {
        state.player->acceleration.x += 0.1f;
    }
    
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
                        state.player->is_jumping = true;
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
        state.player->movement.x = -0.5f;
        state.player->acceleration.x += -0.5f;
        state.player->animation_indices = state.player->walking[state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        state.player->movement.x = 0.5f;
        state.player->acceleration.x += 0.5f;
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
        state.player->update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT);
        delta_time -= FIXED_TIMESTEP;
    }
    
    if (state.player->collided_bottom_success){
        LOG("yes");
        success = true;
    } else if (state.player->collided_top_fail || state.player->collided_bottom_fail)
    {
        LOG("NO");
        fail = true;
    }
    
    
    accumulator = delta_time;
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    state.player->render(&program);
    if(success)
    {
        state.text_success->render(&program);
    }
    if(fail)
    {
        state.text_fail->render(&program);
    }
    
    for (int i = 0; i < PLATFORM_COUNT; i++) state.platforms[i].render(&program);
    
    SDL_GL_SwapWindow(display_window);
}

void shutdown()
{
    SDL_Quit();
    
    delete[] state.platforms;
    delete state.player;
    delete state.text_success;
    delete state.text_fail;
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