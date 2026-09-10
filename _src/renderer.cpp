#include "renderer.hpp"

#include <algorithm>
#include <cassert>
#include <SDL3/SDL.h>

#include "rasterizer.hpp"

namespace sfr {
  Renderer::Renderer(int width, int height) {
    bool initSuccessful = SDL_Init(SDL_INIT_VIDEO);
    if (!initSuccessful) {
      SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
      assert(false && "Failed to initialize SDL! Check the SDL Log for error code.");
    }

    window_ = SDL_CreateWindow("Software Renderer", width, height, 0);
    if (!window_) {
      SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
      assert(false && "Failed to create SDL window! Check the SDL Log for error code.");
    }

    screenSurface_ = SDL_GetWindowSurface(window_);
    if (!screenSurface_) {
      SDL_Log("Screen surface could not be obtained! SDL error: %s\n", SDL_GetError());
      assert(false && "Failed to obtain SDL surface! Check the SDL Log for error code.");
    }

    rasterizer_ = std::make_unique<Rasterizer>(screenSurface_);
    if (!rasterizer_) {
      assert(false && "Failed to create the rasterizer!");
    }
  }

  Renderer::~Renderer() {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
    screenSurface_ = nullptr;

    SDL_Quit();
  }

  void Renderer::startup() {
    // set camera defaults
    camera_.transform.scale = {1.0, 1.0, 1.0};
    camera_.fov = 90.0;
    int w, h;
    SDL_GetWindowSize(window_, &w, &h);
    camera_.aspectRatio = static_cast<double>(w) / static_cast<double>(h);
    camera_.nearPlane = 0.1;
    camera_.farPlane = 100.0;
  }

  void Renderer::addObjectToDrawList(Object& obj) {
    drawList_.push_back(&obj);
  }

  void Renderer::update() {
    lastTime_ = SDL_GetTicks();

    // input
    SDL_Event e;
    SDL_zero(e);
    while (SDL_PollEvent(&e) == true) {
      if (e.type == SDL_EVENT_QUIT) {
        closeNextFrame_ = true;
      }

      if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (e.button.button == SDL_BUTTON_RIGHT) {
          rightMouseButtonDown_ = true;

          SDL_SetWindowRelativeMouseMode(
            window_,
            true
          );
        }
      }

      if (e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (e.button.button == SDL_BUTTON_RIGHT) {
          rightMouseButtonDown_ = false;

          SDL_SetWindowRelativeMouseMode(
            window_,
            false
          );
        }
      }

      if (e.type == SDL_EVENT_MOUSE_MOTION &&
        rightMouseButtonDown_) {
        camera_.transform.rotation.y -=
          e.motion.xrel * mouseSensitivity_;

        camera_.transform.rotation.x -=
          e.motion.yrel * mouseSensitivity_;
      }
    }
    
    // camera  movement
    double maxPitch = PI / 2.0 - 0.01;
    camera_.transform.rotation.x =
      std::clamp(
        camera_.transform.rotation.x,
        -maxPitch,
        maxPitch
      );
    
    Vector3 forward = getCameraForward(camera_.transform.rotation);
    Vector3 worldUp{0.0, 1.0, 0.0};
    Vector3 right =
      Vector3::normalize(
        Vector3::cross(forward, worldUp)
      );

    const bool* keys = SDL_GetKeyboardState(nullptr);

    double movementSpeed = speed_ * deltaTime_;
    if (keys[SDL_SCANCODE_W])
      camera_.transform.position += forward * movementSpeed;

    if (keys[SDL_SCANCODE_S])
      camera_.transform.position -= forward * movementSpeed;

    if (keys[SDL_SCANCODE_D])
      camera_.transform.position += right * movementSpeed;

    if (keys[SDL_SCANCODE_A])
      camera_.transform.position -= right * movementSpeed;

    if (keys[SDL_SCANCODE_E])
      camera_.transform.position.y += movementSpeed;

    if (keys[SDL_SCANCODE_Q])
      camera_.transform.position.y -= movementSpeed;

    // draw
    rasterizer_->beginFrame();
    rasterizer_->clear({0, 0, 0, 255});

    Matrix4 view = calculateView(camera_.transform);
    Matrix4 projection = calculatePerspective(camera_.fov, camera_.aspectRatio, camera_.nearPlane, camera_.farPlane);

    for (Object* object : drawList_) {
      rasterizer_->draw(*object, view, projection);
    }

    // smooth out fps spikes
    double currentFPS = 1.0 / deltaTime_;
    fps_ = fps_ * 0.9 + currentFPS * 0.1;

    char text[64];
    std::snprintf(
      text,
      sizeof(text),
      "FPS: %.1f",
      fps_
    );

    rasterizer_->drawString(25, 25, text, {255, 255, 255, 255});

    drawList_.clear();
    rasterizer_->endFrame();

    SDL_UpdateWindowSurface(window_);

    currentTime_ = SDL_GetTicks();
    deltaTime_ = (currentTime_ - lastTime_) / 1000.0;
    lastTime_ = currentTime_;
  }

  void Renderer::close() {
  }

  bool Renderer::closeNextFrame() {
    return closeNextFrame_;
  }

  void Renderer::setCameraPosition(double x, double y, double z) {
    camera_.transform.position = {x, y, z};
  }

  void Renderer::setCameraRotation(double x, double y, double z) {
    camera_.transform.rotation = {x, y, z};
  }

  void Renderer::setCameraLookAt(double x, double y, double z) {
    Vector3 target{x, y, z};

    Vector3 direction = target - camera_.transform.position;

    double yaw = std::atan2(-direction.x, -direction.z);

    double horizontalDistance = std::sqrt(direction.x * direction.x + direction.z * direction.z);

    double pitch = std::atan2(direction.y, horizontalDistance);

    camera_.transform.rotation = {
      pitch,
      yaw,
      0.0
    };
  }
}
