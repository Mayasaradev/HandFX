#pragma once 

class Engine 
{
  public:
    bool init();
    void update();
    void render();
    void shutdown();
    bool isRunning() const;

  private:
    bool running = true;
};
