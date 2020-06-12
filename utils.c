#include <stdio.h>
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
  uint8_t buffer[BUFSIZ];
  int n;

  while ((n = fread(buffer, 1, BUFSIZ, stdin)) != EOF) {
    fwrite(buffer, n, 1, stdout);
  }
}
