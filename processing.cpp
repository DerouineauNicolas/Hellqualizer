#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <processing.h>

Processing::Processing(int size){
    right_ch_in=(uint16_t*)malloc((size/2)*sizeof(uint16_t));
    left_ch_in=(uint16_t*)malloc((size/2)*sizeof(uint16_t));
    //right_ch_out=(uint16_t*)malloc((size/2)*sizeof(uint16_t));
    //left_ch_out=(uint16_t*)malloc((size/2)*sizeof(uint16_t));
}

Processing::~Processing(){
    free(right_ch_in);
    free(left_ch_in);
}

 
// array to hold input samples
int16_t insamp[ BUFFER_LEN ];
 
// FIR init
void firFixedInit( void )
{
    memset( insamp, 0, sizeof( insamp ) );
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
        // saturate the result 
        if ( acc > 0x3fffffff ) {
            acc = 0x3fffffff;
        } else if ( acc < -0x40000000 ) { 
            acc = -0x40000000; 
        } 
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
int16_t coeffs[ FILTER_LEN ] =
{
    -179,319,-176,50,506,543,25,-383,-217,11,-253,-466,86
    ,815,632,-98,-145,273,-225,-1371,-1190,531,1388,440,153,1826,2036,-2161,-6608,
    -4213,4160,8922,4160,-4213,-6608,-2161,2036,1826,153,440,1388,531,-1190,-1371,-225,273,
    -145,-98,632,815,86,-466,-253,11,-217,-383,25,543,506,50,-176,319,-179
};
 
// number of samples to read per loop
#define SAMPLES   80 

void Processing::process(uint8_t **samples_in, int size){

    uint16_t **in = (uint16_t **)samples_in;

    for(int i=0;i<size;i++){
        if((i%2)==0)
           left_ch_in[i/2]=(*in)[i];
        else
           right_ch_in[i/2]=(*in)[i];
    }

    //memset(right_ch_in,0,(size/2));
    //firFixed(coeffs,(int16_t*)left_ch_in,(int16_t*)left_ch_in,(size/2),63);

    for(int i=0;i<size;i++){
        if((i%2)==0)
           (*in)[i]=left_ch_in[i/2];
        else
           (*in)[i]=right_ch_in[i/2];;
    }
}
