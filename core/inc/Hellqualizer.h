/*Shared context for hellqualizer program*/

#ifndef _HELLQUALIZER_H
#define _HELLQUALIZER_H

#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <ring_buffer.h>

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

    int Sampling_rate;
    int channels;
    int is_realtime;
    /*TBD
    int sample_rate;*/

    pthread_mutex_t m_mutex_decode_to_process;   //=PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t m_signal_decode_to_process;
    //pthread_cond_init (&m_signal,NULL);
    RingBuffer* Buffer_decode_process;
}HQ_Context;



#endif
