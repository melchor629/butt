#ifndef FLAC_ENCODE_H
#define FLAC_ENCODE_H

#include <FLAC/stream_encoder.h>

struct flac_enc {
    FLAC__StreamEncoder  *encoder;
    int bitrate;
    int samplerate;
    int channel;
    volatile int state;
};

int flac_enc_init(flac_enc *flac);
int flac_enc_init_FILE(flac_enc *flac, FILE *fout);
int flac_enc_encode(flac_enc *flac, short *pcm_buf, int samples_per_chan, int channel);
int flac_enc_reinit(flac_enc *flac);
FLAC__uint64 flac_enc_get_bytes_written(void);
void flac_enc_close(flac_enc *flac);


#endif
