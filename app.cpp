#include "app.h"

#include <stdio.h>

void drawBitmap(game_offscreen_buffer *buffer, LoadedBitmap *bitmap, int x, int y) {
}

void drawRect(game_offscreen_buffer *buffer, int minX, int minY, int width, int height) {
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
      *pixel++ = 0xff0000;
    }

    row += buffer->width * buffer->bytesPerPixel;
  }
}

LoadedBitmap debugLoadBMP(debugReadEntireFileType debugReadEntireFile, const char *name) {
  LoadedBitmap result;

  DebugReadFileResult file = debugReadEntireFile("test.bmp");
  if (file.contents) {
  }

  return result;
}


void Tick(Memory *memory, Input input, game_offscreen_buffer *buffer) {
  if (!memory->isInitilized) {
    memory->isInitilized = true;

    memory->app.x = 10;
    memory->app.y = 10;

    memory->app.image = debugLoadBMP(memory->debugReadEntireFile, "test.bmp");
  }

  int pitch = buffer->width * buffer->bytesPerPixel;

  uint8 *row = (uint8 *)buffer->memory;

  for (int y=0; y<buffer->height; y++) {
    uint8 *pixel = (uint8 *)row;

    for (int x=0; x<buffer->width; x++) {
      // blue
      *pixel++ = x;

      // green
      *pixel++ = y;

      // red
      *pixel++ = 0;

      pixel++;
    }

    row += pitch;
  }

  drawRect(buffer, input.mouseX - 50, input.mouseY - 50, 100, 100);

  drawRect(buffer, memory->app.x, memory->app.y, 25, 25);

  drawBitmap(buffer, &memory->app.image, 50, 50);

  for (int i=0; i<10; i++) {
    drawRect(buffer, i*50, 500, 40, 40);
  }
}
