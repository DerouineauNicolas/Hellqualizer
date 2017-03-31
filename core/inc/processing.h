#ifndef _PROCESSING
#define _PROCESSING

#include <fir_float.h>

#define NUM_EQ_BANDS 5

typedef struct processing_options{
    double GAIN[5];
    int do_process;
    int verbosity;
}processing_options;

class Processing
{
public:
    Processing( int sample_size);
    ~Processing();
    void process(uint8_t **samples_in, int size, processing_options options);
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
};

#endif



