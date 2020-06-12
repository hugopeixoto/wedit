#include <stdio.h>
#include <string.h>
#include "utils.h"

void leu16write(uint16_t n, uint8_t* b) {
  b[0] = (n >> 0) & 0xFF;
  b[1] = (n >> 8) & 0xFF;
}

void leu24write(uint32_t n, uint8_t* b) {
  b[0] = (n >> 0) & 0xFF;
  b[1] = (n >> 8) & 0xFF;
  b[2] = (n >> 16) & 0xFF;
}

void leu32write(uint32_t n, uint8_t* b) {
  b[0] = (n >> 0) & 0xFF;
  b[1] = (n >> 8) & 0xFF;
  b[2] = (n >> 16) & 0xFF;
  b[3] = (n >> 24) & 0xFF;
}

uint32_t leu32(const uint8_t* b) {
  return (b[0] << 0) | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
}

uint32_t leu24(const uint8_t* b) {
  return (b[0] << 0) | (b[1] << 8) | (b[2] << 16);
}

int32_t les24(const uint8_t* b) {
  return ((b[0] << 8) | (b[1] << 16) | (b[2] << 24)) >> 8;
}

uint16_t leu16(const uint8_t* b) {
  return (b[0] << 0) | (b[1] << 8);
}

void copy(uint32_t bytes) {
  uint8_t buffer[BUFSIZ];

  while (bytes > 0) {
    uint32_t n = BUFSIZ < bytes ? BUFSIZ : bytes;
    fread(buffer, n, 1, stdin);
    fwrite(buffer, n, 1, stdout);
    bytes -= n;
  }
}

void skip(uint32_t bytes) {
  uint8_t buffer[BUFSIZ];

  while (bytes > 0) {
    uint32_t n = BUFSIZ < bytes ? BUFSIZ : bytes;
    fread(buffer, n, 1, stdin);
    bytes -= n;
  }
}

void pipe() {
  uint8_t buffer[1<<20];
  int n;

  while ((n = fread(buffer, 1, sizeof(buffer), stdin)) != 0) {
    fwrite(buffer, n, 1, stdout);
  }
}

int parse_spcm(struct stream_t* stream) {
  uint8_t buffer[1<<14];

  if (fread(buffer, 12, 1, stdin) != 1) {
    fprintf(stderr, "unable to read SPCM header\n");
    return -1;
  }

  if (memcmp(buffer, "SPCM", 4) != 0) {
    fprintf(stderr, "expected SPCM magic number\n");
    return -1;
  }

  stream->size = leu16(buffer + 4);
  stream->channels = leu16(buffer + 4 + 2);
  stream->hz = leu32(buffer + 4 + 2 + 2);

  return 0;
}

int write_spcm(const struct stream_t* stream) {
  uint8_t buffer[12];

  memcpy(buffer, "SPCM", 4);

  leu16write(stream->size, buffer + 4);
  leu16write(stream->channels, buffer + 4 + 2);
  leu32write(stream->hz, buffer + 4 + 2 + 2);

  fwrite(buffer, 1, sizeof(buffer), stdout);

  return 0;
}
