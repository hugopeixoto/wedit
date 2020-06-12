# Tools to help me edit podcasts (or just any wav file, really)

This repository contains some command line tools to help me manipulate audio
files. The main tool is `wedit`, which works on an `SPCM` format.

The `SPCM` format is a `PCM` stream prepended with a header that has
information on the number of channels, frequency, and sample format (`s16le`,
`s24le`, etc).

This format only works with signed little endian formats for now. The only
variable is the number of bits per sample.


## `wav2spcm`

Converts a `wav` file, read from stdin, to a `spcm` file, written to stdout.


## `wedit`

Takes a `spcm` stream from stdin, an arbitrary set of commands from the
argument list, and spits out an edited version to stdout.

This tool supports the following commands:

* `skip <seconds>`: skips samples for the given number of seconds
* `skipms <milliseconds>`: same as `skip`, but in milliseconds
* `only <channel>`: mutes all channels except the given one, from now on
* `soundall`: unmutes every channel
* `copy <seconds>`: outputs samples for the given number of seconds
* `copyms <milliseconds>`: same as `copy`, but in milliseconds
* `gain <channel> <gain>`: applies a gain multiplier to a channel
* `exit`: stops processing the stream and exits


Example:

```bash
cat original.spcm |
  wedit skip 2 copy 1 gain 0 200 copyms 500 exit \
  > edited.spcm
```

## `spcm-info`

Prints the header information of a `spcm` file, read from stdin.

## `spcm2wav`

Converts an `spcm` file, read from stdin, to a `wav` file, written to stdout.
Depends on `ffmpeg` and `spcm-info`.

# My workflow

I edit a file named `edit.txt` and run a bash script to see how it sounds.

The bash script plays the last few seconds of edited audio and the following
seconds of unedited audio so I can see how it sounds.

~~~~bash
#!/usr/bin/env bash

INSTRUCTIONS="$(cat edit.txt)"

OFFSET="1000"

EDITED_DURATION="$(
  echo "$INSTRUCTIONS" |
    tr '\n' ' ' |
    sed -e 's/\(gain\|only\|soundall\|skip\|skipms\|copy\|copyms\)/\n\1/g' |
    grep 'copy' |
    awk '/copyms /{ a += $2 }/copy /{a += $2 * 1000 } END{print a}'
)"

PROGRESS_DURATION="$(
  echo "$INSTRUCTIONS" |
    tr '\n' ' ' |
    sed -e 's/\(gain\|only\|soundall\|skip\|skipms\|copy\|copyms\)/\n\1/g' |
    grep -e 'copy\|skip' |
    awk '/copyms /{ a += $2 }
         /copy /{a += $2 * 1000 }
         /skipms /{ a += $2 }
         /skip /{a += $2 * 1000 }
         END{print a}'
)"

echo edited: "$(( $EDITED_DURATION / 1000 / 60 ))"
echo progress: "$(( $PROGRESS_DURATION / 1000 / 60 ))"

wav2spcm < original.wav |
  wedit $INSTRUCTIONS |
  wedit skipms "$(( $EDITED_DURATION - $OFFSET ))" |
  spcm2wav |
  ffplay -
~~~~

When I'm happy with the results, I can run this to get the final wav file:

~~~~bash
wav2spcm < original.wav | wedit $(cat edit.txt) | spcm2wav > edited.wav
~~~~
