all: wedit wav2spcm spcm-info

clean:
	rm -f wedit wav2spcm spcm-info

wav2spcm: wav2spcm.c utils.c utils.h
	clang -o wav2spcm wav2spcm.c utils.c

wedit: wedit.c utils.c utils.h
	clang -O3 -o wedit wedit.c utils.c

spcm-info: spcm-info.c utils.c utils.h
	clang -o spcm-info spcm-info.c utils.c
