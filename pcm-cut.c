#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

uint32_t leu32(const uint8_t* b) {
  return (b[0] << 0) | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
}

uint16_t leu16(const uint8_t* b) {
  return (b[0] << 0) | (b[1] << 8);
}

void skip(uint32_t bytes) {
  uint8_t buffer[BUFSIZ];

  while (bytes > 0) {
    uint32_t n = BUFSIZ < bytes ? BUFSIZ : bytes;
    fread(buffer, n, 1, stdin);
    bytes -= n;
  }
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

int parse_wav_header(uint16_t* size, uint32_t* hz, uint32_t* nmemb) {
  uint8_t buffer[1<<14];

  fread(buffer, 4, 1, stdin); // "RIFF"
  fread(buffer, 4, 1, stdin); // size
  //uint32_t riff_size = leu32(buffer);

  fread(buffer, 4, 1, stdin); // "WAVE"
  fread(buffer, 4, 1, stdin); // "fmt "

  fread(buffer, 4, 1, stdin); // size=16
  if (leu32(buffer) != 16) return -1;

  fread(buffer, 2, 1, stdin); // audio format
  if (leu16(buffer) != 1) return -1;

  fread(buffer, 2, 1, stdin); // numchannels
  fread(buffer, 4, 1, stdin); // samples per second
  *hz = leu32(buffer);

  fread(buffer, 4, 1, stdin); // bytes per second
  fread(buffer, 2, 1, stdin); // block align
  *size = leu16(buffer);

  fread(buffer, 2, 1, stdin); // bits per sample
  for (;;) {
    fread(buffer, 4, 1, stdin); // "data"
    if (!memcmp(buffer, "data", 4)) {
      break;
    } else {
      fread(buffer, 4, 1, stdin);
      skip(leu32(buffer));
    }
  }

  fread(buffer, 4, 1, stdin); // stream size
  *nmemb = leu32(buffer)/(*size);

  return 0;
}

int next(int argc, char* argv[], uint32_t hz, uint32_t* start, uint32_t* finish) {
  if (argc < 2)
    return 0;

  *start = (uint32_t)(atof(argv[0]) * hz);
  *finish = (uint32_t)(atof(argv[1]) * hz);

  return 1;
}

int main(int argc, char* argv[]) {
  uint16_t size;
  uint32_t hz;
  uint32_t nmemb;
  uint32_t start, finish;
  uint32_t pos = 0;

  if (parse_wav_header(&size, &hz, &nmemb) != 0) {
    return -1;
  }

  argc--;
  argv++;

  while (next(argc, argv, hz, &start, &finish)) {
    copy(size*(start-pos));
    skip(size*(finish-start));
    pos = finish;

    argc -= 2;
    argv += 2;
  }

  copy(size*(nmemb-pos));
  return 0;
}
