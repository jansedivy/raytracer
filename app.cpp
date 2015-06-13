#include "app.h"

void drawRect(GameOffscreenBuffer *buffer, int minX, int minY, int width, int height) {
  int maxX = minX + width;
  int maxY = minY + height;

  if (minX < 0) { minX = 0; }
  if (minY < 0) { minY = 0; }
  if (maxX > buffer->width) { maxX = buffer->width; }
  if (maxY > buffer->height) { maxY = buffer->height; }

  uint8 *row = ((uint8 *)buffer->memory) + (minX*buffer->bytesPerPixel + minY*buffer->width*buffer->bytesPerPixel);

  for (int y=minY; y<maxY; y++) {
    uint32 *pixel = (uint32 *)row;

    for (int x=minX; x<maxX; x++) {
      *pixel++ = 0xffff00;
    }

    row += buffer->width * buffer->bytesPerPixel;
  }
}

HitResult ray_sphere_intersect(Ray *ray, Sphere *sphere) {
  HitResult result;
  result.hit = false;
  result.distance = max_distance;

  vec3 offset = ray->start - sphere->center;
  float a = inner(ray->direction, ray->direction);
  float b = 2 * inner(ray->direction, offset);

  float c = inner(offset, offset) - sphere->radius * sphere->radius;
  float discriminant = b * b - 4 * a * c;

  if (discriminant > 0.0f) {
    float t = (-b - sqrt(discriminant)) / (2 * a);
    if (t > 0.0f) {
      vec3 position = ray->start + (ray->direction * t);

      result.hit = true;
      result.reflection = sphere->reflection;
      result.normal = (position - sphere->center) / sphere->radius;
      result.position = position;
      result.distance = t;
      result.color = sphere->color;
    }
  }

  return result;
}

HitResult ray_plane_intersect(Ray *ray, Plane *plane) {
  HitResult result;
  result.hit = false;
  result.distance = max_distance;

  float denom = inner(plane->normal, ray->direction);
  if (fabs(denom) > 0.0f) {
    float t = inner(plane->position - ray->start, plane->normal) / denom;
    if (t >= 0.001f) {
      result.hit = true;
      result.reflection = plane->reflection;
      result.distance = t;
      result.position = ray->start + ray->direction * t;
      result.color = plane->color;
      result.normal = plane->normal;
    }
  }

  return result;
}

uint32 blend_colors(uint32 a, uint32 b, float step) {
  uint8 a_x = a >> 16;
  uint8 a_y = a >> 8;
  uint8 a_z = a >> 0;

  uint8 b_x = b >> 16;
  uint8 b_y = b >> 8;
  uint8 b_z = b >> 0;

  float x = (1.0f - step) * a_x + step * b_x;
  float y = (1.0f - step) * a_y + step * b_y;
  float z = (1.0f - step) * a_z + step * b_z;

  return ((uint8)x << 16) | ((uint8)y << 8) | ((uint8)z << 0);
}

HitResult ray_match_all(Ray *ray, Plane *planes, int planes_count, Sphere *spheres, int spheres_count, Light *light) {
  uint32 reflection_color;

  HitResult final_hit;
  final_hit.hit = false;
  final_hit.distance = max_distance;

  for (int l=0; l<planes_count; l++) {
    Plane *other = planes + l;
    HitResult result = ray_plane_intersect(ray, other);

    if (result.hit && result.distance < final_hit.distance) {
      final_hit = result;

      Ray light_ray;
      light_ray.start = result.position;
      light_ray.direction = normalize(light->position - light_ray.start);

      for (int l=0; l<spheres_count; l++) {
        Sphere *sphere = spheres + l;
        HitResult result = ray_sphere_intersect(&light_ray, sphere);

        if (result.hit && result.distance <= length(light->position - light_ray.start)) {
          final_hit.color = (final_hit.color & 0xfefefe) >> 1;
          break;
        }
      }
    }
  }

  Sphere light_sphere;
  light_sphere.center = light->position;
  light_sphere.radius = 4.0f;
  light_sphere.color = 0xffff55;

  HitResult result = ray_sphere_intersect(ray, &light_sphere);

  if (result.hit && result.distance < final_hit.distance) {
    final_hit = result;
  }

  bool in_shadow = false;
  for (int l=0; l<spheres_count; l++) {
    Sphere *other = spheres + l;

    HitResult result = ray_sphere_intersect(ray, other);

    if (result.hit && result.distance < final_hit.distance) {
      final_hit = result;

      Ray light_ray;
      light_ray.start = result.position + result.normal * 0.001f;
      light_ray.direction = normalize(light->position - light_ray.start);

      for (int l=0; l<spheres_count; l++) {
        Sphere *sphere = spheres + l;
        HitResult result = ray_sphere_intersect(&light_ray, sphere);

        if (result.hit) {
          in_shadow = true;
        }
      }
    }
  }

  if (in_shadow) {
    final_hit.color = (final_hit.color & 0xfefefe) >> 1;
  }

  return final_hit;
}

