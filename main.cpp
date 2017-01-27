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
#include <rendering.h>



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

    pthread_mutex_t m_mutex=PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t m_signal;
    RingBuffer* Buffer_decode_process= new RingBuffer(44100*2);
    int EndOfDecoding=0;

    DemuxDecode *decoder=new DemuxDecode(src_filename,&m_mutex,&m_signal,Buffer_decode_process,&EndOfDecoding);
    Rendering *renderer=new Rendering(&m_mutex,&m_signal,decoder->GetFormatCtx(),decoder->GetAVCtx(),Buffer_decode_process,&EndOfDecoding);


    decoder->StartInternalThread();
    renderer->StartInternalThread();
    decoder->WaitForInternalThreadToExit();
    renderer->WaitForInternalThreadToExit();

    delete decoder;
    delete renderer;


//end:
//    avcodec_free_context(&video_dec_ctx);
//    avcodec_free_context(&audio_dec_ctx);
//    avformat_close_input(&fmt_ctx);
//    av_frame_free(&frame);
//    av_free(video_dst_data[0]);

//    return ret < 0;
}
