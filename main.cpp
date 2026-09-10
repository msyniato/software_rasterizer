#include <chrono>
#include <memory>
#include <SDL3/SDL_main.h>

#include "renderer.hpp"

int main(int argc, char* argv[]) {
  auto renderer = std::make_unique<sfr::Renderer>(1600, 900);

  renderer->startup();

  sfr::Object sphere;
  sfr::Mesh* mesh = &sphere.mesh;
#pragma region SPHERE_CREATION

  double radius = 1.0;
  int segments = 32;
  int rings = 16;

  for (int ring = 0; ring <= rings; ++ring) {
    double phi = sfr::PI * static_cast<double>(ring) / rings;

    double y = std::cos(phi);
    double r = std::sin(phi);

    for (int segment = 0; segment < segments; ++segment) {
      double theta =
        2.0 * sfr::PI * static_cast<double>(segment) / segments;

      double x = r * std::cos(theta);
      double z = r * std::sin(theta);

      mesh->verts.push_back({
        radius * x,
        radius * y,
        radius * z
      });
    }
  }

  for (int ring = 0; ring < rings; ++ring) {
    for (int segment = 0; segment < segments; ++segment) {
      int current =
        ring * segments + segment;

      int next =
        ring * segments +
        (segment + 1) % segments;

      int below =
        (ring + 1) * segments + segment;

      int belowNext =
        (ring + 1) * segments +
        (segment + 1) % segments;

      mesh->faces.push_back(current);
      mesh->faces.push_back(below);
      mesh->faces.push_back(next);

      mesh->faces.push_back(next);
      mesh->faces.push_back(below);
      mesh->faces.push_back(belowNext);
    }
  }
#pragma endregion

  sphere.transform.position = {0.0, 0.0, 0.0};
  sphere.transform.rotation = {0.0, 0.0, 0.0};
  sphere.transform.scale = {1.0, 1.0, 1.0};

  sfr::Object cube;
  mesh = &cube.mesh;
  double size = 1.0;
  double s = size / 2.0;
#pragma region CUBE_CREATION

  mesh->verts = {
    {-s, -s, s},
    {s, -s, s},
    {s, s, s},
    {-s, s, s},

    {s, -s, -s},
    {-s, -s, -s},
    {-s, s, -s},
    {s, s, -s},

    {s, -s, s},
    {s, -s, -s},
    {s, s, -s},
    {s, s, s},

    {-s, -s, -s},
    {-s, -s, s},
    {-s, s, s},
    {-s, s, -s},

    {-s, s, s},
    {s, s, s},
    {s, s, -s},
    {-s, s, -s},

    {-s, -s, -s},
    {s, -s, -s},
    {s, -s, s},
    {-s, -s, s}
  };

  for (int face = 0; face < 6; ++face) {
    int v = face * 4;

    mesh->faces.push_back(v + 0);
    mesh->faces.push_back(v + 2);
    mesh->faces.push_back(v + 1);

    mesh->faces.push_back(v + 0);
    mesh->faces.push_back(v + 3);
    mesh->faces.push_back(v + 2);
  }
#pragma endregion

  cube.transform.position = {2.0, -1.0, 2.0};
  cube.transform.rotation = {0.0, 0.0, 0.0};
  cube.transform.scale = {1.0, 1.0, 1.0};

  sfr::Object pyramid;
  mesh = &pyramid.mesh;

#pragma region PYRAMID_CREATION
  mesh->verts = {
    {-s, 0.0, -s},
    {s, 0.0, -s},
    {s, 0.0, s},
    {-s, 0.0, s},


    {0.0, 1.0, 0.0}
  };

  mesh->faces = {
    0, 2, 1,
    0, 3, 2,

    3, 4, 2,

    2, 4, 1,

    1, 4, 0,

    0, 4, 3
  };
#pragma endregion

  pyramid.transform.position = {2.0, 1.0, -2.0};
  pyramid.transform.rotation = {0.0, 0.0, 0.0};
  pyramid.transform.scale = {1.0, 1.0, 1.0};

  renderer->setCameraPosition(5.0, 5.0, 5.0);
  renderer->setCameraLookAt(0.0, 0.0, 0.0);

  auto start = std::chrono::steady_clock::now();

  while (!renderer->closeNextFrame()) {
    auto now = std::chrono::steady_clock::now();

    double elapsed =
      std::chrono::duration<double>(now - start).count();

    sphere.transform.rotation.x = elapsed * 0.4;
    sphere.transform.rotation.y = elapsed * 0.6;
    sphere.transform.rotation.z = elapsed * 0.2;

    cube.transform.rotation.x = elapsed * 0.3;
    cube.transform.rotation.y = elapsed * 0.7;
    cube.transform.rotation.z = elapsed * 0.5;

    pyramid.transform.rotation.x = elapsed * 0.6;
    pyramid.transform.rotation.y = elapsed * 0.6;
    pyramid.transform.rotation.z = elapsed * 0.6;

    renderer->addObjectToDrawList(sphere);
    renderer->addObjectToDrawList(cube);
    renderer->addObjectToDrawList(pyramid);
    renderer->update();
  }

  renderer->close();

  return 0;
}
