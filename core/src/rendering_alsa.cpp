#include <rendering.h>
#include <ao/ao.h>
#include <unistd.h>

static ao_device *device;
static ao_sample_format ao_format;
static int default_driver;
static const int buffer_size=AVCODEC_MAX_AUDIO_FRAME_SIZE+ FF_INPUT_BUFFER_PADDING_SIZE;

/*############################LIBABO############*/

Rendering::Rendering( HQ_Context *ctx){

    //LIBAO INIT
    ao_initialize();

    /* -- Setup for default driver -- */

    default_driver = ao_default_driver_id();

    memset(&ao_format, 0, sizeof(ao_format));

    ao_format.bits = 16;
    ao_format.channels = ctx->channels;
    ao_format.rate = ctx->Sampling_rate;//audio_dec_ctx->sample_rate;
    ao_format.byte_format = AO_FMT_NATIVE;
    ao_format.matrix=0;

    /* -- Open driver -- */
    device = ao_open_live(default_driver, &ao_format, NULL /* no options */);
    if (device == NULL) {
        fprintf(stderr, "Error opening device.\n");
    }

    m_mutex=&ctx->m_mutex_decode_to_process;
    m_signal=&ctx->m_signal_decode_to_process;
    m_buffer_decode_process=ctx->Buffer_decode_process;
    m_ctx=ctx;

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
        if(m_ctx->state==PLAY){
            pthread_mutex_lock(m_mutex);
            //printf("RENDER: %d \n",m_buffer_decode_process->GetReadAvail());
            while(m_buffer_decode_process->GetReadAvail()<output_size){
                if(m_ctx->state==END_OF_DECODING)
                    break;
                pthread_cond_wait(m_signal, m_mutex);
            }
            if(m_ctx->state==END_OF_DECODING)
                break;
            m_buffer_decode_process->Read(samples,output_size);
            processor->process(&samples,output_size, m_ctx);
            ao_play(device,(char*)samples, output_size);
            pthread_mutex_unlock(m_mutex);
        }
        else{
            usleep(1000000);
        }

    }

    delete processor;

    free(samples);
}

void Rendering::InternalThreadEntry(){
   this->play_thread(NULL);
}
