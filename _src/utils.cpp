#include "utils.hpp"

namespace sfr {
  Matrix4 calculateModel(const Transform& tr) {
    Vector3 p = tr.position;
    Matrix4 translation = Matrix4::translation(p.x, p.y, p.z);

    Vector3 r = tr.rotation;
    Matrix4 rX = Matrix4::rotationX(r.x);
    Matrix4 rY = Matrix4::rotationY(r.y);
    Matrix4 rZ = Matrix4::rotationZ(r.z);
    Matrix4 rotation = rZ * rY * rX;

    Vector3 s = tr.scale;
    Matrix4 scale = Matrix4::scale(s.x, s.y, s.z);

    return translation * rotation * scale;
  }

  Matrix4 calculateView(const Transform& tr) {
    Vector3 p = tr.position;
    Matrix4 translation = Matrix4::translation(-p.x, -p.y, -p.z);

    Vector3 r = tr.rotation;
    Matrix4 rX = Matrix4::rotationX(-r.x);
    Matrix4 rY = Matrix4::rotationY(-r.y);
    Matrix4 rZ = Matrix4::rotationZ(-r.z);
    Matrix4 rotation = rX * rY * rZ;

    return rotation * translation;
  }

  Matrix4 calculatePerspective(double fov, double aspect, double nearPlane, double farPlane) {
    Matrix4 result{};

    double fovRadians = fov * PI / 180.0;

    double tanHalfFov = std::tan(fovRadians / 2.0);

    result(0, 0) = 1.0 / (aspect * tanHalfFov);
    result(1, 1) = 1.0 / tanHalfFov;

    result(2, 2) = -(farPlane + nearPlane) /
      (farPlane - nearPlane);

    result(2, 3) = -(2.0 * farPlane * nearPlane) /
      (farPlane - nearPlane);

    result(3, 2) = -1.0;

    return result;
  }

  Vector4 toClipSpace(const Vector3& vec, const Matrix4& mvp) {
    return mvp * Vector4{vec.x, vec.y, vec.z, 1.0};
  }

  Vector3 toNormalizedCoordinates(const Vector4& vec) {
    assert(vec.w != 0.0);

    return {
      vec.x / vec.w,
      vec.y / vec.w,
      vec.z / vec.w
    };
  }

  Vector3 toScreenCoordinates(const Vector3& vec, int w, int h) {
    Vector3 result = {vec.x, vec.y, vec.z};

    result.x = (result.x + 1.0) * 0.5 * (w - 1);
    result.y = (1.0 - result.y) * 0.5 * (h - 1);

    return result;
  }
  
  double edgeFunction(const Vector3& a, const Vector3& b, double x, double y) {
    return (x - a.x) * (b.y - a.y) - (y - a.y) * (b.x - a.x);
  }

  Vector3 getCameraForward(Vector3 rotation) {
    double pitch = rotation.x;
    double yaw = rotation.y;

    Vector3 forward{
      -std::sin(yaw) * std::cos(pitch),
      std::sin(pitch),
      -std::cos(yaw) * std::cos(pitch)
    };

    return Vector3::normalize(forward);
  }
}
