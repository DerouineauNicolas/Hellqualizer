#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <processing.h>

// FIR init
FIR_FLOAT_1Ch::FIR_FLOAT_1Ch()
{
    memset( insamp, 0, sizeof( insamp ) );
}

// FIR init
FIR_FLOAT_1Ch::~FIR_FLOAT_1Ch()
{
    //free(insamp);
}

// store new input samples
double *FIR_FLOAT_1Ch::firStoreNewSamples( double *inp, int length )
{
    // put the new samples at the high end of the buffer
    memcpy( &insamp[MAX_FLT_LEN - 1], inp,
            length * sizeof(double) );
    // return the location at which to apply the filtering
    return &insamp[MAX_FLT_LEN - 1];
}

// move processed samples
void FIR_FLOAT_1Ch::firMoveProcSamples( int length )
{
    // shift input samples back in time for next time
    memmove( &insamp[0], &insamp[length],
            (MAX_FLT_LEN - 1) * sizeof(double) );
}

//// FIR init
//void firFixedInit( void )
//{
//    memset( insamp, 0, sizeof( insamp ) );
//}

Processing::Processing(int size){
    right_ch_in=(uint16_t*)malloc((size/2)*sizeof(uint16_t));
    left_ch_in=(uint16_t*)malloc((size/2)*sizeof(uint16_t));
    right_ch_out=(int16_t*)malloc((size/2)*sizeof(int16_t));
    left_ch_out=(int16_t*)malloc((size/2)*sizeof(int16_t));



    f_left_ch_in=(double*)malloc((size/2)*sizeof(double));
    f_left_ch_out_1=(double*)malloc((size/2)*sizeof(double));
    f_left_ch_out_2=(double*)malloc((size/2)*sizeof(double));
    f_left_ch_out_3=(double*)malloc((size/2)*sizeof(double));
    f_left_ch_out_4=(double*)malloc((size/2)*sizeof(double));
    f_left_ch_out=(double*)malloc((size/2)*sizeof(double));

    f_right_ch_in=(double*)malloc((size/2)*sizeof(double));
    f_right_ch_out_1=(double*)malloc((size/2)*sizeof(double));
    f_right_ch_out_2=(double*)malloc((size/2)*sizeof(double));
    f_right_ch_out_3=(double*)malloc((size/2)*sizeof(double));
    f_right_ch_out_4=(double*)malloc((size/2)*sizeof(double));
    f_right_ch_out=(double*)malloc((size/2)*sizeof(double));


    right_FIR=new FIR_FLOAT_1Ch();
    left_FIR=new FIR_FLOAT_1Ch();
}

Processing::~Processing(){
//    free(right_ch_in);
//    free(left_ch_in);
//    free(right_ch_out);
//    free(left_ch_out);
    free(right_ch_in);
    free(left_ch_in);
    free(right_ch_out);
    free(left_ch_out);



    free(f_left_ch_in);
    free(f_left_ch_out_1);
    free(f_left_ch_out_2);
    free(f_left_ch_out_3);
    free(f_left_ch_out_4);
    free(f_left_ch_out);

    free(f_right_ch_in);
    free(f_right_ch_out_1);
    free(f_right_ch_out_2);
    free(f_right_ch_out_3);
    free(f_right_ch_out_4);
    free(f_right_ch_out);
    delete right_FIR;
    delete left_FIR;
}

// the FIR filter function


// bandpass filter centred around between 5000 and 10000kHz
#define FILTER_LEN  63
double coeffs_bp_5000_10000[ FILTER_LEN ] =
{
    //    -0.0448093,0.005
    -0.0054685,0.0097457,-0.0053658,0.0015182,0.0154469,0.0165822,0.0007649,-0.011696,
    -0.0066276,0.0003359,-0.0077141,-0.0142352,0.0026186,0.0248596,0.0192814,-0.0030055,
    -0.0044339,0.0083433,-0.0068648,-0.0418269,-0.0363057,0.0162062,0.0423516,0.0134343,
    0.0046763,0.0557132,0.0621480,-0.0659334,-0.2016687,-0.1285787,0.1269571,0.2722713,0.1269571,
    -0.1285787,-0.2016687,-0.0659334,0.0621480,0.0557132,0.0046763,0.0134343,0.0423516,0.0162062,
    -0.0363057,-0.0418269,-0.0068648,0.0083433,-0.0044339,-0.0030055,0.0192814,0.0248596,0.0026186,-0.0142352,
    -0.0077141,0.0003359,-0.0066276,-0.011696,0.0007649,0.0165822,0.0154469,0.0015182,-0.0053658,0.0097457,
    -0.0054685
};

