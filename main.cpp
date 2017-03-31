#include <math.h>
#include <stdint.h>
#include <ring_buffer.h>
#include <pthread.h>
#include <unistd.h>
#include <processing.h>
#include <demuxing_decoding.h>
#include <rendering.h>
#include <string.h>

int main (int argc, char **argv)
{
    processing_options options;
    int log_level=0;
    memset(&options,0,sizeof(processing_options));

    if ( (argc <2)) {
        fprintf(stderr, "Wrong Usage \n");
        exit(1);
    }
    const char *src_filename = argv[1];

    for(int i=0;i<argc;i++){
        if(!strcmp(argv[i],"-v"))
            log_level= 1;
        else if(!strcmp(argv[i],"-f")){
            options.do_process=1;
            if(argv[i+1]){
                if (strchr(argv[i+1], ':')){
                    sscanf(argv[i+1], "%lf:%lf:%lf:%lf:%lf", &options.GAIN[0], &options.GAIN[1], &options.GAIN[2], &options.GAIN[3],&options.GAIN[4]);
                }else
                {
                    printf("Incorrectly formated filtering options \n");
                    exit(-1);
                }
            }
            else
            {
                printf("Incorrectly formated filtering options \n");
                exit(-1);
            }
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
