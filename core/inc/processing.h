#ifndef _PROCESSING_H
#define _PROCESSING_H
#include <Hellqualizer.h>
#include <fir_float.h>
#include <stdint.h>
#include <threadclass.h>


#define NUM_EQ_BANDS 5

class Processing: public MyThreadClass
{
public:
    Processing( HQ_Context *ctx);
    Processing();
    ~Processing();
    void process(uint8_t **samples_in, int size, HQ_Context *options);
private:
    uint16_t *right_ch_in;
    uint16_t *left_ch_in;
    int16_t *right_ch_out;
    int16_t *left_ch_out;
    double *f_right_ch_out;
    double *f_left_ch_out;
    /*These buffers are used as intermediate filtering stage*/
    double *f_left_ch_out_tmp;
    double *f_right_ch_out_tmp;
    /***/
    double *f_right_ch_in;
    double *f_left_ch_in;
    FIR_FLOAT_1Ch *right_FIR;
    FIR_FLOAT_1Ch *left_FIR;
    pthread_mutex_t *m_mutex_decode_process;
    pthread_cond_t *m_signal_decode_process;
    RingBuffer *m_buffer_decode_process;
    pthread_mutex_t *m_mutex_process_render;
    pthread_cond_t *m_signal_process_render;
    RingBuffer *m_buffer_process_render;
    HQ_Context *m_ctx;
    void EQ_stereo_44100(int size,processing_options options);
    void EQ_stereo_48000(int size,processing_options options);
    void InternalThreadEntry();
    void *processing_thread(void *x_void_ptr);
};

#endif



