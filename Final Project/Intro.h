#include "Scene.h"

class Intro : public Scene {
public:
    int ENEMY_COUNT = 0;
    bool start_menu_entered = false;
    
    ~Intro();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
