#include "font.h"
#include <string.h>

void drawChar(int x, int y, char text, unsigned int color, unsigned int bgcolor,
              unsigned int* pos)
{
  int i, j;
  int index = text * 16;
  unsigned char tmp;
  unsigned char mask;

  for(i = index; i < index + 16; i++){
    mask = 0x80;
    for(j = 0; j < 8; j++){
      tmp = fontdata_8x16[i] & mask;
      if(tmp != 0) {
        pos[1280 * y + (x + j)] = color;
      } else {
        pos[1280 * y + (x + j)] = bgcolor;
      }
      mask = mask >> 1;
    }
    y++;
  }
}

void draw_non_filled_box(int x, int y, int x_size, int y_size, unsigned int *pos) {
  int i = 0;
  int j = 0;
  for (i=x; i<x+x_size; i++) {
    for (j=y; j<(y+y_size); j++) {

      if (i == x || j == y || j == (y+y_size-1) || i == (x+x_size-1)) {
        pos[i + j*1280] = 0xFFFF0000; // Red
      }
    }
  }
}

void drawBox(int x, int y, int x_size, int y_size, unsigned int *pos) {
  int i = 0;
  int j = 0;
  for (i=x; i<x+x_size; i++) {
    for (j=y; j<(y+y_size); j++)
    {
      pos[i + j*1280] = 0xFFFF0000; // Red
    }
  }
}

void drawText(int x, int y, char *msg, unsigned int color,
              unsigned int bgcolor, unsigned int* pos)
{
  int i;

  for(i = 0; i < strlen(msg); i++){
    drawChar(x + (i * 8), y, msg[i], color, bgcolor, pos);
  }
}
