#include "Scene.h"

class LevelX : public Scene {
public:
    int ENEMY_COUNT = 1;
    
    ~LevelX();
    
    void initialise(int lives) override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
