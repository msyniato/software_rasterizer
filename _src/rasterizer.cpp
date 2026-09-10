#include "rasterizer.hpp"

#include <cmath>
#include <SDL3/SDL.h>

namespace sfr {
  Rasterizer::Rasterizer(SDL_Surface* surface)
    : surface_(surface),
      width_(surface->w),
      height_(surface->h),
      pitch_(surface->pitch),
      pixels_(nullptr) {
    depthBuffer_.resize(width_ * height_);
  }

  void Rasterizer::beginFrame() {
    if (SDL_MUSTLOCK(surface_)) {
      SDL_LockSurface(surface_);
    }
    pixels_ = static_cast<uint32_t*>(surface_->pixels);
  }

  void Rasterizer::endFrame() {
    if (SDL_MUSTLOCK(surface_)) {
      SDL_UnlockSurface(surface_);
    }
    pixels_ = nullptr;
  }

  void Rasterizer::clear(Color c) {
    std::uint32_t color =
      SDL_MapSurfaceRGB(surface_, c.r, c.g, c.b);

    SDL_FillSurfaceRect(surface_, nullptr, color);

    std::fill(
      depthBuffer_.begin(),
      depthBuffer_.end(),
      std::numeric_limits<double>::infinity()
    );
  }

  void Rasterizer::draw(const Object& object, const Matrix4& view, const Matrix4& projection) {
    Matrix4 model = calculateModel(object.transform);
    Matrix4 mvp = projection * view * model;

    const Mesh& mesh = object.mesh;
    for (int i = 0; i < mesh.numFaces(); i++) {
      // lighting
      Vector3 aa = mesh.vert(i, 0);
      Vector3 bb = mesh.vert(i, 1);
      Vector3 cc = mesh.vert(i, 2);

      Vector3 aw = model * aa;
      Vector3 bw = model * bb;
      Vector3 cw = model * cc;

      Vector3 edge1 = bw - aw;
      Vector3 edge2 = cw - aw;

      Vector3 normal =
        Vector3::normalize(Vector3::cross(edge1, edge2));

      // global directional light
      Vector3 toLight =
        Vector3::normalize({-1.0, -1.0, -1.0});

      double intensity =
        0.2 + 0.8 * std::max(0.0, Vector3::dot(normal, toLight));

      Color shadedColor{
        static_cast<uint8_t>(255 * intensity),
        static_cast<uint8_t>(255 * intensity),
        static_cast<uint8_t>(255 * intensity),
        255
      };

      // projection
      Vector4 a = toClipSpace(aa, mvp);
      Vector4 b = toClipSpace(bb, mvp);
      Vector4 c = toClipSpace(cc, mvp);

      if (a.z + a.w < 0.0 || b.z + b.w < 0.0 || c.z + c.w < 0.0) continue;

      Vector3 an = toNormalizedCoordinates(a);
      Vector3 bn = toNormalizedCoordinates(b);
      Vector3 cn = toNormalizedCoordinates(c);

      Vector3 sa = toScreenCoordinates(an, width_, height_);
      Vector3 sb = toScreenCoordinates(bn, width_, height_);
      Vector3 sc = toScreenCoordinates(cn, width_, height_);

      drawTriangle(sa, sb, sc, shadedColor);
    }
  }

  void Rasterizer::drawTriangle(const Vector3& a, const Vector3& b, const Vector3& c, Color color) {
    // bounding box
    int minX = static_cast<int>(std::floor(std::min({a.x, b.x, c.x})));
    int minY = static_cast<int>(std::floor(std::min({a.y, b.y, c.y})));
    int maxX = static_cast<int>(std::ceil(std::max({a.x, b.x, c.x})));
    int maxY = static_cast<int>(std::ceil(std::max({a.y, b.y, c.y})));

    // clamp to screen
    minX = std::max(minX, 0);
    minY = std::max(minY, 0);
    maxX = std::min(maxX, width_ - 1);
    maxY = std::min(maxY, height_ - 1);

    double area = edgeFunction(b, c, a.x, a.y);
    
    // discard back-face 
    if (area >= 0.0)
      return;

    double invArea = 1.0 / area;

    double startX = minX + 0.5;
    double startY = minY + 0.5;

    double w0Row = edgeFunction(b, c, startX, startY);
    double w1Row = edgeFunction(c, a, startX, startY);
    double w2Row = edgeFunction(a, b, startX, startY);

    double w0StepX = c.y - b.y;
    double w1StepX = a.y - c.y;
    double w2StepX = b.y - a.y;

    double w0StepY = b.x - c.x;
    double w1StepY = c.x - a.x;
    double w2StepY = a.x - b.x;

    double alpha = w0Row * invArea;
    double beta = w1Row * invArea;
    double gamma = w2Row * invArea;

    double zRow = alpha * a.z + beta * b.z + gamma * c.z;

    double zStepX = (w0StepX * a.z + w1StepX * b.z + w2StepX * c.z) * invArea;
    double zStepY = (w0StepY * a.z + w1StepY * b.z + w2StepY * c.z) * invArea;

    std::uint32_t packedColor =
      SDL_MapSurfaceRGB(
        surface_,
        color.r,
        color.g,
        color.b
      );

    int pitchPixels =
      pitch_ / sizeof(std::uint32_t);

    for (int y = minY; y <= maxY; ++y) {
      double w0 = w0Row;
      double w1 = w1Row;
      double w2 = w2Row;

      double z = zRow;

      int depthIndex = y * width_ + minX;
      int pixelIndex = y * pitchPixels + minX;

      for (int x = minX; x <= maxX; ++x) {
        
        if (w0 <= 0.0 && w1 <= 0.0 && w2 <= 0.0) {
          if (z < depthBuffer_[depthIndex]) {
            depthBuffer_[depthIndex] = z;
            pixels_[pixelIndex] = packedColor;
          }
        }

        w0 += w0StepX;
        w1 += w1StepX;
        w2 += w2StepX;

        z += zStepX;
        
        ++depthIndex;
        ++pixelIndex;
      }

      w0Row += w0StepY;
      w1Row += w1StepY;
      w2Row += w2StepY;
      
      zRow += zStepY;
    }
  }