// Low pass filter from 0 to 5000kHz
double coeffs_lp_0_5000[ FILTER_LEN ] =
{
    0.0137572,-0.0021217,-0.0056138,-0.0074873,-0.0076700,-0.0073137,-0.0048236,0.0011468,0.0075766,0.0096836,0.0069478,0.0019516,-0.0041731,-0.0114084,
    -0.0160477,-0.0131499,-0.0028137,0.0092241,0.0183042,0.0224698,0.0190012,0.0048048,-0.0168556,-0.0358260,-0.0423656,-0.0324079,-0.0043603,
    0.0416495,0.0990781,0.1533275,0.1902280,0.2029056,0.1902280,0.1533275,0.0990781,0.0416495,-0.0043603,-0.0324079,-0.0423656,-0.0358260,
    -0.0168556,0.0048048,0.0190012,0.0224698,0.0183042,0.0092241,-0.0028137,-0.0131499,-0.0160477,-0.0114084,-0.0041731,0.0019516,0.0069478,
    0.0096836,0.0075766,0.0011468,-0.0048236,-0.0073137,-0.0076700,-0.0074873,-0.0056138,-0.0021217,0.0137572
};

// the FIR filter function
void FIR_FLOAT_1Ch::firFloat( double *coeffs, double *input, double *output,
               int length, int filterLength , double Gain)
{
    double acc;     // accumulator for MACs
    double *coeffp; // pointer to coefficients
    double *inputp; // pointer to input samples
    int n;
    int k;

    // apply the filter to each input sample
    for ( n = 0; n < length; n++ ) {
        // calculate output n
        coeffp = coeffs;
        inputp = &input[n];
                 //&insamp[filterLength - 1 + n];
        acc = 0;
        for ( k = 0; k < filterLength; k++ ) {
            acc += (*coeffp++) * (*inputp--);
        }
        output[n] = acc;
        //if(output>1.0)
    }
}

void intToFloat( int16_t *input, double *output, int length )
{
    int i;

    for ( i = 0; i < length; i++ ) {
        output[i] = (double)input[i];
    }
}

void floatToInt( double *input, int16_t *output, int length )
{
    int i;

    for ( i = 0; i < length; i++ ) {
        if ( input[i] > 32767.0 ) {
            input[i] = 32767.0;
        } else if ( input[i] < -32768.0 ) {
            input[i] = -32768.0;
        }
        // convert
        output[i] = (int16_t)input[i];
    }
}

void mix_samples(double *ch1, double *ch2, double *ch3, double *ch4, double *out, int num_samples)
{
    int s;
    double tmp;
    for (s=0;s<num_samples;s++){

        tmp=(ch1[s]+ch2[s]/2);
        out[s]=tmp;
    }

}

void Processing::process(uint8_t **samples_in, int size, int process){

    uint16_t **in = (uint16_t **)samples_in;
    //int16_t **tmp[3000];
    double *inp;

    if(process){

        for(int i=0;i<size/2;i++){
            if((i%2)==0){
                left_ch_in[i/2]=(*in)[i];
            }
            else{
                right_ch_in[i/2]=(*in)[i];
            }
        }

        //memset(left_ch_in,0, (size/4)*sizeof(uint16_t));

        intToFloat( (int16_t*)left_ch_in, f_left_ch_in, (size/4) );
        inp = left_FIR->firStoreNewSamples( f_left_ch_in, (size/4) );
        left_FIR->firFloat(coeffs_lp_0_5000, inp, f_left_ch_out_1, (size/4), FILTER_LEN, 1);
        left_FIR->firMoveProcSamples((size/4));

        //mix_samples(f_left_ch_out_1,f_left_ch_out_2,NULL,NULL,f_left_ch_out,(size/4));

        floatToInt( f_left_ch_out_1, left_ch_out, (size/4) );

        /*####################################*/
        intToFloat( (int16_t*)right_ch_in, f_right_ch_in, (size/4) );
        inp = right_FIR->firStoreNewSamples( f_right_ch_in, (size/4) );
        right_FIR->firFloat(coeffs_lp_0_5000, inp, f_right_ch_out_1, (size/4), FILTER_LEN, 1);
        right_FIR->firMoveProcSamples((size/4));

        floatToInt( f_right_ch_out_1, right_ch_out, (size/4) );


        for(int i=0;i<size/2;i++){
            if((i%2)==0)
                (*in)[i]=left_ch_out[i/2];
            else
                (*in)[i]=right_ch_out[i/2];
        }
    }
}
