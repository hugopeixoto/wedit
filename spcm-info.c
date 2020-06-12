#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "utils.h"

int main(int argc, char* argv[]) {
  struct stream_t stream;

  if (parse_spcm(&stream) != 0) {
    fprintf(stderr, "invalid SPCM header\n");
    return -1;
  }

  printf(
    "size=%hu\n"
    "channels=%u\n"
    "hz=%u\n",
    stream.size,
    stream.channels,
    stream.hz
  );

  return 0;
}
