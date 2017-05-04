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

static void init_Hellqualizer(HQ_Context *Ctx){
    pthread_cond_init (&Ctx->m_signal_decode_to_process,NULL);
    pthread_mutex_init(&Ctx->m_mutex_decode_to_process,NULL);
    Ctx->Buffer_decode_process=new RingBuffer(44100*2);
    Ctx->proc_opt.do_process=1;
    Ctx->proc_opt.GAIN[0]=1.0;
    Ctx->proc_opt.GAIN[1]=1.0;
    Ctx->proc_opt.GAIN[2]=1.0;
    Ctx->proc_opt.GAIN[3]=1.0;
    Ctx->proc_opt.GAIN[4]=1.0;
    Ctx->state=PLAY;
}

static void Destroy_Hellqualizer(HQ_Context *Ctx){
    delete(Ctx->Buffer_decode_process);
}

int main (int argc, char **argv)
{
    HQ_Context Ctx;



    if ( (argc <2)) {
        fprintf(stderr, "Wrong Usage \n");
        exit(1);
    }
    char *src_filename = argv[1];

    init_Hellqualizer(&Ctx);

    DemuxDecode *decoder=new DemuxDecode(src_filename,&Ctx);
    Rendering *renderer=new Rendering(&Ctx);


#ifdef HQ_GUI
    GUI *gui_control=new GUI(src_filename,&EndOfDecoding, &proc_options);
#else
    Controler *control=new Controler(src_filename, &Ctx);
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

}
