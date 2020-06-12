#ifndef _WEDIT_UTILS_
#define _WEDIT_UTILS_

#include <stdint.h>

struct stream_t {
  uint16_t size;
  uint16_t channels;
  uint32_t hz;
};

void leu16write(uint16_t n, uint8_t* b);
void leu32write(uint32_t n, uint8_t* b);
void leu24write(uint32_t n, uint8_t* b);

uint16_t leu16(const uint8_t* b);
uint32_t leu24(const uint8_t* b);
int32_t les24(const uint8_t* b);
uint32_t leu32(const uint8_t* b);

void pipe();
void copy(uint32_t bytes);
void skip(uint32_t bytes);

int parse_spcm(struct stream_t* stream);
int write_spcm(const struct stream_t* stream);

#endif
