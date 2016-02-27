# Tools to help me edit podcasts

pcm-cut receives a wav from stdin and
a set of time intervals from the arg list.
It removes those time intervals from the wav
and outputs the resulting PCM to stdout.

pcm2wav converts a 44.1kHz s16le PCM stream
back to a WAV file (it basically adds the header).
It is currently using ffmpeg.

pcm-play plays the 44.1kHz s16le PCM stream,
currently using ffplay.
