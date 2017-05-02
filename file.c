#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"

static long _file_get_size(FILE* fp);
static long _file_apply_contents(struct fileviewer *fv, char *contents);

static int _file_readall(FILE* fp, char **contents) {
  char *temp;

  if (fp == NULL) {
    return -1;
  }

  int file_size = (int)_file_get_size(fp);

  temp = (char*)malloc(sizeof(char) * (file_size + 10));
  *contents = (char*)malloc(sizeof(char) * (file_size + 10));

  if (temp == NULL) {
    return -1;
  }

  printf("malloced : %d\n", file_size + 10);

  if (fread(temp, file_size, 1, fp) < 0) {
    return -1;
  }

  if (fclose(fp) == -1) {
    return -1;
  }

  strncpy(*contents, temp, file_size);
  free(temp);

  return 0;
}

static long _file_get_size(FILE* fp) {
  if (fp == NULL) {
    return -1;
  }

  if (fseek(fp, 0, SEEK_END) == -1) {
    return -1;
  }

  long fsize = ftell(fp);

  if (fsize == -1) {
    fseek(fp, 0, SEEK_SET);
    return -1;
  }

  if (fseek(fp, 0, SEEK_SET) == -1) {
    return -1;
  }

  return fsize;
}

static long countlines(char *path) {
  FILE *fp = fopen(path, "r");
  int ch;
  long count=0;
  do {
      ch = fgetc(fp);
      if(ch == '\n') count++;
  } while( ch != EOF );

  return count;
}

static long _file_apply_contents(struct fileviewer *fv, char *contents) {
  if (fv == NULL) {
    return -1;
  }

  char *token;
  int line_index = 0;

  token = strtok(contents, "\n");

  printf("token %s\n", token);

  if (token == NULL) {
    return -1;
  }

  if (token != NULL) {
    fv->lines = countlines(fv->path);
    //fv->contents = (char**)malloc(sizeof(char**) * fv->lines);
    //fv->contents[line_index] = malloc(sizeof(char*) * (strlen(contents) + 1));
    fv->tv = (struct textviewer*)malloc(sizeof(struct textviewer) * fv->lines);
    fv->tv[line_index].contents = malloc(sizeof(char) * strlen(contents) + 1);
    fv->tv[line_index].line = line_index;
    strcpy(fv->tv[line_index].contents, token);
    line_index += 1;
  }

  token = strtok(NULL, "\n");

  while(token != NULL) {
    fv->tv[line_index].contents = malloc(sizeof(char*) * (strlen(token) + 1));
    strcpy(fv->tv[line_index].contents, token);
    fv->tv[line_index].line = line_index;
    line_index += 1;
    token = strtok(NULL, "\n");
  }

  return 0;
}

int fileviewer_init(struct fileviewer *fv, const char path[]) {
  if (fv == NULL) {
    return -1;
  }

  fv->fp = fopen(path, "r");

  fv->path = (char*)malloc(sizeof(char) * (strlen(path) + 3));
  strcpy(fv->path, path);

  if (fv->fp == NULL) {
    return -1;
  }

  return 0;
}

int fileviewer_getline_index(struct fileviewer *fv, int index, char **buf, int *line_number) {
  *buf = (char*)malloc(sizeof(char) * (strlen(fv->tv[index].contents) + 3));

  if (*buf == NULL) {
    return -1;
  }

  strcpy(*buf, fv->tv[index].contents);
  *line_number = fv->tv[index].line;

  return 0;
}

int fileviewer_run(struct fileviewer *fv) {
  if (fv == NULL) {
    return -1;
  }

  /* must free */
  char *contents;
  char **bak_contents = &contents;

  if(_file_readall(fv->fp, &contents) == -1) {
    printf("Failed to read file\n");
    return -1;
  }

  if (_file_apply_contents(fv, contents) == -1) {
    printf("Failed to apply\n");
    return -1;
  }

  if (*bak_contents) {
    free(*bak_contents);
  }


  return 0;
}
