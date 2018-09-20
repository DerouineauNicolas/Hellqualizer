#include <Hellqualizer.h>
#include <stdio.h>

HQ_Context context;

void InitHellqualizer() {
    pthread_cond_init(&context.m_signal_decode_to_process, NULL);
    pthread_mutex_init(&context.m_mutex_decode_to_process, NULL);
    pthread_cond_init(&context.m_signal_process_to_render, NULL);
    pthread_mutex_init(&context.m_mutex_process_to_render, NULL);
    context.Buffer_decode_process = new RingBuffer(44100 * 2);
    context.Buffer_process_render = new RingBuffer(44100 * 2);
    context.proc_opt.do_process = 1;
    context.proc_opt.GAIN[0] = 1.0;
    context.proc_opt.GAIN[1] = 1.0;
    context.proc_opt.GAIN[2] = 1.0;
    context.proc_opt.GAIN[3] = 1.0;
    context.proc_opt.GAIN[4] = 1.0;
    context.state = PLAY;
    context.verbosity = 0;
    context.is_realtime = 0;
}

void SetHellVerbosity(int level) {
    context.verbosity = level;
}

void DestroyHellqualizer(){
    delete(context.Buffer_decode_process);
    delete(context.Buffer_process_render);
    pthread_cond_destroy(&context.m_signal_decode_to_process);
    pthread_mutex_destroy(&context.m_mutex_decode_to_process);
    pthread_cond_destroy(&context.m_signal_process_to_render);
    pthread_mutex_destroy(&context.m_mutex_process_to_render);
}

void HellLOG(int loglevel, char *str){
    if(loglevel>context.verbosity){
        printf("%s \n",str);
    }
}


