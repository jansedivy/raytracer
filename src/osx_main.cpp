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
#include "platform.h"

struct AppCode {
  TickType* tick;

  time_t last_time_write;

  const char *path;
  void* library;
};

time_t get_last_write_time(const char *path) {
  time_t result = 0;

  struct stat FileStat;

  if (stat(path, &FileStat) == 0) {
    result = FileStat.st_mtimespec.tv_sec;
  }

  return result;
}

AppCode load_app_code() {
  AppCode result;

  // TODO(sedivy): move string into constant
  const char *path = "app.dylib";

  void *Lib = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
  if (!Lib) {
    fprintf(stderr, "%s\n", dlerror());
  }

  result.library = Lib;
  result.last_time_write = get_last_write_time(path);
  result.path = path;

  result.tick = (TickType*)dlsym(Lib, "tick");

  return result;
}

void UnloadAppCode(AppCode *code) {
  dlclose(code->library);
}

DebugReadFileResult debug_read_entire_file(const char *path) {
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

struct WorkEntry {
  PlatformWorkQueueCallback *callback;
  void *data;
};

struct Queue {
  int volatile count;
  int volatile completed;
  int volatile next_index;

  WorkEntry entries[256];

  SDL_sem *semaphore;
};

bool do_queue_work(Queue *queue) {
  bool sleep = false;

  int original_next_index = queue->next_index;
  int new_next_index = original_next_index + 1;

  if (original_next_index != queue->count) {
    SDL_bool value = SDL_AtomicCAS((SDL_atomic_t *)&queue->next_index, original_next_index, new_next_index);

    if (value) {
      WorkEntry *entry = queue->entries + original_next_index;
      entry->callback(entry->data);

      SDL_AtomicIncRef((SDL_atomic_t *)&queue->completed);
    }
  } else {
    sleep = true;
  }

  return sleep;
}

static int thread_function(void *data) {
  Queue *queue = (Queue *)data;

  while (true) {
    if (do_queue_work(queue)) {
      SDL_SemWait(queue->semaphore);
    }
  }
}

void add_work(Queue *queue, PlatformWorkQueueCallback *callback, void *data) {
  WorkEntry *entry = queue->entries + queue->count;
  entry->callback = callback;
  entry->data = data;

  SDL_CompilerBarrier();

  SDL_AtomicIncRef((SDL_atomic_t *)&queue->count);
  SDL_SemPost(queue->semaphore);
}

void complete_all_work(Queue *queue) {
  while (queue->completed != queue->count) {
    do_queue_work(queue);
  }

  queue->count = 0;
  queue->completed = 0;
  queue->next_index = 0;
}

uint32 get_time() {
  return SDL_GetTicks();
}

int main() {
  Queue queue = {};
  queue.semaphore = SDL_CreateSemaphore(0);

  for (int i=0; i<SDL_GetCPUCount(); i++) {
    SDL_CreateThread(thread_function, "worker_thread", &queue);
  }

  Memory memory;
  memory.width = 1280;
  memory.height = 720;
  memory.is_initialized = false;
  memory.debug_read_entire_file = debug_read_entire_file;
  memory.should_reload = true;
  memory.permanent_storage = malloc(Megabytes(10));
  memory.get_time = &get_time;

  memory.queue = &queue;
  memory.add_work = add_work;
  memory.complete_all_work = complete_all_work;

  GameOffscreenBuffer buffer;
#if 1
  buffer.width = memory.width/2;
  buffer.height = memory.height/2;
#else
  buffer.width = memory.width;
  buffer.height = memory.height;
#endif
  buffer.bytesPerPixel = 4;
  buffer.memory = (uint8*)malloc(buffer.width*buffer.height*4);

  chdir(SDL_GetBasePath());

  AppCode code = load_app_code();

  SDL_Init(SDL_INIT_EVERYTHING);

  SDL_Window *window = SDL_CreateWindow("RayTracer",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      memory.width, memory.height,
      SDL_WINDOW_OPENGL);

  SDL_GLContext context = SDL_GL_CreateContext(window);

  SDL_GL_SetSwapInterval(-1);

  SDL_Event event;

  bool running = true;

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, buffer.width, buffer.height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  while (running) {
    if (get_last_write_time(code.path) > code.last_time_write) {
      memory.should_reload = true;
      UnloadAppCode(&code);
      code = load_app_code();
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

    code.tick(&memory, input, &buffer);

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

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, buffer.width, buffer.height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer.memory);

    SDL_GL_SwapWindow(window);
  }

  return 1;
}
