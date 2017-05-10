#include <rendering.h>
#include <alsa/asoundlib.h>
#include <unistd.h>

static int err;
static short buf[128];
static snd_pcm_t *playback_handle;
snd_pcm_hw_params_t *hwparams;
snd_pcm_sw_params_t *swparams;
static char *device = "plughw:0,0";                     /* playback device */
static snd_pcm_format_t format = SND_PCM_FORMAT_S16;    /* sample format */
static unsigned int rate = 44100;                       /* stream rate */
static unsigned int channels = 2;                       /* count of channels */
static unsigned int buffer_time = 500000;               /* ring buffer length in us */
static unsigned int period_time = 100000;               /* period time in us */
static double freq = 440;                               /* sinusoidal wave frequency in Hz */
static int verbose = 0;                                 /* verbose flag */
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

    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_alloca(&swparams);

    rate=ctx->Sampling_rate;
    channels=ctx->channels;

    if ((err = snd_pcm_open (&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n");
        exit (1);
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

static void generate_sine(const snd_pcm_channel_area_t *areas,
                          snd_pcm_uframes_t offset,
                          int count, double *_phase)
{
        static double max_phase = 2. * M_PI;
        double phase = *_phase;
        double step = max_phase*freq/(double)rate;
        unsigned char *samples_out[channels];
        int steps[channels];
        unsigned int chn;
        int format_bits = snd_pcm_format_width(format);
        unsigned int maxval = (1 << (format_bits - 1)) - 1;
        int bps = format_bits / 8;  /* bytes per sample */
        int phys_bps = snd_pcm_format_physical_width(format) / 8;
        int big_endian = snd_pcm_format_big_endian(format) == 1;
        int to_unsigned = snd_pcm_format_unsigned(format) == 1;
        int is_float = (format == SND_PCM_FORMAT_FLOAT_LE ||
                        format == SND_PCM_FORMAT_FLOAT_BE);
        /* verify and prepare the contents of areas */
        for (chn = 0; chn < channels; chn++) {
                if ((areas[chn].first % 8) != 0) {
                        printf("areas[%i].first == %i, aborting...\n", chn, areas[chn].first);
                        exit(EXIT_FAILURE);
                }
                samples_out[chn] = /*(signed short *)*/(((unsigned char *)areas[chn].addr) + (areas[chn].first / 8));
                if ((areas[chn].step % 16) != 0) {
                        printf("areas[%i].step == %i, aborting...\n", chn, areas[chn].step);
                        exit(EXIT_FAILURE);
                }
                steps[chn] = areas[chn].step / 8;
                samples_out[chn] += offset * steps[chn];
        }
        /* fill the channel areas */
        while (count-- > 0) {
                union {
                        float f;
                        int i;
                } fval;
                int res, i;
                if (is_float) {
                        fval.f = sin(phase);
                        res = fval.i;
                } else
                        res = sin(phase) * maxval;
                if (to_unsigned)
                        res ^= 1U << (format_bits - 1);
                for (chn = 0; chn < channels; chn++) {
                        /* Generate data in native endian format */
                        if (big_endian) {
                                for (i = 0; i < bps; i++)
                                        *(samples_out[chn] + phys_bps - 1 - i) = (res >> i * 8) & 0xff;
                        } else {
                                for (i = 0; i < bps; i++)
                                        *(samples_out[chn] + i) = (res >>  i * 8) & 0xff;
                        }
                        samples_out[chn] += steps[chn];
                }
                phase += step;
                if (phase >= max_phase)
                        phase -= max_phase;
        }
        *_phase = phase;
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
    unsigned int chn;

    uint8_t *samples;
    samples=(uint8_t*)malloc(buffer_size*sizeof(uint8_t));

    signed short *samples_out;
    samples_out = (short int*)malloc((period_size * channels * snd_pcm_format_physical_width(format)) / 8);

    areas = (snd_pcm_channel_area_t*)calloc(channels, sizeof(snd_pcm_channel_area_t));
    if (areas == NULL) {
            printf("No enough memory\n");
            exit(EXIT_FAILURE);
    }
    for (chn = 0; chn < channels; chn++) {
            areas[chn].addr = samples_out;
            areas[chn].first = chn * snd_pcm_format_physical_width(format);
            areas[chn].step = channels * snd_pcm_format_physical_width(format);
    }

    double phase = 0;
    signed short *ptr;
    int err, cptr;

    while(1){
        if(m_ctx->state==PLAY){
#if 1
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

                if ((err = snd_pcm_writei (handle, samples_out, output_size/4)) != (output_size/4)) {
                    fprintf (stderr, "write to audio interface failed (%s)\n",
                             snd_strerror (err));
                    exit (1);
                }
            //}
            //ao_play(device,(char*)samples, output_size);
            pthread_mutex_unlock(m_mutex);
#endif

#if 0
            generate_sine(areas, 0, period_size, &phase);
            ptr = samples_out;
            cptr = period_size;
            while (cptr > 0) {
                    err = snd_pcm_writei(handle, ptr, cptr);
                    for(int i=0;i<cptr;i++)
                        printf("%d \n",*(ptr+i));
//                    if (err == -EAGAIN)
//                            continue;
//                    if (err < 0) {
//                            if (xrun_recovery(handle, err) < 0) {
//                                    printf("Write error: %s\n", snd_strerror(err));
//                                    exit(EXIT_FAILURE);
//                            }
//                            break;  /* skip one period */
//                    }
                    ptr += err * channels;
                    cptr -= err;
            }
#endif
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
