#include <ring_buffer.h>
#include <pthread.h>
#include <unistd.h>
#include <demuxing_decoding.h>
#include <rendering.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <Hellqualizer.h>


#ifdef HQ_GUI
#include <gui.h>
#else
#include <controler.h>
#endif

int main (int argc, char **argv)
{

    HQ_Context Ctx;

    Ctx.proc_opt.do_process=1;
    Ctx.proc_opt.GAIN[0]=0.5;
    Ctx.proc_opt.GAIN[1]=0.5;
    Ctx.proc_opt.GAIN[2]=0.5;
    Ctx.proc_opt.GAIN[3]=0.5;
    Ctx.proc_opt.GAIN[4]=0.5;
    Ctx.state=PAUSE;

    if ( (argc <2)) {
        fprintf(stderr, "Wrong Usage \n");
        exit(1);
    }
    char *src_filename = argv[1];

    pthread_mutex_t m_mutex=PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t m_signal;
    pthread_cond_init (&m_signal,NULL);
    RingBuffer* Buffer_decode_process= new RingBuffer(44100*2);
    int EndOfDecoding=0;

    DemuxDecode *decoder=new DemuxDecode(src_filename,&m_mutex,&m_signal,Buffer_decode_process,&Ctx);
    Rendering *renderer=new Rendering(&m_mutex,&m_signal,decoder->GetFormatCtx(),
                                      decoder->GetAVCtx(),Buffer_decode_process,&Ctx);


#ifdef HQ_GUI
    GUI *gui_control=new GUI(src_filename,&EndOfDecoding, &proc_options);
#else
    Controler *control=new Controler(src_filename,&EndOfDecoding, &Ctx);
#endif

    decoder->StartInternalThread();
    renderer->StartInternalThread();

#ifdef HQ_GUI
    gui_control->StartInternalThread();
#else
    control->StartInternalThread();
#endif

    decoder->WaitForInternalThreadToExit();
    renderer->WaitForInternalThreadToExit();

#ifdef HQ_GUI
    gui_control->WaitForInternalThreadToExit();
#else
    control->WaitForInternalThreadToExit();
#endif


    delete decoder;
    delete renderer;
#ifdef HQ_GUI
    delete gui_control;
#else
    delete control;
#endif

    delete Buffer_decode_process;
}
