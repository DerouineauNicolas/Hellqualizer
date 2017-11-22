#include <ring_buffer.h>
#include <pthread.h>
#include <unistd.h>
#include <demuxing_decoding.h>
#include <rendering.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <record.h>
#include <Hellqualizer.h>
#include <processing.h>


#ifdef HQ_GUI
#include <gui.h>
#else
#include <controler.h>
#endif

static void init_Hellqualizer(HQ_Context *Ctx){
    pthread_cond_init (&Ctx->m_signal_decode_to_process,NULL);
    pthread_mutex_init(&Ctx->m_mutex_decode_to_process,NULL);
    pthread_cond_init (&Ctx->m_signal_process_to_render,NULL);
    pthread_mutex_init(&Ctx->m_mutex_process_to_render,NULL);

    Ctx->Buffer_decode_process=new RingBuffer(44100*2);
    Ctx->Buffer_process_render=new RingBuffer(44100*2);
    Ctx->proc_opt.do_process=1;
    Ctx->proc_opt.GAIN[0]=1.0;
    Ctx->proc_opt.GAIN[1]=1.0;
    Ctx->proc_opt.GAIN[2]=1.0;
    Ctx->proc_opt.GAIN[3]=1.0;
    Ctx->proc_opt.GAIN[4]=1.0;
    Ctx->state=PLAY;
    Ctx->is_realtime=0;
}

static void Destroy_Hellqualizer(HQ_Context *Ctx){
    delete(Ctx->Buffer_decode_process);
    delete(Ctx->Buffer_process_render);
    pthread_cond_destroy(&Ctx->m_signal_decode_to_process);
    pthread_mutex_destroy(&Ctx->m_mutex_decode_to_process);
    pthread_cond_destroy(&Ctx->m_signal_process_to_render);
    pthread_mutex_destroy(&Ctx->m_mutex_process_to_render);
}

int main (int argc, char **argv)
{
    HQ_Context Ctx;
    DemuxDecode *decoder;
    char *src_filename=NULL;

    if ( (argc <2)) {
        fprintf(stderr, "Wrong Usage \n");
        exit(EXIT_SUCCESS);
    }

    init_Hellqualizer(&Ctx);

    src_filename = argv[1];


    decoder =new DemuxDecode(src_filename,&Ctx);

    Rendering *renderer=new Rendering(&Ctx);
    Processing *processor=new Processing(&Ctx);

#ifdef HQ_GUI
    GUI *gui_control=new GUI(src_filename,&Ctx);
#else
    Controler *control=new Controler(src_filename, &Ctx);
#endif

    decoder->StartInternalThread();
    renderer->StartInternalThread();
    processor->StartInternalThread();

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
    delete processor;
    
#ifdef HQ_GUI
    delete gui_control;
#else
    delete control;
#endif

    Destroy_Hellqualizer(&Ctx);

}