void tick(Memory *memory, Input input, GameOffscreenBuffer *buffer) {
  App *app = (App*)memory->permanent_storage;

  if (!memory->is_initialized) {
    memory->is_initialized = true;

    app->light.position = vec3(0.0f, -90.0f, 35.0f);
    app->total_time = 0.0f;
    app->animation_number = 0;
  }

  app->total_time += 1.0f/24.0f;

  if (memory->should_reload) {
    memory->should_reload = false;

    drawRect(buffer, 0, 0, buffer->width, buffer->height);

    int pitch = buffer->width * buffer->bytesPerPixel;

    uint8 *row = (uint8 *)buffer->memory;

    float x_pos = 0.0f;
    float y_pos = 0.0f;

    float aspect = (float)memory->width / (float)memory->height;

    float size_x = 1;
    float size_y = ((float)memory->height / aspect) / (float)memory->height;

    Sphere spheres[8];

    for (int i=0; i<array_count(spheres); i++) {
      Sphere *sphere = spheres + i;

      sphere->center.x = (i + 1) * 3290 % 131 - 80.0f;
      sphere->center.y = (i + 1) * 6199 % 89 - 80.0f;
      sphere->center.z = (i + 1) * 7177 % 47;
      sphere->color = ((((i+1) * 321) % 255) << 16 | (((i+1) * 483) % 255) << 8 | (((i+1) * 190) % 255) << 0);
      sphere->radius = ((i+1) * 479) % 20;
      sphere->reflection = 0.3;
    }

    Plane planes[6];
    planes[0].position = vec3(0.0f, 20.0f, 0.0f);
    planes[0].normal = vec3(0.0f, 1.0f, 0.0f);
    planes[0].color = 0xe3e3e3;
    planes[0].reflection = 0.0f;

    planes[1].position = vec3(-110.0f, 20.0f, 0.0f);
    planes[1].normal = vec3(1.0f, 0.0f, 0.0f);
    planes[1].color = 0xff0000;
    planes[1].reflection = 0.0f;

    planes[2].position = vec3(110.0f, 20.0f, 0.0f);
    planes[2].normal = vec3(-1.0f, 0.0f, 0.0f);
    planes[2].color = 0x00ff00;
    planes[2].reflection = 0.0f;

    planes[3].position = vec3(0.0f, 0.0f, 80.0f);
    planes[3].normal = vec3(0.0f, 0.0f, 1.0f);
    planes[3].color = 0xbbbbbb;
    planes[3].reflection = 0.0f;

    planes[4].position = vec3(0.0f, -110.0f, 0.0f);
    planes[4].normal = vec3(0.0f, -1.0f, 0.0f);
    planes[4].color = 0xe3e3e3;
    planes[4].reflection = 0.0f;

    planes[5].position = vec3(0.0f, 0.0f, -130.0f);
    planes[5].normal = vec3(0.0f, 0.0f, -1.0f);
    planes[5].color = 0xbbbbbb;
    planes[5].reflection = 0.0f;

#if 1
    app->light.position = vec3(sin(app->total_time*2.0f) * 100.0f, sin(app->total_time*3.0f) * 60.0f - 40.0f, sin(app->total_time*3.0f) * 20.0f + 40.0f);
    memory->should_reload = true;
#endif

    for (int y=0; y<buffer->height; y++) {
      uint8 *pixel = (uint8 *)row;

      for (int x=0; x<buffer->width; x++) {
        x_pos = (x / (float)buffer->width) * 2.0f - 1.0f;
        y_pos = ((y / (float)buffer->height) * 2.0f - 1.0f) / aspect;

        Ray ray;
        ray.direction = normalize(vec3(x_pos, y_pos, 1.0f));
        ray.start = vec3(0.0f, -40.0f, -120.0f);
#if 0
        ray.start = vec3(
            (((float)input.mouseX / (float)memory->width) * 2.0f - 1.0f) * 100.0f,
            (((float)input.mouseY / (float)memory->height) * 2.0f - 1.0f) * 100.0f,
            -120.0f);

        memory->should_reload = true;
#endif

#if 0
        ray.start = vec3(0.0f, -20.0f, (sin(app->total_time) + 1.0f)/2.0f * -120.0f);
        ray.direction = normalize(vec3(x_pos + sin(app->total_time)/5.0f, y_pos + cos(app->total_time*2.0f)/8.0f, 1.0f));
#endif

#if 0
        memory->should_reload = true;
        app->light.position = vec3(
            (((float)input.mouseX / (float)memory->width) * 2.0f - 1.0f) * 100.0f,
            (((float)input.mouseY / (float)memory->height) * 2.0f - 1.0f) * 100.0f,
            40.0f
            );
#endif

        float distance = max_distance;
        bool first = true;

        float reflection;
        uint32 color;

        for (int i=0; i<4; i++) {
          HitResult hit = ray_match_all(&ray, planes, array_count(planes), spheres, array_count(spheres), &app->light);

          if (hit.hit) {
            if (first) {
              color = hit.color;
              reflection = hit.reflection;
            } else {
              color = blend_colors(color, hit.color, reflection);
              reflection *= hit.reflection;
            }

            first = false;

            ray.start = hit.position;
            ray.direction = reflect(ray.direction, hit.normal);
          }
        }

        *pixel++ = (uint8)(color >> 16);
        *pixel++ = (uint8)(color >> 8);
        *pixel++ = (uint8)(color >> 0);

        *pixel++ = 255;
      }

      row += pitch;
    }

#if 0
    // NOTE: saves every frame as png image in build/animation folder
    char path[100];
    sprintf(path, "animation/output%05d.png", app->animation_number);
    stbi_write_png(path, buffer->width, buffer->height, 4, buffer->memory, buffer->bytesPerPixel * buffer->width);
    app->animation_number += 1;
#endif
  }
}
