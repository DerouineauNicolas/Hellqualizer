#define __STDC_CONSTANT_MACROS

extern "C" {
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
}

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

#include <ao/ao.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>
#include <ring_buffer.h>
#include <limits>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include <processing.h>

static AVFormatContext *fmt_ctx = NULL;
static AVCodecContext *video_dec_ctx = NULL, *audio_dec_ctx;
static AVStream *audio_stream = NULL;
static const char *src_filename = NULL;


static int audio_stream_idx = -1;
static uint8_t *video_dst_data[4] = {NULL};
static AVFrame *frame = NULL;
static AVPacket pkt;
static int data_size=0;
static int plane_size=0;

/* Enable or disable frame reference counting. You are not supposed to support
 * both paths in your application but pick the one most appropriate to your
 * needs. Look for the use of refcount in this example to see what are the
 * differences of API usage between them. */
static int refcount = 0;

/*############################LIBABO############*/


static ao_device *device;
static ao_sample_format ao_format;
static int default_driver;

const int buffer_size=AVCODEC_MAX_AUDIO_FRAME_SIZE+ FF_INPUT_BUFFER_PADDING_SIZE;


RingBuffer* Buffer_decode_process= new RingBuffer(44100*2);
//RingBuffer* Buffer_process_play= new RingBuffer(4410*2);

int log_level=0;


#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

pthread_mutex_t lock;




void decode_packet(int *got_frame, int *bytes_read,int cached)
{
    int ret = 0;

    *got_frame = 0;
    int decoded = pkt.size;

    if (pkt.stream_index == audio_stream_idx) {
        /* decode audio frame */

        ret = avcodec_decode_audio4(audio_dec_ctx, frame, got_frame, &pkt);
        if (ret < 0) {
            /*fprintf(stderr, "Error decoding audio frame (%s)\n", av_err2str(ret));*/
            //return ret;
            *bytes_read=-1;
            goto endofdecoding;
        }

        data_size = av_samples_get_buffer_size(&plane_size, audio_dec_ctx->channels,
                                               frame->nb_samples,
                                               audio_dec_ctx->sample_fmt, 1);


        /* Some audio decoders decode only part of the packet, and have to be
         * called again with the remainder of the packet data.
         * Sample: fate-suite/lossless-audio/luckynight-partial.shn
         * Also, some decoders might over-read the packet. */
        decoded = FFMIN(ret, pkt.size);

        uint8_t samples[buffer_size];
        uint16_t *out = (uint16_t *)samples;

        if (*got_frame) {
            if(audio_dec_ctx->sample_fmt==AV_SAMPLE_FMT_FLTP){
                for (int j=0; j<audio_dec_ctx->channels; j++) {
                    float* inputChannel = (float*)frame->extended_data[j];
                    for (int i=0 ; i<frame->nb_samples ; i++) {
                        float sample = inputChannel[i];
                        if (sample<-1.0f) sample=-1.0f;
                        else if (sample>1.0f) sample=1.0f;
                        out[i*audio_dec_ctx->channels + j] = (int16_t) (sample * 32767.0f );
                    }
                }
                pthread_mutex_lock(&lock);
                Buffer_decode_process->Write(samples, 2*audio_dec_ctx->channels*frame->nb_samples);
                pthread_mutex_unlock(&lock);
            }
        }
    }


    if(!(ret<0))
        *bytes_read=ret;
endofdecoding:
    ;

    //return decoded;
}

void *decode_thread(void *x_void_ptr)
{
    int num_bytes=0,got_frame;
    int got_space;
    while (1) {
        pthread_mutex_lock(&lock);
        got_space=Buffer_decode_process->GetWriteAvail();
        pthread_mutex_unlock(&lock);
        //printf("got_space= %d",got_space );
        if(got_space>5000){
            if(av_read_frame(fmt_ctx, &pkt) >= 0){
                AVPacket orig_pkt = pkt;
                do {
                    decode_packet(&got_frame, &num_bytes,0);
                    if (num_bytes < 0)
                        break;
                    pkt.data += num_bytes;
                    pkt.size -= num_bytes;
                } while (pkt.size > 0);
                av_packet_unref(&orig_pkt);}
            else
            {
                break;
            }
        }
        else{
            if(log_level)
                printf("Input Buffer overflow !!! \n");
        }
    }

    /* the function must return something - NULL will do */
    return NULL;

}

