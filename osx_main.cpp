#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/stat.h>

#include "app.h"
#include <OpenGl/gl.h>

#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "SDL2/SDL.h"

struct app_code {
  TickType* Tick;

  time_t LastTimeWrite;

  const char *Path;
  void* Library;
};

time_t GetlastWriteTime(const char *Path) {
  time_t Result = 0;

  struct stat FileStat;

  if (stat(Path, &FileStat) == 0) {
    Result = FileStat.st_mtimespec.tv_sec;
  }

  return Result;
}

app_code LoadAppCode() {
  app_code Result;

  // TODO(sedivy): move string into constant
  const char *Path = "app.dylib";

  void *Lib = dlopen(Path, RTLD_LAZY | RTLD_GLOBAL);
  if (!Lib) {
    fprintf(stderr, "%s\n", dlerror());
  }

  Result.Library = Lib;
  Result.LastTimeWrite = GetlastWriteTime(Path);
  Result.Path = Path;

  Result.Tick = (TickType*)dlsym(Lib, "Tick");

  return Result;
}

void UnloadAppCode(app_code *Code) {
  dlclose(Code->Library);
}

DebugReadFileResult debugReadEntireFile(const char *path) {
  DebugReadFileResult result = {0};

  int file = open(path, O_RDONLY);
  if (file != -1) {
    struct stat fileStat;

    if (fstat(file, &fileStat) == 0) {
      result.fileSize = fileStat.st_size;
      result.contents = (char *)malloc(result.fileSize);

      if (result.contents) {
        ssize_t bytesRead = read(file, result.contents, result.fileSize);
        if (bytesRead != result.fileSize) {
          free(result.contents);
          result.contents = 0;
        }
      }

      return result;
    }
  }

  return result;
}

int main() {
  Memory memory;
  memory.isInitilized = false;
  memory.app.width = 1280;
  memory.app.height = 720;
  memory.debugReadEntireFile = debugReadEntireFile;

  game_offscreen_buffer buffer;
  buffer.width = 1280;
  buffer.height = 720;
  buffer.bytesPerPixel = 4;
  buffer.memory = (uint8*)malloc(buffer.width*buffer.height*4);

  chdir(SDL_GetBasePath());

  app_code Code = LoadAppCode();

  SDL_Init(SDL_INIT_EVERYTHING);

  /* SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); */
  /* SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); */
  /* SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2); */

  /* SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); */
  /* SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); */
  /* SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8); */

  SDL_Window *window = SDL_CreateWindow("Game",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      memory.app.width, memory.app.height,
      SDL_WINDOW_OPENGL);

  SDL_GLContext context = SDL_GL_CreateContext(window);

  SDL_GL_SetSwapInterval(-1);

  SDL_Event event;

  bool running = true;

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, memory.app.width, memory.app.height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  while (running) {
    if (GetlastWriteTime(Code.Path) > Code.LastTimeWrite) {
      UnloadAppCode(&Code);
      Code = LoadAppCode();
    }

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          running = false;
          break;
      }
    }

    Input input;
    SDL_GetMouseState(&input.mouseX, &input.mouseY);

    Code.Tick(&memory, input, &buffer);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLfloat vertices[] = {
      -1, -1, 0,
      -1, 1, 0,
      1, 1, 0,
      1, -1, 0
    };

    GLfloat texture_coords[] = {
      0, 1,
      0, 0,
      1, 0,
      1, 1
    };

    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, texture_coords);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindTexture(GL_TEXTURE_2D, texture);

    glEnable(GL_TEXTURE_2D);

    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    glDisable(GL_TEXTURE_2D);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, buffer.width, buffer.height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer.memory);

    SDL_GL_SwapWindow(window);
  }
}
