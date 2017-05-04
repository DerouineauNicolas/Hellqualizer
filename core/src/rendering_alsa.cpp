#include <rendering.h>
#include <alsa/asoundlib.h>
#include <unistd.h>

static int err;
static short buf[128];
static snd_pcm_t *playback_handle;
static snd_pcm_hw_params_t *hw_params;

/*############################LIBABO############*/

Rendering::Rendering( HQ_Context *ctx){

    /*
    ao_initialize();

    default_driver = ao_default_driver_id();

    memset(&ao_format, 0, sizeof(ao_format));

    ao_format.bits = 16;
    ao_format.channels = ctx->channels;
    ao_format.rate = ctx->Sampling_rate;//audio_dec_ctx->sample_rate;
    ao_format.byte_format = AO_FMT_NATIVE;
    ao_format.matrix=0;


    device = ao_open_live(default_driver, &ao_format, NULL  no options );
    if (device == NULL) {
        fprintf(stderr, "Error opening device.\n");
    }*/


    if ((err = snd_pcm_open (&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n");
        exit (1);
    }

    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf (stderr, "cannot set access type (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_channels (playback_handle, hw_params, ctx->channels)) < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    snd_pcm_hw_params_free (hw_params);

    if ((err = snd_pcm_prepare (playback_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    m_mutex=&ctx->m_mutex_decode_to_process;
    m_signal=&ctx->m_signal_decode_to_process;
    m_buffer_decode_process=ctx->Buffer_decode_process;
    m_ctx=ctx;

}

Rendering::~Rendering(){
    /*ao_close(device);
    ao_shutdown();*/
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
            for (int i = 0; i < output_size; ++i) {
                if ((err = snd_pcm_writei (playback_handle, samples, 128)) != 128) {
                    fprintf (stderr, "write to audio interface failed (%s)\n",
                             snd_strerror (err));
                    exit (1);
                }
            }
            //ao_play(device,(char*)samples, output_size);
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
