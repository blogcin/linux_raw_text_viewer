#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <termios.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <linux/fb.h>

#include "videodev2.h"
#include "hdmi_api.h"
#include "hdmi_lib.h"
#include "s3c_lcd.h"
#include "font.h"

#include "file.h"
#include "display.h"

#define FB_DEV	"/dev/fb0"

typedef struct FrameBuffer {
	int         fd;
	void        *start;
	size_t      length;
	struct fb_var_screeninfo var;
	struct fb_fix_screeninfo fix;
} FrameBuffer;


// 키보드 이벤트를 처리하기 위한 함수, Non-Blocking 입력을 지원
//  값이 없으면 0을 있으면 해당 Char값을 리턴
static int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if(ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}

int fb_open(FrameBuffer *fb){
	int fd;
	int ret;

	fd = open(FB_DEV, O_RDWR);
	if(fd < 0){
		perror("FB Open");
		return -1;
	}
	ret = ioctl(fd, FBIOGET_FSCREENINFO, &fb->fix);
	if(ret < 0){
		perror("FB ioctl");
		close(fd);
		return -1;
	}
	ret = ioctl(fd, FBIOGET_VSCREENINFO, &fb->var);
	if(ret < 0){
		perror("FB ioctl");
		close(fd);
		return -1;
	}
	fb->start = (unsigned char *)mmap (0, fb->fix.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(fb->start == NULL){
		perror("FB mmap");
		close(fd);
		return -1;
	}
	fb->length = fb->fix.smem_len;
	fb->fd = fd;
	return fd;
}

void fb_close(FrameBuffer *fb){
	if(fb->fd > 0)
		close(fb->fd);
	if(fb->start > 0){
		msync(fb->start, fb->length, MS_INVALIDATE | MS_SYNC);
		munmap(fb->start, fb->length);
	}
}

void draw_ui(unsigned int *pos, const char *file_name, int current_line, struct fileviewer *fv) {
	int textline_index = 0;
	char *linebuf;
	int line_number;
	char *line_number_buf;
	char *filename_buf;
	char *page_number_buf;
	int page_number;

	memset(pos, 0x00, 1280*720*4);

	line_number_buf = (char*)malloc(sizeof(char) * 100);
	page_number_buf = (char*)malloc(sizeof(char) * (strlen("Page : ") + 100));
	filename_buf = (char*)malloc(sizeof(char) * (strlen(file_name) + strlen("filename : ") + 15));

	if (line_number_buf == NULL || page_number_buf == NULL || filename_buf == NULL) {
		printf("Memory error\n");
		return ;
	}

	sprintf(filename_buf, "filename : %s", file_name);

	// Filename
	draw_non_filled_box(10, 10, 400, 50, pos);
	drawText(20, 20, filename_buf, 0xFFFF0000, 0, pos);

	page_number = current_line / 30;

	sprintf(page_number_buf, "Page : %d / %d", page_number+1, ((fv->lines-30) / 30)+1);

	// Page Menu
	draw_non_filled_box(1100, 10, 150, 50, pos);
	drawText(1120, 20, page_number_buf, 0xFFFF0000, 0, pos);

	// Menu
	draw_non_filled_box(10, 650, 1250, 50, pos);
	drawText(30, 660, "q : quit        a : page up         d : page down          w : up           s : down        ", 0xFFFF0000, 0, pos);

	for(textline_index = current_line; textline_index < fv->lines; textline_index++) {
		if ((textline_index-current_line) == 30) {
			break;
		}

		if ((current_line+30) > fv->lines) {
			break;
		}

		if (fileviewer_getline_index(fv, textline_index, &linebuf, &line_number) == 0) {
			memset(line_number_buf, '\0', 100);
			snprintf(line_number_buf, 10, "%d", line_number+1);

			drawText(20, 80 + ((textline_index-current_line) * 16) + 5, line_number_buf, 0xFFFFFF00, 0, pos);
			drawText(50, 80 + ((textline_index-current_line) * 16) + 5, linebuf, 0xFFFF0000, 0, pos);
			if(linebuf) {
				free(linebuf);
				linebuf = NULL;
			}
		}
	}

	if (filename_buf) {
		free(filename_buf);
	}

	if (line_number_buf) {
		free(line_number_buf);
	}

	if (page_number_buf) {
		free(page_number_buf);
	}
}

#define FILE_NAME "font.c"

int main()
{
	int x, y;
	int ret;
	unsigned int *pos;
	int endFlag = 0;
	int ch;
	unsigned int phyLCDAddr = 0;
	FrameBuffer gfb;
	struct fileviewer fv;
	int current_line = 0;

	printf("Font Test Program Start\n");

	ret = fb_open(&gfb);
	if(ret < 0){
		printf("Framebuffer open error");
		perror("");
		return -1;
	}

	// get physical framebuffer address for LCD
	if (ioctl(ret, S3CFB_GET_LCD_ADDR, &phyLCDAddr) == -1)
	{
		printf("%s:ioctl(S3CFB_GET_LCD_ADDR) fail\n", __func__);
		return 0;
	}
	printf("phyLCD:%x\n", phyLCDAddr);

	hdmi_initialize();

	hdmi_gl_initialize(0);
	hdmi_gl_set_param(0, phyLCDAddr, 1280, 720, 0, 0, 0, 0, 1);
	hdmi_gl_streamon(0);

	x = 50;
	y = 100;

	pos = (unsigned int*)gfb.start;

	//Clear Screen(Black)
	memset(pos, 0x00, 1280*720*4);

	// Draw Box

	if (fileviewer_init(&fv, FILE_NAME) == -1) {
		printf("failed to open\n");
		return -1;
	}

	if (fileviewer_run(&fv) == -1) {
    printf("failed to run\n");
    return -1;
  }

	draw_ui(pos, FILE_NAME, current_line, &fv);

	printf("'q' is Quit\n");
	while (!endFlag)
	{
		usleep(10*1000);

		if (kbhit())
		{
			ch = getchar();
			switch ( ch ) {
				case 'q': endFlag = 1;
					break;

				case 'w':
					if (current_line > 0) {
						current_line -= 1;
						printf("Up %d\n", current_line);
						draw_ui(pos, FILE_NAME, current_line, &fv);
					}
					break;

				case 's':
					if (current_line+30 <= fv.lines-1) {
						current_line += 1;
						printf("Down %d\n", current_line);
						draw_ui(pos, FILE_NAME, current_line, &fv);
					}
					break;

				// a : page up         d : page down
				case 'a':
				if (current_line-30 > 0) {
					current_line -= 30;
					draw_ui(pos, FILE_NAME, current_line, &fv);
				} else {
					current_line = 0;
					draw_ui(pos, FILE_NAME, current_line, &fv);
				}
				break;

				case 'd':
				if (current_line + (30 * 2) <= fv.lines-1) {
					current_line += 30;
					draw_ui(pos, FILE_NAME, current_line, &fv);
				} else if (current_line + (30 * 2) > fv.lines){
					current_line = fv.lines-30;
					draw_ui(pos, FILE_NAME, current_line, &fv);
				}
				break;
			}
		}
	}

	hdmi_gl_streamoff(0);
	hdmi_gl_deinitialize(0);
	hdmi_deinitialize();
	fb_close(&gfb);

	return 0;
}
