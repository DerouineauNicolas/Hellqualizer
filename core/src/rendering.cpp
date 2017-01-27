#include <rendering.h>

/*############################LIBABO############*/

Rendering::Rendering(pthread_mutex_t *mutex,pthread_cond_t *signal, AVFormatContext *fmt_ctx,AVCodecContext *audio_dec_ctx,
                          RingBuffer *Buffer_decode_process, int *endofdecoding, int processing_options
                     ){

    //LIBAO INIT
    ao_initialize();

    /* -- Setup for default driver -- */

    default_driver = ao_default_driver_id();

    memset(&ao_format, 0, sizeof(ao_format));
    if(audio_dec_ctx->sample_fmt==AV_SAMPLE_FMT_FLT || audio_dec_ctx->sample_fmt==AV_SAMPLE_FMT_FLTP) {
    ao_format.bits = 16;
    ao_format.channels = audio_dec_ctx->channels;
    ao_format.rate = audio_dec_ctx->sample_rate;
    ao_format.byte_format = AO_FMT_NATIVE;
    ao_format.matrix=0;
    }
    else
    {
    //exit(1);
        printf("Format not detected \n");
    }

    /* -- Open driver -- */
    device = ao_open_live(default_driver, &ao_format, NULL /* no options */);
    if (device == NULL) {
        fprintf(stderr, "Error opening device.\n");
    }

    m_mutex=mutex;
    m_signal=signal;
    m_endofdecoding=endofdecoding;
    m_buffer_decode_process=Buffer_decode_process;
    m_processing_options=processing_options;
}

Rendering::~Rendering(){
        ao_close(device);
        ao_shutdown();
}

void *Rendering::play_thread(void *x_void_ptr)
{
    static int output_size=2048;
    Processing* processor=new Processing(output_size);
    const int buffer_size=AVCODEC_MAX_AUDIO_FRAME_SIZE+ FF_INPUT_BUFFER_PADDING_SIZE;

    uint8_t *samples;
    samples=(uint8_t*)malloc(buffer_size*sizeof(uint8_t));

    while(1){
        pthread_mutex_lock(m_mutex);
            while(m_buffer_decode_process->GetReadAvail()<output_size){
                if(*m_endofdecoding)
                break;
                pthread_cond_wait(m_signal, m_mutex);
            }
            if(*m_endofdecoding)
                break;
            m_buffer_decode_process->Read(samples,output_size);
            processor->process(&samples,output_size, m_processing_options);
            ao_play(device,(char*)samples, output_size);
        pthread_mutex_unlock(m_mutex);
    }

    delete processor;

    free(samples);
}

void Rendering::InternalThreadEntry(){
   this->play_thread(NULL);
}
