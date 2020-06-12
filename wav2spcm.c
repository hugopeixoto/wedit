#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "utils.h"

int parse_wav_header(uint16_t* size, uint16_t* channels, uint32_t* hz, uint32_t* nmemb) {
  uint8_t buffer[1<<14];

  fread(buffer, 4, 1, stdin); // "RIFF"
  fread(buffer, 4, 1, stdin); // size
  //uint32_t riff_size = leu32(buffer);

  fread(buffer, 4, 1, stdin); // "WAVE"
  if (memcmp(buffer, "WAVE", 4) != 0) { fprintf(stderr, "expected WAVE, got %.4s\n", buffer); return -1; }

  fread(buffer, 4, 1, stdin); // "fmt "
  if (memcmp(buffer, "fmt ", 4) != 0) { fprintf(stderr, "expected 'fmt ', got %.4s\n", buffer); return -1; }

  fread(buffer, 4, 1, stdin); // size=16
  if (leu32(buffer) < 16) { fprintf(stderr, "expected size=16, got %u\n", leu32(buffer)); return -1; }

  fread(buffer, 2, 1, stdin); // audio format
  const uint16_t format = leu16(buffer);

  if (format != 1 && format != 65534) { fprintf(stderr, "expected audiofmt=1 or 65534, got %hu\n", leu16(buffer)); return -1; }

  fread(buffer, 2, 1, stdin); // numchannels
  *channels = leu16(buffer);

  fread(buffer, 4, 1, stdin); // samples per second
  *hz = leu32(buffer);

  fread(buffer, 4, 1, stdin); // bytes per second
  fread(buffer, 2, 1, stdin); // block align
  *size = leu16(buffer);

  fread(buffer, 2, 1, stdin); // bits per sample

  if (format == 65534) {
    fread(buffer, 2, 1, stdin);
    if (leu16(buffer) != 22) { fprintf(stderr, "extension size %hu\n", leu16(buffer)); return -1; }
    fread(buffer, 22, 1, stdin);
  }

  for (;;) {
    fread(buffer, 4, 1, stdin); // "data"
    if (!memcmp(buffer, "data", 4)) {
      break;
    } else {
      fread(buffer, 4, 1, stdin);
      fseek(stdin, leu32(buffer), SEEK_CUR);
    }
  }

  fread(buffer, 4, 1, stdin); // stream size
  *nmemb = leu32(buffer)/(*size);

  return 0;
}

int main(int argc, char* argv[]) {
  struct stream_t stream;
  uint8_t buffer[2 + 2 + 4 + 4];

  if (parse_wav_header(&stream.size, &stream.channels, &stream.hz, &stream.nmemb) != 0) {
    fprintf(stderr, "invalid wav header\n");
    return -1;
  }

  leu16write(stream.size, buffer);
  leu16write(stream.channels, buffer + 2);
  leu32write(stream.hz, buffer + 2 + 2);
  leu32write(stream.nmemb, buffer + 2 + 2 + 4);

  fwrite("SPCM", 1, 4, stdout);
  fwrite(buffer, 1, sizeof(buffer), stdout);

  copy(stream.size*stream.nmemb);
  return 0;
}

