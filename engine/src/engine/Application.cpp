#include "engine/Application.hpp"
#include "utils/logger.hpp"

namespace HandFX {

void Application::init()
{
    logMessage("Application initialized");
}

void Application::update()
{
    // game loop / engine update nanti di sini
}

void Application::shutdown()
{
    logMessage("Application shutdown");
}

}
