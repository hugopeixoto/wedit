# Tools to help me edit podcasts (or just any wav file, really)

`pcm-cut` receives a wav from stdin and
a set of time intervals from the arg list.
It removes those time intervals from the wav
and outputs the resulting PCM to stdout.

`pcm2wav` converts a 44.1kHz s16le PCM stream
back to a WAV file (it basically adds the header).
It is currently using ffmpeg.

`pcm-play` plays the 44.1kHz s16le PCM stream,
currently using ffplay.

## My workflow

I edit file containing the time intervals. Whenever I want to test it, I run
the following command:

```bash
pcm-cut $(cat cuts.txt) < unedited.wav | pcm-play
```

If I'm happy with the result, I run the following:

```bash
rm edited.wav; pcm-cut $(cat cuts.txt) < unedited.wav | pcm2wav edited.wav
```
