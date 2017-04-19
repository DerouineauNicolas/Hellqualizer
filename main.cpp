#include <math.h>
#include <stdint.h>
#include <ring_buffer.h>
#include <pthread.h>
#include <unistd.h>
#include <processing.h>
#include <demuxing_decoding.h>
#include <rendering.h>
#include <string.h>
#include <controler.h>
#include <gui.h>

int main (int argc, char **argv)
{
    processing_options proc_options;
    int log_level=0;
    //memset(&proc_options,0,sizeof(processing_options));
    proc_options.do_process=1;
    proc_options.GAIN[0]=0.5;
    proc_options.GAIN[1]=0.5;
    proc_options.GAIN[2]=0.5;
    proc_options.GAIN[3]=0.5;
    proc_options.GAIN[4]=0.5;

    if ( (argc <2)) {
        fprintf(stderr, "Wrong Usage \n");
        exit(1);
    }
    char *src_filename = argv[1];

    for(int i=0;i<argc;i++){
        if(!strcmp(argv[i],"-v"))
            log_level= 1;
        else if(!strcmp(argv[i],"-f")){
            proc_options.do_process=1;
            if(argv[i+1]){
                if (strchr(argv[i+1], ':')){
                    sscanf(argv[i+1], "%lf:%lf:%lf:%lf:%lf", &proc_options.GAIN[0], &proc_options.GAIN[1], &proc_options.GAIN[2], &proc_options.GAIN[3],&proc_options.GAIN[4]);
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
                                      decoder->GetAVCtx(),Buffer_decode_process,&EndOfDecoding,&proc_options);
    //Controler *control=new Controler(src_filename,&EndOfDecoding, &proc_options);
    GUI *gui_control=new GUI(src_filename,&EndOfDecoding, &proc_options);

    decoder->StartInternalThread();
    renderer->StartInternalThread();
    //control->StartInternalThread();
    gui_control->StartInternalThread();

    decoder->WaitForInternalThreadToExit();
    renderer->WaitForInternalThreadToExit();
    gui_control->WaitForInternalThreadToExit();
    //control->WaitForInternalThreadToExit();


    delete decoder;
    delete renderer;
    //delete control;
    delete gui_control;
    delete Buffer_decode_process;
}
