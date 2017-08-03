#include <rendering.h>
#include <alsa/asoundlib.h>
#include <unistd.h>

static int err;
snd_pcm_hw_params_t *hwparams;
snd_pcm_sw_params_t *swparams;
static snd_pcm_format_t format = SND_PCM_FORMAT_S16;    /* sample format */
static unsigned int rate = 44100;                       /* stream rate */
static unsigned int channels = 2;                       /* count of channels */
static unsigned int buffer_time = 500000;               /* ring buffer length in us */
static unsigned int period_time = 100000;               /* period time in us */
static int resample = 1;                                /* enable alsa-lib resampling */
static int period_event = 0;                            /* produce poll event after each period */
static snd_pcm_sframes_t buffer_size;
static snd_pcm_sframes_t period_size;
static snd_pcm_t *handle;
snd_pcm_channel_area_t *areas;

static int set_hwparams(snd_pcm_t *handle,
                        snd_pcm_hw_params_t *params,
                        snd_pcm_access_t access)
{
        unsigned int rrate;
        snd_pcm_uframes_t size;
        int err, dir;
        /* choose all parameters */
        err = snd_pcm_hw_params_any(handle, params);
        if (err < 0) {
                printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
                return err;
        }
        /* set hardware resampling */
        err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
        if (err < 0) {
                printf("Resampling setup failed for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* set the interleaved read/write format */
        err = snd_pcm_hw_params_set_access(handle, params, access);
        if (err < 0) {
                printf("Access type not available for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* set the sample format */
        err = snd_pcm_hw_params_set_format(handle, params, format);
        if (err < 0) {
                printf("Sample format not available for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* set the count of channels */
        err = snd_pcm_hw_params_set_channels(handle, params, channels);
        if (err < 0) {
                printf("Channels count (%i) not available for playbacks: %s\n", channels, snd_strerror(err));
                return err;
        }
        /* set the stream rate */
        rrate = rate;
        err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
        if (err < 0) {
                printf("Rate %iHz not available for playback: %s\n", rate, snd_strerror(err));
                return err;
        }
        if (rrate != rate) {
                printf("Rate doesn't match (requested %iHz, get %iHz)\n", rate, err);
                return -EINVAL;
        }
        /* set the buffer time */
        err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, &dir);
        if (err < 0) {
                printf("Unable to set buffer time %i for playback: %s\n", buffer_time, snd_strerror(err));
                return err;
        }
        err = snd_pcm_hw_params_get_buffer_size(params, &size);
        if (err < 0) {
                printf("Unable to get buffer size for playback: %s\n", snd_strerror(err));
                return err;
        }
        buffer_size = size;
        /* set the period time */
        err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, &dir);
        if (err < 0) {
                printf("Unable to set period time %i for playback: %s\n", period_time, snd_strerror(err));
                return err;
        }
        err = snd_pcm_hw_params_get_period_size(params, &size, &dir);
        if (err < 0) {
                printf("Unable to get period size for playback: %s\n", snd_strerror(err));
                return err;
        }
        period_size = size;
        /* write the parameters to device */
        err = snd_pcm_hw_params(handle, params);
        if (err < 0) {
                printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
                return err;
        }
        return 0;
}

static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
        int err;
        /* get the current swparams */
        err = snd_pcm_sw_params_current(handle, swparams);
        if (err < 0) {
                printf("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* start the transfer when the buffer is almost full: */
        /* (buffer_size / avail_min) * avail_min */
        err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);
        if (err < 0) {
                printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* allow the transfer when at least period_size samples can be processed */
        /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
        err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_event ? buffer_size : period_size);
        if (err < 0) {
                printf("Unable to set avail min for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* enable period events when requested */
        if (period_event) {
                err = snd_pcm_sw_params_set_period_event(handle, swparams, 1);
                if (err < 0) {
                        printf("Unable to set period event: %s\n", snd_strerror(err));
                        return err;
                }
        }
        /* write the parameters to the playback device */
        err = snd_pcm_sw_params(handle, swparams);
        if (err < 0) {
                printf("Unable to set sw params for playback: %s\n", snd_strerror(err));
                return err;
        }
        return 0;
}

/*############################LIBABO############*/

Rendering::Rendering( HQ_Context *ctx){

    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_alloca(&swparams);

    rate=ctx->Sampling_rate;
    channels=ctx->channels;

    if ((err = snd_pcm_open (&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf (stderr, "cannot open audio device \n");
        exit (EXIT_FAILURE);
    }

    if ((err = set_hwparams(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
            printf("Setting of hwparams failed: %s\n", snd_strerror(err));
            exit(EXIT_FAILURE);
    }

    if ((err = set_swparams(handle, swparams)) < 0) {
            printf("Setting of swparams failed: %s\n", snd_strerror(err));
            exit(EXIT_FAILURE);
    }



    m_mutex=&ctx->m_mutex_decode_to_process;
    m_signal=&ctx->m_signal_decode_to_process;
    m_buffer_decode_process=ctx->Buffer_decode_process;
    m_ctx=ctx;

}


Rendering::~Rendering(){

}

void *Rendering::play_thread(void *x_void_ptr)
{
    static int output_size=2048;
    Processing* processor=new Processing(output_size);

    uint8_t *samples;
    samples=(uint8_t*)malloc(output_size*sizeof(uint8_t));

    signed short *samples_out=NULL;

    int err,cptr;

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

            samples_out=(signed short*)samples;
            //            for(int i=0;i<(output_size/4);i++)
            //               printf("%d \n",*(samples_out+i));
            cptr = (output_size/4);
            while (cptr > 0) {
                err = snd_pcm_writei (handle, samples_out, output_size/4);
                if (err == -EAGAIN)
                        continue;
                else if (err == -EPIPE) {
                      snd_pcm_prepare(handle);
                      continue;
                }
                else if (err < 0) {
                    printf("[ALSA] Write failure: %s\n", snd_strerror(err));
                }
                samples_out += err * channels;
                cptr -= err;
                //exit (1);
            }
            pthread_mutex_unlock(m_mutex);

        }
        else{
            usleep(1000000);
        }

    }

    delete processor;

    return NULL;

    free(samples);
}

void Rendering::InternalThreadEntry(){
    this->play_thread(NULL);
}
