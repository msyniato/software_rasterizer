#pragma once
#include <cmath>
#include <cassert>
#include <vector>

namespace sfr {
  constexpr double PI = 3.14159265358979323846;

  // structs
  struct Vector3 {
    double x, y, z;

    Vector3 operator-(const Vector3& other) const {
      return {
        x - other.x,
        y - other.y,
        z - other.z
      };
    }

    Vector3 operator+(const Vector3& other) const {
      return {
        x + other.x,
        y + other.y,
        z + other.z
      };
    }

    Vector3 operator*(double scalar) const {
      return {
        x * scalar,
        y * scalar,
        z * scalar
      };
    }

    Vector3& operator+=(const Vector3& other) {
      x += other.x;
      y += other.y;
      z += other.z;

      return *this;
    }

    Vector3& operator-=(const Vector3& other) {
      x -= other.x;
      y -= other.y;
      z -= other.z;

      return *this;
    }

    double& operator[](const int i) {
      assert(i >= 0 && i < 3);
      return i == 0 ? x : i == 1 ? y : z;
    }

    double operator[](const int i) const {
      assert(i >= 0 && i < 3);
      return i == 0 ? x : i == 1 ? y : z;
    }

    static Vector3 cross(const Vector3& a, const Vector3& b) {
      return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
      };
    }

    static double dot(const Vector3& a, const Vector3& b) {
      return a.x * b.x +
        a.y * b.y +
        a.z * b.z;
    }

    static Vector3 normalize(const Vector3& v) {
      double length = std::sqrt(
        v.x * v.x +
        v.y * v.y +
        v.z * v.z
      );

      if (length == 0.0)
        return {0.0, 0.0, 0.0};

      return {
        v.x / length,
        v.y / length,
        v.z / length
      };
    }
  };

  struct Vector4 {
    double x, y, z, w;

    double& operator[](const int i) {
      assert(i >= 0 && i < 4);
      return i == 0 ? x : i == 1 ? y : i == 2 ? z : w;
    }

    double operator[](const int i) const {
      assert(i >= 0 && i < 4);
      return i == 0 ? x : i == 1 ? y : i == 2 ? z : w;
    }
  };

  struct Matrix4 {
    double m[4][4];

    double& operator()(int row, int col) {
      assert(row >= 0 && row < 4);
      assert(col >= 0 && col < 4);
      return m[row][col];
    }

    double operator()(int row, int col) const {
      assert(row >= 0 && row < 4);
      assert(col >= 0 && col < 4);
      return m[row][col];
    }

    Matrix4 operator+(const Matrix4& other) const {
      Matrix4 result;

      for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
          result.m[row][col] = m[row][col] + other.m[row][col];
        }
      }

      return result;
    }

    Matrix4 operator-(const Matrix4& other) const {
      Matrix4 result;

      for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
          result.m[row][col] = m[row][col] - other.m[row][col];
        }
      }

      return result;
    }

    Matrix4 operator*(const Matrix4& other) const {
      Matrix4 result{};

      for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
          for (int k = 0; k < 4; ++k) {
            result.m[row][col] +=
              m[row][k] * other.m[k][col];
          }
        }
      }

      return result;
    }

    Vector4 operator*(const Vector4& v) const {
      Vector4 result{};

      for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
          result[row] += m[row][col] * v[col];
        }
      }

      return result;
    }

    Vector3 operator*(const Vector3& v) const {
      Vector4 result = *this * Vector4{v.x, v.y, v.z, 1.0};

      return {
        result.x,
        result.y,
        result.z
      };
    }

    static Matrix4 translation(double x, double y, double z) {
      Matrix4 result{};

      result(0, 0) = 1;
      result(1, 1) = 1;
      result(2, 2) = 1;
      result(3, 3) = 1;

      result(0, 3) = x;
      result(1, 3) = y;
      result(2, 3) = z;

      return result;
    }

    static Matrix4 scale(double x, double y, double z) {
      Matrix4 result{};

      result(0, 0) = x;
      result(1, 1) = y;
      result(2, 2) = z;
      result(3, 3) = 1;

      return result;
    }

    static Matrix4 rotationX(double angle) {
      Matrix4 result{};

      double c = std::cos(angle);
      double s = std::sin(angle);

      result(0, 0) = 1;
      result(1, 1) = c;
      result(1, 2) = -s;
      result(2, 1) = s;
      result(2, 2) = c;
      result(3, 3) = 1;

      return result;
    }

    static Matrix4 rotationY(double angle) {
      Matrix4 result{};

      double c = std::cos(angle);
      double s = std::sin(angle);

      result(0, 0) = c;
      result(0, 2) = s;
      result(1, 1) = 1;
      result(2, 0) = -s;
      result(2, 2) = c;
      result(3, 3) = 1;

      return result;
    }

    static Matrix4 rotationZ(double angle) {
      Matrix4 result{};

      double c = std::cos(angle);
      double s = std::sin(angle);

      result(0, 0) = c;
      result(0, 1) = -s;
      result(1, 0) = s;
      result(1, 1) = c;
      result(2, 2) = 1;
      result(3, 3) = 1;

      return result;
    }
  };

  struct Color {
    uint8_t r, g, b, a;
  };

  struct Mesh {
    std::vector<Vector3> verts;
    std::vector<int> faces;

    int numVerts() const { return verts.size(); }
    int numFaces() const { return faces.size() / 3; }
    Vector3 vert(const int i) const { return verts[i]; }
    Vector3 vert(const int face, const int nthvert) const { return verts[faces[face * 3 + nthvert]]; }
  };

  struct Transform {
    Vector3 position, rotation, scale;
  };

  struct Object {
    Mesh mesh;
    Transform transform;
  };

  struct Camera {
    Transform transform;
    double fov;
    double aspectRatio;
    double nearPlane;
    double farPlane;
  };

  // functions
  Matrix4 calculateModel(const Transform& tr);
  Matrix4 calculateView(const Transform& tr);
  Matrix4 calculatePerspective(double fov, double aspect, double nearPlane, double farPlane);
  Vector4 toClipSpace(const Vector3& vec, const Matrix4& mvp);
  Vector3 toNormalizedCoordinates(const Vector4& vec);
  Vector3 toScreenCoordinates(const Vector3& vec, int w, int h);
  double edgeFunction(const Vector3& a, const Vector3& b, double x, double y);
  Vector3 getCameraForward(Vector3 rotation);
}
