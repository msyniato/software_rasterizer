#pragma once

#include <memory>
#include "utils.hpp"

struct SDL_Window;
struct SDL_Surface;

namespace sfr {
  class Rasterizer;

  class Renderer {
  public:
    Renderer(int width, int height);
    ~Renderer();

    // remove copy and move constructor
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    // TO-DO: set pixel format
    void startup();
    void update();
    bool closeNextFrame();
    void close();

    void setCameraPosition(double x, double y, double z);
    void setCameraRotation(double x, double y, double z);
    void setCameraLookAt(double x, double y, double z);

    void addObjectToDrawList(Object& obj);

  private:
    SDL_Window* window_ = nullptr;
    SDL_Surface* screenSurface_ = nullptr;
    bool closeNextFrame_ = false;

    Camera camera_;
    bool rightMouseButtonDown_ = false;
    double mouseSensitivity_ = 0.002;
    double speed_ = 3.0;

    uint64_t lastTime_ = 0, currentTime_ = 0;
    double deltaTime_ = 0.00001;
    double fps_ = 0.0;

    std::vector<Object*> drawList_;
    std::unique_ptr<Rasterizer> rasterizer_;
  };
}
