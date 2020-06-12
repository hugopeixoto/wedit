#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "utils.h"

const int32_t GAIN_MULTIPLIER = 100;

const int DEBUG = 0;

int parse_spcm(struct stream_t* stream) {
  uint8_t buffer[1<<14];

  fread(buffer, 4 + 2 + 2 + 4 + 4, 1, stdin);

  if (memcmp(buffer, "SPCM", 4) != 0) {
    if (DEBUG) fprintf(stderr, "expected SPCM\n");
    return -1;
  }

  stream->size = leu16(buffer+4);
  stream->channels = leu16(buffer+4+2);
  stream->hz = leu32(buffer+4+2+2);
  stream->nmemb = leu32(buffer+4+2+2+4);

  return 0;
}

struct filter_t {
  uint32_t silence;
  int32_t gain[2];
};

void filter2stderr(struct filter_t* filter) {
  if (DEBUG) fprintf(
    stderr,
    "silence: %8X, gain: %d %d\n",
    filter->silence,
    filter->gain[0],
    filter->gain[1]
  );
}

void filter_sample(struct stream_t* stream, struct filter_t* filter, int32_t* samples) {
  uint32_t bits_per_channel = stream->size * 8 / stream->channels;
  int32_t maximum = (1L << (bits_per_channel - 1)) - 1;
  int32_t minimum = -(1L << (bits_per_channel - 1));

  if (filter->silence & 1) samples[0] = 0;
  if (filter->silence & 2) samples[1] = 0;

  samples[0] = samples[0] * filter->gain[0] / GAIN_MULTIPLIER;
  samples[1] = samples[1] * filter->gain[1] / GAIN_MULTIPLIER;

  if (samples[0] > maximum) { samples[0] = maximum; }
  if (samples[1] > maximum) { samples[1] = maximum; }
  if (samples[0] < minimum) { samples[0] = minimum; }
  if (samples[1] < minimum) { samples[1] = minimum; }
}


int read_sample(struct stream_t* stream, int32_t* samples) {
  static uint8_t buffer[BUFSIZ];
  uint32_t bytes_per_channel = stream->size / stream->channels;

  if (fread(buffer, stream->size, 1, stdin) != 1) {
    return -1;
  }

  if (bytes_per_channel == 2) {
    samples[0] = leu16(buffer);
    if (stream->channels == 2) samples[1] = leu16(buffer + bytes_per_channel);
  }

  if (bytes_per_channel == 3) {
    samples[0] = les24(buffer);
    if (stream->channels == 2) samples[1] = les24(buffer + bytes_per_channel);
  }

  return 0;
}

void write_sample(struct stream_t* stream, const int32_t* samples) {
  static uint8_t buffer[BUFSIZ];

  uint32_t bytes_per_channel = stream->size / stream->channels;

  if (bytes_per_channel == 2) {
    leu16write(samples[0], buffer);
    if (stream->channels == 2) leu24write(samples[1], buffer + bytes_per_channel);
  }

  if (bytes_per_channel == 3) {
    leu24write(samples[0], buffer);
    if (stream->channels == 2) leu24write(samples[1], buffer + bytes_per_channel);
  }

  fwrite(buffer, stream->size, 1, stdout);
}

void copy_filtered(uint32_t nsamples, struct stream_t* stream, struct filter_t* filter) {
  int32_t samples[2];

  for (int i = 0; i < nsamples; i++) {
    if (read_sample(stream, samples) == -1) {
      return;
    }

    filter_sample(stream, filter, samples);
    write_sample(stream, samples);
  }
}

void pipe_filtered(struct stream_t* stream, struct filter_t* filter) {
  int32_t samples[2];

  while (read_sample(stream, samples) == 0) {
    filter_sample(stream, filter, samples);
    write_sample(stream, samples);
  }
}

