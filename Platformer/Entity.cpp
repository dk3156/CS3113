#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define PLATFORM_COUNT 10
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"

Entity::Entity()
{
    position     = glm::vec3(0.0f);
    velocity     = glm::vec3(0.0f);
    acceleration = glm::vec3(0.0f);
    size = glm::vec3(1.0f);
    
    movement = glm::vec3(0.0f);
    
    speed = 0;
    model_matrix = glm::mat4(1.0f);
}

Entity::~Entity()
{
    delete [] animation_up;
    delete [] animation_down;
    delete [] animation_left;
    delete [] animation_right;
    delete [] walking;
}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram *program, GLuint texture_id, int index)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float) (index % animation_cols) / (float) animation_cols;
    float v_coord = (float) (index / animation_cols) / (float) animation_rows;
    
    // Step 2: Calculate its UV size
    float width = 1.0f / (float) animation_cols;
    float height = 1.0f / (float) animation_rows;
    
    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };
    
    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };
    
    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::activate_ai(Entity *player, Entity *weapon)
{
    switch (ai_type)
    {
        case WALKER:
            ai_walker();
            break;
            
        case GUARD:
            ai_guard(player);
            break;
            
        case ATTACKER:
            ai_attacker(player, weapon);
            
            
        case WEAPON:
            check_collision_y(player, 1);
            check_collision_x(player, 1);
        default:
            break;
    }
}

void Entity::ai_walker()
{
    if(pit_right_detected)
    {
        movement = glm::vec3(-1.0f, 0.0f, 0.0f);
    } else if (pit_left_detected)
    {
        movement = glm::vec3(1.0f, 0.0f, 0.0f);
    } else
    {
        movement = glm::vec3(-1.0f, 0.0f, 0.0f);
    }
}

void Entity::ai_attacker(Entity *player, Entity *weapon)
{
    switch(ai_state){
        case IDLE:
            if(glm::distance(position, player->position) < 3.0f)
            {
                ai_state = ATTACKING;
            }
            break;
    
        case ATTACKING:
            use_weapon(weapon);
            break;
            
        default:
            break;
    }
}

void Entity::use_weapon(Entity *weapon)
{
    weapon->activate();
    //loop through wepaon so it can shoot multiple missle
}

void Entity::ai_guard(Entity *player)
{
    switch(ai_state){
        case IDLE:
            if(glm::distance(position, player->position) < 3.0f)
            {
                ai_state = WALKING;
            }
            break;
            
        case WALKING:
            if(position.x > player->get_position().x) {
                movement = glm::vec3(-1.0f, 0.0f, 0.0f);
                if(pit_left_detected)
                {
                    movement = glm::vec3(0.0f, 0.0f, 0.0f);
                }
            } else {
                movement = glm::vec3(1.0f, 0.0f, 0.0f);
                if(pit_right_detected)
                {
                    movement = glm::vec3(0.0f, 0.0f, 0.0f);
                }
            }
            break;
            
        default:
            break;
    }
}

void Entity::update(float delta_time, Entity *player, Entity *objects, int object_count, Map *map)
{
    if (!is_active) return;
 

    collided_top = false;
    collided_bottom = false;
    collided_left   = false;
    collided_right  = false;
    
    //ENEMY ONLY
    if (entity_type == ENEMY) activate_ai(player, objects);

    
    if (animation_indices != NULL)
    {
        if (glm::length(movement) != 0)
        {
            animation_time += delta_time;
            float frames_per_second = (float) 1 / SECONDS_PER_FRAME;
            
            if (animation_time >= frames_per_second)
            {
                animation_time = 0.0f;
                animation_index++;
                
                if (animation_index >= animation_frames)
                {
                    animation_index = 0;
                }
            }
        }
    }
    
    // Our character moves from left to right, so they need an initial velocity
    velocity.x = movement.x * speed;
    
    // Now we add the rest of the gravity physics
    velocity += acceleration * delta_time;
    
    position.y += velocity.y * delta_time;
    check_collision_y(objects, object_count);
    check_collision_y(map);
    
    position.x += velocity.x * delta_time;
    check_collision_x(objects, object_count);
    check_collision_x(map);
    
    // Jump
    if (is_jumping)
    {
        // STEP 1: Immediately return the flag to its original false state
        is_jumping = false;
        
        // STEP 2: The player now acquires an upward velocity
        velocity.y = jumping_power;
    }
    
    model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
    model_matrix = glm::scale(model_matrix, size);
}

void const Entity::check_collision_y(Entity *collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity *collidable_entity = &collidable_entities[i];
        
        if (collidable_entity->entity_type == ENEMY)
        {
            if (check_collision(collidable_entity))
            {
                
                float y_distance = fabs(position.y - collidable_entity->position.y);
                float y_overlap = fabs(y_distance - (height / 2.0f) - (collidable_entity->height / 2.0f));
                if (velocity.y > 0 && collidable_entity->velocity.y == 0) {
                    position.y   -= y_overlap * 2;
                    velocity.y    = 0;
                    collided_top  = true;
                    collidable_entity->collided_bottom = true;
                    y_collided = true;
                } else if (velocity.y < -0.2 && collidable_entity->velocity.y == 0) {
                    LOG(velocity.y);
                    position.y      += y_overlap * 2;
                    velocity.y       = 5.0f;
                    collided_bottom  = true;
                    collidable_entity->collided_top = true;
                }
            }
        }
        
    }
}

