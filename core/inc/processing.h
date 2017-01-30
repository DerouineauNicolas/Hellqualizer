#ifndef _PROCESSING
#define _PROCESSING

//////////////////////////////////////////////////////////////
//  Filter Code Definitions
//////////////////////////////////////////////////////////////

// maximum number of inputs that can be handled
// in one function call
#define MAX_INPUT_LEN   3000
// maximum length of filter than can be handled
#define MAX_FLT_LEN     63
// buffer to hold all of the input samples
#define BUFFER_LEN      (MAX_FLT_LEN - 1 + MAX_INPUT_LEN)

class FIR_FLOAT_1Ch
{
public:
    FIR_FLOAT_1Ch();
    ~FIR_FLOAT_1Ch();
    void firFloat( double *coeffs, double *input, double *output, int length, int filterLength, double Gain);
    double* firStoreNewSamples( double *inp, int length);
    void firMoveProcSamples( int length);
private:
    double insamp[BUFFER_LEN];
};

class Processing
{
public:
    Processing( int sample_size);
    ~Processing();
    void process(uint8_t **samples_in, int size , int options);
private:
    uint16_t *right_ch_in;
    uint16_t *left_ch_in;
    int16_t *right_ch_out;
    int16_t *left_ch_out;
    double *f_right_ch_out;
    double *f_left_ch_out;
    /*These buffers are used as intermediate filtering stage*/
    double *f_left_ch_out_1;
    double *f_left_ch_out_2;
    double *f_left_ch_out_3;
    double *f_left_ch_out_4;
    double *f_right_ch_out_1;
    double *f_right_ch_out_2;
    double *f_right_ch_out_3;
    double *f_right_ch_out_4;
    /***/
    double *f_right_ch_in;
    double *f_left_ch_in;
    FIR_FLOAT_1Ch *right_FIR;
    FIR_FLOAT_1Ch *left_FIR;
    FIR_FLOAT_1Ch *right_FIR2;
    FIR_FLOAT_1Ch *left_FIR2;
    FIR_FLOAT_1Ch *right_FIR3;
    FIR_FLOAT_1Ch *left_FIR3;
    FIR_FLOAT_1Ch *right_FIR4;
    FIR_FLOAT_1Ch *left_FIR4;
    FIR_FLOAT_1Ch *right_FIR5;
    FIR_FLOAT_1Ch *left_FIR5;
};

#endif