int next(int argc, char* argv[], struct stream_t* stream, struct filter_t* filter) {
  if (argc < 1)
    return -1;

  if (!strcmp(argv[0], "skip")) {
    if (argc < 2) exit(-1);
    if (DEBUG) fprintf(stderr, "instruction %s %s\n", argv[0], argv[1]);

    uint64_t seconds = atoi(argv[1]);

    skip(seconds * stream->hz * stream->size);

    return 2;
  }

  if (!strcmp(argv[0], "skipms")) {
    if (argc < 2) exit(-1);
    if (DEBUG) fprintf(stderr, "instruction %s %s\n", argv[0], argv[1]);

    uint64_t ms = atoi(argv[1]);

    skip(ms * stream->hz / 1000 * stream->size);

    return 2;
  }

  if (!strcmp(argv[0], "only")) {
    if (argc < 2) exit(-1);
    if (DEBUG) fprintf(stderr, "instruction %s %s\n", argv[0], argv[1]);

    uint32_t channel = atoi(argv[1]);

    filter->silence = ~(1 << channel);

    filter2stderr(filter);

    return 2;
  }

  if (!strcmp(argv[0], "soundall")) {
    if (DEBUG) fprintf(stderr, "instruction %s\n", argv[0]);

    filter->silence = 0;
    filter2stderr(filter);

    return 1;
  }

  if (!strcmp(argv[0], "copy")) {
    if (argc < 2) exit(-1);
    if (DEBUG) fprintf(stderr, "instruction %s %s\n", argv[0], argv[1]);

    uint32_t seconds = atoi(argv[1]);

    copy_filtered(seconds * stream->hz, stream, filter);

    return 2;
  }

  if (!strcmp(argv[0], "copyms")) {
    if (argc < 2) exit(-1);
    if (DEBUG) fprintf(stderr, "instruction %s %s\n", argv[0], argv[1]);

    uint64_t ms = atoi(argv[1]);

    copy_filtered(ms * stream->hz / 1000, stream, filter);

    return 2;
  }

  if (!strcmp(argv[0], "gain")) {
    if (argc < 3) exit(-1);
    if (DEBUG) fprintf(stderr, "instruction %s %s %s\n", argv[0], argv[1], argv[2]);

    uint32_t channel = atoi(argv[1]);
    uint32_t gain = atoi(argv[2]);

    filter->gain[channel] = gain;

    filter2stderr(filter);

    return 3;
  }

  if (!strcmp(argv[0], "exit")) {
    if (DEBUG) fprintf(stderr, "instruction %s\n", argv[0]);
    exit(0);
  }

  exit(-1);
}

int main(int argc, char* argv[]) {
  struct stream_t stream;
  struct filter_t filter = { 0, { GAIN_MULTIPLIER, GAIN_MULTIPLIER } };

  uint32_t start, finish;
  uint32_t pos = 0;

  if (parse_spcm(&stream) != 0) {
    fprintf(stderr, "invalid SPCM header\n");
    return -1;
  }

  if (DEBUG) fprintf(stderr, "gain [%u %u]\n", filter.gain[0], filter.gain[1]);

  if (DEBUG) fprintf(
    stderr,
    "size: %hu, channels: %u, hz: %u, nmemb: %u\n",
    stream.size,
    stream.channels,
    stream.hz,
    stream.nmemb
  );

  uint8_t buffer[2 + 2 + 4 + 4];

  leu16write(stream.size, buffer);
  leu16write(stream.channels, buffer + 2);
  leu32write(stream.hz, buffer + 2 + 2);
  leu32write(stream.nmemb, buffer + 2 + 2 + 4);

  fwrite("SPCM", 1, 4, stdout);
  fwrite(buffer, 1, sizeof(buffer), stdout);

  argc--;
  argv++;

  for (;;) {
    int skips = next(argc, argv, &stream, &filter);
    if (skips < 0) break;

    argc -= skips;
    argv += skips;
  }

  pipe_filtered(&stream, &filter);
  return 0;
}
