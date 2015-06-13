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

  DebugReadFileResult debugReadEntireFile(const char *name);
  typedef DebugReadFileResult debugReadEntireFileType(const char *name);

  struct Memory {
    bool is_initialized;
    bool should_reload;

    int width;
    int height;

    void *permanent_storage;

    debugReadEntireFileType *debug_read_entire_file;
  };

  void tick(Memory *memory, Input input, GameOffscreenBuffer *buffer);
  typedef void TickType(Memory *memory, Input input, GameOffscreenBuffer *buffer);
}
