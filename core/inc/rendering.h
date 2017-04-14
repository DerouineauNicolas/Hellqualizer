#ifndef __PROCESSING_H
#define __PROCESSING_H

extern "C" {
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
}

#include <pthread.h>
#include <ao/ao.h>
#include <ring_buffer.h>
#include <processing.h>
#include <threadclass.h>

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000


class Rendering:public MyThreadClass
{
public:
    Rendering(pthread_mutex_t *m_mutex,pthread_cond_t *m_signal, AVFormatContext *fmt_ctx,AVCodecContext *audio_dec_ctx, RingBuffer *Buffer_decode_process,int *endofdecoding, processing_options *options);
    ~Rendering();
    //void decode_packet(int *got_frame, int *bytes_read,int cached);
    void *play_thread(void *x_void_ptr);
private:
    void InternalThreadEntry();
    pthread_mutex_t *m_mutex;
    pthread_cond_t *m_signal;
    AVFormatContext *fmt_ctx;// = NULL;
    //AVCodecContext *video_dec_ctx; //= NULL, ;
    AVCodecContext *audio_dec_ctx;
    int *m_endofdecoding;
    ao_device *device;
    ao_sample_format ao_format;
    int default_driver;
    const int buffer_size=AVCODEC_MAX_AUDIO_FRAME_SIZE+ FF_INPUT_BUFFER_PADDING_SIZE;
    RingBuffer *m_buffer_decode_process;
    processing_options *m_processing_options;
};

#endif

