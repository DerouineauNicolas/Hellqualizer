#include <ring_buffer.h>
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#ifdef WIN32
//#include <unistd_win32.h>
#else
#include <unistd.h>
#endif
#include <demuxing_decoding.h>
#include <rendering.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <record.h>
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
    Ctx->is_realtime=0;
}

static void Destroy_Hellqualizer(HQ_Context *Ctx){
    delete(Ctx->Buffer_decode_process);
    pthread_cond_destroy(&Ctx->m_signal_decode_to_process);
    pthread_mutex_destroy(&Ctx->m_mutex_decode_to_process);
}

int main (int argc, char **argv)
{
    HQ_Context Ctx;
    DemuxDecode *decoder;
    RECORDER *recorder;
    char *src_filename=NULL;

    if ( (argc <2)) {
        fprintf(stderr, "Wrong Usage \n");
        exit(EXIT_SUCCESS);
    }

    init_Hellqualizer(&Ctx);

    if(!strcmp(argv[1],"-alsa")){
        Ctx.channels=2;
        Ctx.is_realtime=1;
        Ctx.Sampling_rate=44100;
    }
    else
        src_filename = argv[1];

    if(Ctx.is_realtime)
        recorder=new RECORDER("default",&Ctx);
    else
        decoder =new DemuxDecode(src_filename,&Ctx);

    Rendering *renderer=new Rendering(&Ctx);

#ifdef HQ_GUI
    GUI *gui_control=new GUI(src_filename,&Ctx);
#else
    Controler *control=new Controler(src_filename, &Ctx);
#endif

    if(Ctx.is_realtime)
        recorder->StartInternalThread();
    else
        decoder->StartInternalThread();

    renderer->StartInternalThread();

#ifdef HQ_GUI
    gui_control->StartInternalThread();
#else
    control->StartInternalThread();
#endif

    if(Ctx.is_realtime)
        recorder->WaitForInternalThreadToExit();
    else
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

    Destroy_Hellqualizer(&Ctx);

}
