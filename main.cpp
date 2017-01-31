#include <math.h>
#include <stdint.h>
#include <ring_buffer.h>
#include <pthread.h>
#include <unistd.h>
#include <processing.h>
#include <demuxing_decoding.h>
#include <rendering.h>
#include <string.h>

int log_level=0;


#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

int main (int argc, char **argv)
{
    processing_options options;
    memset(&options,0,sizeof(processing_options));

    if ( (argc <2)) {
        fprintf(stderr, "Wrong Usage \n");
        exit(1);
    }
    const char *src_filename = argv[1];
    if(argc>3){
    if(!strcmp(argv[2],"-v"))
        log_level= atoi(argv[2]);
    else if(!strcmp(argv[2],"-f"))
         options.do_process=1;
         if (strchr(argv[3], ':')){
             sscanf(argv[3], "%lf:%lf:%lf:%lf:%lf", &options.GAIN[0], &options.GAIN[1], &options.GAIN[2], &options.GAIN[3],&options.GAIN[4]);
         }else
         {
             printf("Incorrectly formated filtering options \n");
             exit(-1);
         }
    }

    pthread_mutex_t m_mutex=PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t m_signal;
    pthread_cond_init (&m_signal,NULL);
    RingBuffer* Buffer_decode_process= new RingBuffer(44100*2);
    int EndOfDecoding=0;

    DemuxDecode *decoder=new DemuxDecode(src_filename,&m_mutex,&m_signal,Buffer_decode_process,&EndOfDecoding);
    Rendering *renderer=new Rendering(&m_mutex,&m_signal,decoder->GetFormatCtx(),
                                      decoder->GetAVCtx(),Buffer_decode_process,&EndOfDecoding,options);

    decoder->StartInternalThread();
    renderer->StartInternalThread();
    decoder->WaitForInternalThreadToExit();
    renderer->WaitForInternalThreadToExit();


    delete decoder;
    delete renderer;
    delete Buffer_decode_process;
}
