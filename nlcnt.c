#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

int main (int argc, char *argv[]) {
  char* whichLibrary = malloc(200*sizeof(char));
  char* file = malloc(200*sizeof(char));
  char* dlfile = malloc(20*sizeof(char));
  void *handle;
  int (*my_cnt)(char*);

  if (argc < 3) {
    perror("2 arguments required. Exit.");
    exit(1);
  }

  whichLibrary = argv[1];
  file = argv[2];

  if (strstr(whichLibrary, "mapping") != NULL) {
    dlfile = "./mapping.so";
  } 
  else if (strstr(whichLibrary, "reading") != NULL) {
    dlfile = "./reading.so";
  } else {
    perror("no library specified. Exit.");
    exit(1);
  }

  handle = dlopen(dlfile, RTLD_LAZY);
  my_cnt = dlsym(handle, "my_cnt");
  printf ("%d \n", (*my_cnt)(file));

  dlclose(handle);
}
