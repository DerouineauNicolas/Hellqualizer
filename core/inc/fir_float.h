#ifndef _FIR_FLOAT_H
#define _FIR_FLOAT_H
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

// the FIR filter function

#endif

