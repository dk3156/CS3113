#pragma once
#include "Map.h"

enum EntityType { PLATFORM, PLAYER, ENEMY, BREAKABLE, JUMPER, WEAPON, ITEM};
enum AIType     { WALKER, GUARD, ATTACKER, FLYER};
enum AIState    { WALKING, IDLE, ATTACKING, FLYING};

class Entity
{
private:
    bool is_active = true;
    EntityType entity_type;
    AIType ai_type;
    AIState ai_state;
    
    int *animation_right = NULL; // move to the right
    int *animation_left  = NULL; // move to the left
    int *animation_up    = NULL; // move upwards
    int *animation_down  = NULL; // move downwards
    
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 size;
    
    float width  = 0.8f;
    float height = 0.8f;
    
public:
    
    // Static attributes
    static const int SECONDS_PER_FRAME = 4;
    static const int LEFT  = 0,
                     RIGHT = 1,
                     UP    = 2,
                     DOWN  = 3;
    
    // Existing
    GLuint texture_id;
    glm::mat4 model_matrix;
    EntityType type;
    
    // Translating
    float speed;
    float original_speed;
    glm::vec3 movement;
    glm::vec3 acceleration;
    
    // Animating
    int **walking          = new int*[4] { animation_left, animation_right, animation_up, animation_down };
    int *animation_indices = NULL;
    int animation_frames   = 0;
    int animation_index    = 0;
    float animation_time   = 0.0f;
    int animation_cols     = 0;
    int animation_rows     = 0;
    
    // Jumping
    bool is_jumping     = false;
    float jumping_power = 0;
    int jumping_count = 0;
    
    //dashing
    bool is_dashing = false;
    
    // Colliding
    bool collided_top  = false;
    bool collided_bottom = false;
    bool collided_left   = false;
    bool collided_right  = false;
    bool pit_left_detected = false;
    bool pit_right_detected = false;
    
    bool died = false;
    bool open = false;
    bool weapon_enabled = false;
    bool killed = false;

    // Methods
    Entity();
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram *program, GLuint texture_id, int index);
    void update(float delta_time, Entity *player, Entity *object, int object_count, Map *map);
    void render(ShaderProgram *program);
    void activate_ai(Entity *player);
    void ai_walker();
    void ai_guard(Entity *player);
    void ai_attacker(Entity *player);
    void ai_flyer(Entity *player);
    
    void const breakable_collision(Entity *player);
    void jumper_collision(Entity *player);
    void weapon_collision(Entity *player);
    void item_collision(Entity *player);
    void const check_collision_y(Entity *collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity *collidable_entities, int collidable_entity_count);
    void const check_collision_y(Map *map);
    void const check_collision_x(Map *map);
    void const check_collision_y(Entity *player);
//    void const check_collision_x(Entity *player);
    
    bool const check_collision(Entity *other) const;
    bool const check_collision_top(Entity *other) const;
    
    void activate()   { is_active = true;  };
    void deactivate() { is_active = false; };
    void make_dead()  { died = true;       };
    void make_alive() { died = false;      };
    
    EntityType const get_entity_type() const { return entity_type;  };
    AIType    const get_ai_type()      const { return ai_type;      };
    AIState   const get_ai_state()     const { return ai_state;     };
    glm::vec3 const get_position()     const { return position;     };
    glm::vec3 const get_movement()     const { return movement;     };
    glm::vec3 const get_velocity()     const { return velocity;     };
    glm::vec3 const get_acceleration() const { return acceleration; };
    int       const get_width()        const { return width;        };
    int       const get_height()       const { return height;       };
    bool      const get_is_active()    const { return is_active;    };
    
    void const set_entity_type(EntityType new_entity_type)  { entity_type = new_entity_type;   };
    void const set_ai_type(AIType new_ai_type)              { ai_type = new_ai_type;           };
    void const set_ai_state(AIState new_state)              { ai_state = new_state;            };
    void const set_position(glm::vec3 new_position)         { position = new_position;         };
    void const set_movement(glm::vec3 new_movement)         { movement = new_movement;         };
    void const set_velocity(glm::vec3 new_velocity)         { velocity = new_velocity;         };
    void const set_size(glm::vec3 new_size)                 { size = new_size;               };
    void const set_acceleration(glm::vec3 new_acceleration) { acceleration = new_acceleration; };
    void const set_width(float new_width)                   { width = new_width;               };
    void const set_height(float new_height)                 { height = new_height;             };
};
