#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "utils.h"

int parse_spcm(struct stream_t* stream) {
  uint8_t buffer[1<<14];

  fread(buffer, 4 + 2 + 2 + 4 + 4, 1, stdin);

  if (memcmp(buffer, "SPCM", 4) != 0) { fprintf(stderr, "expected SPCM\n"); return -1; }

  stream->size = leu16(buffer+4);
  stream->channels = leu16(buffer+4+2);
  stream->hz = leu32(buffer+4+2+2);
  stream->nmemb = leu32(buffer+4+2+2+4);

  return 0;
}

int main(int argc, char* argv[]) {
  struct stream_t stream;

  if (parse_spcm(&stream) != 0) {
    fprintf(stderr, "invalid SPCM header\n");
    return -1;
  }

  printf(
    "size=%hu\n"
    "channels=%u\n"
    "hz=%u\n"
    "nmemb=%u\n",
    stream.size,
    stream.channels,
    stream.hz,
    stream.nmemb
  );

  return 0;
}
