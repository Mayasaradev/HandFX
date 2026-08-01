#include <engine/engine.hpp>

bool Engine::init()
{
  return true;
}
void Engine::update()
{

}
void Engine::render()
{

}
void Engine::shutdown()
{
  running = false;
}
bool Engine::isRunning() const
{
  return running;
}
