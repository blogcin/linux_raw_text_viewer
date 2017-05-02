#ifndef _DISPLAY_H_
#define _DISPLAY_H_

void drawChar(int x, int y, char text, unsigned int color, unsigned int bgcolor,
  unsigned int* pos);

void drawText(int x, int y, char *msg, unsigned int color,
  unsigned int bgcolor, unsigned int* pos);
void drawBox(int x, int y, int x_size, int y_size, unsigned int *pos);
void draw_non_filled_box(int x, int y, int x_size, int y_size, unsigned int *pos);
#endif
