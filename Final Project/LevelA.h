#include "Scene.h"

class LevelA : public Scene {
    
public:
    int ENEMY_COUNT = 15;
    int BREAK_COUNT = 15;
    int JUMPER_COUNT = 3;
    
    ~LevelA();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
