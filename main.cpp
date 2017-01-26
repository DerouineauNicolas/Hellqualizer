#include <ao/ao.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>
#include <ring_buffer.h>
#include <limits>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <processing.h>
#include <demuxing_decoding.h>

RingBuffer* Buffer_decode_process= new RingBuffer(44100*2);

int log_level=0;
int processing_options=0;

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

int main (int argc, char **argv)
{
    if ( (argc <2)) {
        fprintf(stderr, "Wrong Usage \n");
        exit(1);
    }
    const char *src_filename = argv[1];
    if(argc>3){
    if(!strcmp(argv[2],"-v"))
        log_level= atoi(argv[2]);
    else if(!strcmp(argv[2],"-f"))
        processing_options= atoi(argv[3]);
    }

    /* read frames from the file */

//    pthread_t decoding_thread;
//    pthread_t soundcard_thread;

//    if (pthread_mutex_init(&lock, NULL) != 0)
//    {
//    printf("\n mutex init failed\n");
//    return 1;
//    }

//    if(pthread_create(&decoding_thread, NULL, decode_thread, NULL)) {

//    fprintf(stderr, "Error creating thread\n");
//    return 1;

//    }


//    if(pthread_create(&soundcard_thread, NULL, play_thread, NULL)) {

//    fprintf(stderr, "Error creating thread\n");
//    return 1;

//    }

//    pthread_join(decoding_thread, NULL);

//    pthread_join(soundcard_thread, NULL);


//    /* -- Close and shutdown -- */
//    ao_close(device);

//    ao_shutdown();

//end:
//    avcodec_free_context(&video_dec_ctx);
//    avcodec_free_context(&audio_dec_ctx);
//    avformat_close_input(&fmt_ctx);
//    av_frame_free(&frame);
//    av_free(video_dst_data[0]);

//    return ret < 0;
}
