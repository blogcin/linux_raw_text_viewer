#include <stdio.h>
#include "file.h"

int main() {
  struct fileviewer fv;

  if (fileviewer_init(&fv, "./test.txt") == -1) {
    printf("failed to open\n");
    return -1;
  }

  if (fileviewer_run(&fv) == -1) {
    printf("failed to run\n");
    return -1;
  }

	int textline_index = 0;
	char *linebuf;
	if (fileviewer_getline_index(&fv, textline_index, &linebuf) == 0) {

			printf("line buf : %s\n", linebuf);
			if(linebuf) {
				free(linebuf);
				linebuf = NULL;
			}
		}

}
