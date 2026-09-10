#pragma once

#include <cassert>
#include <vector>

#include "utils.hpp"

struct SDL_Surface;

namespace sfr {
  class Rasterizer {
  public:
    Rasterizer(SDL_Surface* surface);
    ~Rasterizer() = default;

    // remove copy and move constructor
    Rasterizer(const Rasterizer&) = delete;
    Rasterizer& operator=(const Rasterizer&) = delete;
    Rasterizer(Rasterizer&&) = delete;
    Rasterizer& operator=(Rasterizer&&) = delete;

    void beginFrame();
    void endFrame();

    void clear(Color c);
    void draw(const Object& object, const Matrix4& view, const Matrix4& perspective);
    void drawString(int x, int y, const char* text, Color color);

  private:
    void drawTriangle(const Vector3& a, const Vector3& b, const Vector3& c, Color color);
    void drawChar(int x, int y, char c, Color color);

    SDL_Surface* surface_;
    int width_;
    int height_;
    int pitch_;
    uint32_t* pixels_;
    std::vector<double> depthBuffer_;
  };
}
