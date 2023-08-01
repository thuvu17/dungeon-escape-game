#include "Scene.h"

class LevelA : public Scene {
public:
    int ENEMY_COUNT = 1;
    bool door_opened = false;
    ~LevelA();
    
    void initialise(int lives) override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
