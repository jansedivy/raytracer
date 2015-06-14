#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <math.h>
#include "math_lib.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

#define array_count(arr) (sizeof(arr) / sizeof(arr[0]))

// TODO(sedivy): better max distance
#define max_distance 300000.0f

#include "platform.h"

struct Light {
  vec3 position;
};

struct App {
  float total_time;
  int animation_number;
  int number_of_iterations;
  Light light;
};

struct Sphere {
  vec3 center;
  uint32 color;
  float radius;
  float reflection;
};

struct Plane {
  vec3 position;
  vec3 normal;
  uint32 color;
  float reflection;
};

struct Ray {
  vec3 start;
  vec3 direction;
};

struct HitResult {
  bool hit;
  float distance;
  float reflection;
  vec3 position;
  vec3 normal;
  uint32 color;
};

struct Scene {
  Plane planes[6];
  Sphere spheres[7];
};
