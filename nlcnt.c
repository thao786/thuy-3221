#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

int main (int argc, char *argv[]) {
  char m[] = "mapping";
  char* dlfile = malloc(20*sizeof(char));
  void *handle;
  int (*my_cnt)(char*);

  if (strstr(m, "mapping") != NULL) {
    dlfile = "./mapping.so";
  } else {
    dlfile = "./reading.so";
  }

  handle = dlopen(dlfile, RTLD_LAZY);
  my_cnt = dlsym(handle, "my_cnt");
  printf ("%d \n", (*my_cnt)("2.0"));

  dlclose(handle);
}