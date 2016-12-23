#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <processing.h>


// array to hold input samples
int16_t insamp[ BUFFER_LEN ];
double insamp2[ BUFFER_LEN ];

// FIR init
void firFloatInit( void )
{
    memset( insamp2, 0, sizeof( insamp ) );
}

// FIR init
void firFixedInit( void )
{
    memset( insamp, 0, sizeof( insamp ) );
}

Processing::Processing(int size){
    right_ch_in=(uint16_t*)malloc((size/2)*sizeof(uint16_t));
    left_ch_in=(uint16_t*)malloc((size/2)*sizeof(uint16_t));
    right_ch_out=(int16_t*)malloc((size/2)*sizeof(int16_t));
    left_ch_out=(int16_t*)malloc((size/2)*sizeof(int16_t));
    f_right_ch_out=(double*)malloc((size/2)*sizeof(double));
    f_left_ch_out=(double*)malloc((size/2)*sizeof(double));
    f_right_ch_in=(double*)malloc((size/2)*sizeof(double));
    f_left_ch_in=(double*)malloc((size/2)*sizeof(double));
    firFixedInit();
    firFloatInit();
}

Processing::~Processing(){
    free(right_ch_in);
    free(left_ch_in);
    free(right_ch_out);
    free(left_ch_out);
    free(f_right_ch_out);
    free(f_left_ch_out);
    free(f_right_ch_in);
    free(f_left_ch_in);
}

 

 

 
// the FIR filter function
void firFixed( int16_t *coeffs, int16_t *input, int16_t *output,
       int length, int filterLength )
{
    int32_t acc;     // accumulator for MACs
    int16_t *coeffp; // pointer to coefficients
    int16_t *inputp; // pointer to input samples
    int n;
    int k;
 
    // put the new samples at the high end of the buffer
    memcpy( &insamp[filterLength - 1], input,
            length * sizeof(int16_t) );
 
    // apply the filter to each input sample
    for ( n = 0; n < length; n++ ) {
        // calculate output n
        coeffp = coeffs;
        inputp = &insamp[filterLength - 1 + n];
        // load rounding constant
        acc = 1 << 14;
        // perform the multiply-accumulate
        for ( k = 0; k < filterLength; k++ ) {
            acc += (int32_t)(*coeffp++) * (int32_t)(*inputp--);
        }
//        // saturate the result
//        if ( acc > 0x3fffffff ) {
//            acc = 0x3fffffff;
//        } else if ( acc < -0x40000000 ) {
//            acc = -0x40000000;
//        }
        // convert from Q30 to Q15
        output[n] = (int16_t)(acc >> 15);
    }
 
    // shift input samples back in time for next time
    memmove( &insamp[0], &insamp[length],
            (filterLength - 1) * sizeof(int16_t) );
 
}
 
//////////////////////////////////////////////////////////////
//  Test program
//////////////////////////////////////////////////////////////
 
// bandpass filter centred around 1000 Hz
// sampling rate = 8000 Hz
// gain at 1000 Hz is about 1.13
 
#define FILTER_LEN  63
//int16_t coeffs[ FILTER_LEN ] =
//{
//    -179,319,-176,50,506,543,25,-383,-217,11,-253,-466,86
//    ,815,632,-98,-145,273,-225,-1371,-1190,531,1388,440,153,1826,2036,-2161,-6608,
//    -4213,4160,8922,4160,-4213,-6608,-2161,2036,1826,153,440,1388,531,-1190,-1371,-225,273,
//    -145,-98,632,815,86,-466,-253,11,-217,-383,25,543,506,50,-176,319,-179
//};

//#define FILTER_LEN  63
double coeffs2[ FILTER_LEN ] =
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

// the FIR filter function
void firFloat( double *coeffs, double *input, double *output,
       int length, int filterLength )
{
    double acc;     // accumulator for MACs
    double *coeffp; // pointer to coefficients
    double *inputp; // pointer to input samples
    int n;
    int k;

    // put the new samples at the high end of the buffer
    memcpy( &insamp2[filterLength - 1], input,
            length * sizeof(double) );

    // apply the filter to each input sample
    for ( n = 0; n < length; n++ ) {
        // calculate output n
        coeffp = coeffs;
        inputp = &insamp2[filterLength - 1 + n];
        acc = 0;
        for ( k = 0; k < filterLength; k++ ) {
            acc += (*coeffp++) * (*inputp--);
        }
        output[n] = acc;
        //if(output>1.0)
    }
    // shift input samples back in time for next time
    memmove( &insamp2[0], &insamp2[length],
            (filterLength - 1) * sizeof(double) );

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
 

void Processing::process(uint8_t **samples_in, int size){

    uint16_t **in = (uint16_t **)samples_in;
    //int16_t **tmp[3000];

    for(int i=0;i<size;i++){
        if((i%2)==0)
           left_ch_in[i/2]=(*in)[i];
        else
           right_ch_in[i/2]=(*in)[i];
    }

    //memccpy(right_ch_out,right_ch_in,sizeof(uint16_t),(size/2));
    //memset(left_ch_in,0,(size/2));
    //firFixed(coeffs,(int16_t*)right_ch_in,right_ch_out,(size/2),63);
    intToFloat( (int16_t*)right_ch_in, f_right_ch_in, (size/2) );
    //firFloat( coeffs2, f_right_ch_in, f_right_ch_out, (size/2),
    //       FILTER_LEN );
    memcpy((float*)f_right_ch_out,(float*)f_right_ch_in,(size/2)*sizeof(float));
    floatToInt( f_right_ch_out, right_ch_out, (size/2) );

    for(int i=0;i<size;i++){
        if((i%2)==0)
           (*in)[i]=left_ch_in[i/2];
        else
           (*in)[i]=right_ch_out[i/2];;
    }
}