int processing_options=0;

void *play_thread(void *x_void_ptr)
{
    //static init_status;
    int read_available=0;
    int num_fail=0;
    static int output_size=2048;
    Processing* processor=new Processing(output_size);

    uint8_t *samples;
    samples=(uint8_t*)malloc(buffer_size*sizeof(uint8_t));

    while(1){
        pthread_mutex_lock(&lock);
        read_available=Buffer_decode_process->GetReadAvail();
        if(read_available>output_size){
            Buffer_decode_process->Read(samples,output_size);
            processor->process(&samples,output_size, processing_options);
            ao_play(device,(char*)samples, output_size);
        }
        pthread_mutex_unlock(&lock);
        if(read_available==0){
            num_fail++;
            //printf("Sleep\n");
            //Let's try again later
            usleep(100000);
            //printf("Awake\n");
        }
        else
            num_fail=0;
        if(num_fail>(15)){
            printf("No more data to play \n");
            break;
        }
    }

    free(samples);

    /* the function must return something - NULL will do */
    return NULL;

}

static int open_codec_context(int *stream_idx,
                              AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    int ret, stream_index;
    AVStream *st;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(type), src_filename);
        return ret;
    } else {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec) {
            fprintf(stderr, "Failed to find %s codec\n",
                    av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }

        /* Allocate a codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx) {
            fprintf(stderr, "Failed to allocate the %s codec context\n",
                    av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(type));
            return ret;
        }


        /* Init the decoders, with or without reference counting */
        av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
        if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;

    }

    return 0;
}



int main (int argc, char **argv)
{
    int ret = 0; //got_frame;


    if ( (argc <2)) {
        fprintf(stderr, "Wrong Usage \n");
        exit(1);
    }
    src_filename = argv[1];
    if(argc>3){
        if(!strcmp(argv[2],"-v"))
            log_level= atoi(argv[2]);
        else if(!strcmp(argv[2],"-f"))
            processing_options= atoi(argv[3]);
    }


    /* register all formats and codecs */
    av_register_all();

    /* open input file, and allocate format context */
    if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", src_filename);
        exit(1);
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        exit(1);
    }


    if (open_codec_context(&audio_stream_idx, &audio_dec_ctx, fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
        audio_stream = fmt_ctx->streams[audio_stream_idx];
    }

    /* dump input information to stderr */
    av_dump_format(fmt_ctx, 0, src_filename, 0);

    if (!audio_stream) {
        fprintf(stderr, "Could not find audio  stream in the input, aborting\n");
        ret = 1;
        goto end;
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    /* initialize packet, set data to NULL, let the demuxer fill it */
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

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
        exit(1);
    }

    /* -- Open driver -- */
    device = ao_open_live(default_driver, &ao_format, NULL /* no options */);
    if (device == NULL) {
        fprintf(stderr, "Error opening device.\n");
        return 1;
    }

    /* read frames from the file */

    pthread_t decoding_thread;
    pthread_t soundcard_thread;

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    if(pthread_create(&decoding_thread, NULL, decode_thread, NULL)) {

        fprintf(stderr, "Error creating thread\n");
        return 1;

    }


    if(pthread_create(&soundcard_thread, NULL, play_thread, NULL)) {

        fprintf(stderr, "Error creating thread\n");
        return 1;

    }

    pthread_join(decoding_thread, NULL);

    pthread_join(soundcard_thread, NULL);


    /* -- Close and shutdown -- */
    ao_close(device);

    ao_shutdown();

end:
    avcodec_free_context(&video_dec_ctx);
    avcodec_free_context(&audio_dec_ctx);
    avformat_close_input(&fmt_ctx);
    av_frame_free(&frame);
    av_free(video_dst_data[0]);

    return ret < 0;
}
