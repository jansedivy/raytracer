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

  struct game_offscreen_buffer {
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
    bool isInitilized;
    bool should_reload;

    int width;
    int height;

    void *permanent_storage;

    debugReadEntireFileType *debugReadEntireFile;
  };

  void Tick(Memory *memory, Input input, game_offscreen_buffer *buffer);
  typedef void TickType(Memory *memory, Input input, game_offscreen_buffer *buffer);
}
