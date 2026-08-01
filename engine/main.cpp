#include "include/engine/engine.hpp"

int main()
{
    Engine engine;

    engine.init();

    while (engine.isRunning())
    {
        engine.update();
        engine.render();
    }

    engine.shutdown();

    return 0;
}
