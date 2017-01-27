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
    void firFloat( double *coeffs, double *input, double *output, int length, int filterLength );
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
    double *f_right_ch_in;
    double *f_left_ch_in;
    FIR_FLOAT_1Ch *right_FIR;
    FIR_FLOAT_1Ch *left_FIR;
};

#endif



