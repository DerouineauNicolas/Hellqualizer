/*Shared context for hellqualizer program*/

#ifndef _HELLQUALIZER_H
#define _HELLQUALIZER_H

typedef struct processing_options{
    double GAIN[5];
    int do_process;
}processing_options;

enum Player_state{
   END_OF_DECODING,
   PLAY,
   PAUSE
};

typedef struct HQ_Context{
    Player_state state;
    processing_options proc_opt;
    int verbosity;
    /*TBD
    int num_channels;
    int channels;
    int sample_rate;*/
}HQ_Context;

#endif
