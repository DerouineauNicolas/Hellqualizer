#include <Hellqualizer.h>

void InitHellqualizer(HQ_Context *Ctx){
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

void DestroyHellqualizer(HQ_Context *Ctx){
    delete(Ctx->Buffer_decode_process);
    delete(Ctx->Buffer_process_render);
    pthread_cond_destroy(&Ctx->m_signal_decode_to_process);
    pthread_mutex_destroy(&Ctx->m_mutex_decode_to_process);
    pthread_cond_destroy(&Ctx->m_signal_process_to_render);
    pthread_mutex_destroy(&Ctx->m_mutex_process_to_render);
}


