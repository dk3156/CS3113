#include "Scene.h"

class LevelB : public Scene {
public:
    int ENEMY_COUNT = 22;
    int BREAK_COUNT = 1;
    int JUMPER_COUNT = 3;
    
    ~LevelB();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
