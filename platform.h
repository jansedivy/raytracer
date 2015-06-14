#pragma once

typedef uint8_t uint8;
typedef uint32_t uint32;

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

extern "C" {
  struct Input {
    int mouseX;
    int mouseY;
  };

  struct GameOffscreenBuffer {
    int width;
    int height;
    int bytesPerPixel;

    uint8 *memory;
  };

  struct DebugReadFileResult {
    uint32 fileSize;
    char *contents;
  };

  struct LoadedBitmap {
    int width;
    int height;
    void *memory;
  };

  typedef DebugReadFileResult debugReadEntireFileType(const char *name);
  typedef void PlatformWorkQueueCallback(void *data);
  typedef void add_work_type(struct Queue *queue, PlatformWorkQueueCallback *callback, void *data);
  typedef void complete_all_work_type(struct Queue *queue);
  typedef uint32 get_time_type();

  struct Memory {
    bool is_initialized;
    bool should_reload;

    int width;
    int height;

    void *permanent_storage;

    Queue *queue;

    add_work_type *add_work;
    complete_all_work_type *complete_all_work;
    debugReadEntireFileType *debug_read_entire_file;
    get_time_type *get_time;
  };

  void tick(Memory *memory, Input input, GameOffscreenBuffer *buffer);
  typedef void TickType(Memory *memory, Input input, GameOffscreenBuffer *buffer);
}
