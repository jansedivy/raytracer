#pragma once

class vec3 {
  public:
    vec3() {}

    vec3(float x, float y, float z) {
      this->x = x;
      this->y = y;
      this->z = z;
    }

    float x, y, z;

    vec3 operator- (vec3 other) {
      vec3 result;

      result.x = this->x - other.x;
      result.y = this->y - other.y;
      result.z = this->z - other.z;

      return result;
    }

    vec3 operator-= (vec3 other) {
      vec3 result;

      this->x = this->x - other.x;
      this->y = this->y - other.y;
      this->z = this->z - other.z;

      return result;
    }

    vec3 operator+ (vec3 other) {
      vec3 result;

      result.x = this->x + other.x;
      result.y = this->y + other.y;
      result.z = this->z + other.z;

      return result;
    }

    vec3 operator+= (vec3 other) {
      vec3 result;

      this->x = this->x + other.x;
      this->y = this->y + other.y;
      this->z = this->z + other.z;

      return result;
    }

    vec3 operator* (vec3 other) {
      vec3 result;

      result.x = this->x * other.x;
      result.y = this->y * other.y;
      result.z = this->z * other.z;

      return result;
    }

    vec3 operator*= (vec3 other) {
      vec3 result;

      this->x = this->x * other.x;
      this->y = this->y * other.y;
      this->z = this->z * other.z;

      return result;
    }

    vec3 operator* (float a) {
      vec3 result;

      result.x = this->x * a;
      result.y = this->y * a;
      result.z = this->z * a;

      return result;
    }

    vec3 operator*= (float a) {
      vec3 result;

      this->x = this->x * a;
      this->y = this->y * a;
      this->z = this->z * a;

      return result;
    }

    vec3 operator/ (float a) {
      vec3 result;

      result.x = this->x / a;
      result.y = this->y / a;
      result.z = this->z / a;

      return result;
    }

    vec3 operator/= (float a) {
      vec3 result;

      this->x = this->x / a;
      this->y = this->y / a;
      this->z = this->z / a;

      return result;
    }
};

float inner(vec3 a, vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

float length(vec3 value) {
  return sqrt(pow(value.x, 2.0f) + pow(value.y, 2.0f) + pow(value.z, 2.0f));
}

vec3 normalize(vec3 value) {
  vec3 result;

  float len = length(value);

  result.x = value.x / len;
  result.y = value.y / len;
  result.z = value.z / len;

  return result;
}

vec3 reflect(vec3 input, vec3 normal) {
  return input + (normal * inner(input, normal)) * -2.0f;
}