void const Entity::check_collision_x(Entity *collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity *collidable_entity = &collidable_entities[i];
        
            if (check_collision(collidable_entity))
            {
                LOG("x collided");
                float x_distance = fabs(position.x - collidable_entity->position.x);
                float x_overlap = fabs(x_distance - (width / 2.0f) - (collidable_entity->width / 2.0f));
                if(velocity.x == 0 && collidable_entity->velocity.x < 0){
                    position.x     -= x_overlap * 50.0f;
                    velocity.y     = 3.0f;
                    velocity.x     = 0;
                    collided_right  = true;
                    collidable_entity->collided_left = true;
                } else if (velocity.x == 0 && collidable_entity->velocity.x > 0){
                    position.x    += x_overlap * 50.0f;
                    velocity.y     = 3.0f;
                    velocity.x     = 0;
                    collided_left  = true;
                    collidable_entity->collided_right = true;
                } else if (velocity.x > 0) {
                    position.x     -= x_overlap * 50.0f;
                    velocity.y     = 3.0f;
                    velocity.x     = 0;
                    collided_right  = true;
                    collidable_entity->collided_left = true;
                } else if (velocity.x < 0) {
                    position.x    += x_overlap * 50.0f;
                    velocity.y     = 3.0f;
                    velocity.x     = 0;
                    collided_left  = true;
                    collidable_entity->collided_right = true;
                }
            }
        }
    }

void const Entity::check_collision_y(Map *map)
{
    if(map == NULL) return;
    // Probes for tiles
    glm::vec3 top = glm::vec3(position.x, position.y + (height / 2), position.z);
    glm::vec3 top_left = glm::vec3(position.x - (width / 2), position.y + (height / 2), position.z);
    glm::vec3 top_right = glm::vec3(position.x + (width / 2), position.y + (height / 2), position.z);
    
    glm::vec3 bottom = glm::vec3(position.x, position.y - (height / 2), position.z);
    glm::vec3 bottom_left = glm::vec3(position.x - (width / 2), position.y - (height / 2), position.z);
    glm::vec3 bottom_right = glm::vec3(position.x + (width / 2), position.y - (height / 2), position.z);
    
    float penetration_x = 0;
    float penetration_y = 0;
    
    if (map->is_solid(top, &penetration_x, &penetration_y) && velocity.y > 0)
    {
        position.y -= penetration_y;
        velocity.y = 0;
        collided_top = true;
    }
    else if (map->is_solid(top_left, &penetration_x, &penetration_y) && velocity.y > 0)
    {
        position.y -= penetration_y;
        velocity.y = 0;
        collided_top = true;
    }
    else if (map->is_solid(top_right, &penetration_x, &penetration_y) && velocity.y > 0)
    {
        position.y -= penetration_y;
        velocity.y = 0;
        collided_top = true;
    }
    
    if (map->is_solid(bottom, &penetration_x, &penetration_y) && velocity.y < 0)
    {
        position.y += penetration_y;
        velocity.y = 0;
        collided_bottom = true;
    }
    else if (map->is_solid(bottom_left, &penetration_x, &penetration_y) && velocity.y < 0)
    {
        position.y += penetration_y;
        velocity.y = 0;
        collided_bottom = true;
        pit_right_detected = true;
        pit_left_detected = false;
    }
    else if (map->is_solid(bottom_right, &penetration_x, &penetration_y) && velocity.y < 0)
    {
        position.y += penetration_y;
        velocity.y = 0;
        collided_bottom = true;
        pit_left_detected = true;
        pit_right_detected = false;
    }
}

void const Entity::check_collision_x(Map *map)
{
    if(map == NULL) return;
    // Probes for tiles
    glm::vec3 left = glm::vec3(position.x - (width / 2), position.y, position.z);
    glm::vec3 right = glm::vec3(position.x + (width / 2), position.y, position.z);
    
    float penetration_x = 0;
    float penetration_y = 0;
    
    if (map->is_solid(left, &penetration_x, &penetration_y) && velocity.x < 0)
    {
        position.x += penetration_x;
        velocity.x = 0;
        collided_left = true;
        pit_left_detected = true;
        pit_right_detected = false;
}
    if (map->is_solid(right, &penetration_x, &penetration_y) && velocity.x > 0)
    {
        position.x -= penetration_x;
        velocity.x = 0;
        collided_right = true;
        pit_right_detected = true;
        pit_left_detected = false;
    }
}

void Entity::render(ShaderProgram *program)
{
    if (!is_active) return;
    
    program->SetModelMatrix(model_matrix);
    
    if (animation_indices != NULL)
    {
        draw_sprite_from_texture_atlas(program, texture_id, animation_indices[animation_index]);
        return;
    }
    
    float vertices[]   = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = {  0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };
    
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

bool const Entity::check_collision(Entity *other) const
{
    // If either entity is inactive, there shouldn't be any collision
    if (other == this) return false;
    
    if (!is_active || !other->is_active) return false;
    
    float x_distance = fabs(position.x - other->position.x) - ((width  + other->width)  / 2.0f);
    float y_distance = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);
    
    return x_distance < 0.0f && y_distance < 0.0f;
}