  void Rasterizer::drawChar(int x, int y, char c, Color color) {
    static const uint8_t glyphs[16][7] = {
      // F
      {
        0b11111,
        0b10000,
        0b11110,
        0b10000,
        0b10000,
        0b10000,
        0b10000
      },

      // P
      {
        0b11110,
        0b10001,
        0b10001,
        0b11110,
        0b10000,
        0b10000,
        0b10000
      },

      // S
      {
        0b01111,
        0b10000,
        0b10000,
        0b01110,
        0b00001,
        0b00001,
        0b11110
      },

      // :
      {
        0b00000,
        0b00100,
        0b00100,
        0b00000,
        0b00100,
        0b00100,
        0b00000
      },

      // 0
      {
        0b01110,
        0b10001,
        0b10011,
        0b10101,
        0b11001,
        0b10001,
        0b01110
      },

      // 1
      {
        0b00100,
        0b01100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b01110
      },

      // 2
      {
        0b01110,
        0b10001,
        0b00001,
        0b00010,
        0b00100,
        0b01000,
        0b11111
      },

      // 3
      {
        0b11110,
        0b00001,
        0b00001,
        0b01110,
        0b00001,
        0b00001,
        0b11110
      },

      // 4
      {
        0b00010,
        0b00110,
        0b01010,
        0b10010,
        0b11111,
        0b00010,
        0b00010
      },

      // 5
      {
        0b11111,
        0b10000,
        0b11110,
        0b00001,
        0b00001,
        0b10001,
        0b01110
      },

      // 6
      {
        0b00110,
        0b01000,
        0b10000,
        0b11110,
        0b10001,
        0b10001,
        0b01110
      },

      // 7
      {
        0b11111,
        0b00001,
        0b00010,
        0b00100,
        0b01000,
        0b01000,
        0b01000
      },

      // 8
      {
        0b01110,
        0b10001,
        0b10001,
        0b01110,
        0b10001,
        0b10001,
        0b01110
      },

      // 9
      {
        0b01110,
        0b10001,
        0b10001,
        0b01111,
        0b00001,
        0b00010,
        0b11100
      },

      // .
      {
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00110,
        0b00110
      },

      // space
      {
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000
      }
    };

    int glyphIndex;
    switch (c) {
    case 'F': glyphIndex = 0;
      break;
    case 'P': glyphIndex = 1;
      break;
    case 'S': glyphIndex = 2;
      break;
    case ':': glyphIndex = 3;
      break;

    case '0': glyphIndex = 4;
      break;
    case '1': glyphIndex = 5;
      break;
    case '2': glyphIndex = 6;
      break;
    case '3': glyphIndex = 7;
      break;
    case '4': glyphIndex = 8;
      break;
    case '5': glyphIndex = 9;
      break;
    case '6': glyphIndex = 10;
      break;
    case '7': glyphIndex = 11;
      break;
    case '8': glyphIndex = 12;
      break;
    case '9': glyphIndex = 13;
      break;

    case '.': glyphIndex = 14;
      break;
    case ' ': glyphIndex = 15;
      break;

    default:
      return;
    }

    std::uint32_t packedColor =
      SDL_MapSurfaceRGB(
        surface_,
        color.r,
        color.g,
        color.b
      );

    int pitchPixels =
      pitch_ / sizeof(std::uint32_t);

    for (int row = 0; row < 7; row++) {
      for (int col = 0; col < 5; col++) {
        bool filled =
          glyphs[glyphIndex][row] &
          (1 << (4 - col));

        if (!filled)
          continue;

        int px = x + col;
        int py = y + row;

        if (
          px < 0 || px >= width_ ||
          py < 0 || py >= height_
        )
          continue;

        pixels_[py * pitchPixels + px] =
          packedColor;
      }
    }
  }

  void Rasterizer::drawString(int x, int y, const char* text, Color color) {
    int cursorX = x;

    while (*text != '\0') {
      drawChar(
        cursorX,
        y,
        *text,
        color
      );

      cursorX += 6;
      text++;
    }
  }
}
