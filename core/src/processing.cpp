#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <processing.h>
#include <float_coeff.h>
#include <Hellqualizer.h>
#include <unistd.h>


#ifdef HQ_PROFILING
#include <profiling.h>
#endif

/*Number of samples to be processed everytime process is called*/
static int output_size=2048;

// FIR init
FIR_FLOAT_1Ch::FIR_FLOAT_1Ch()
{
    memset( insamp, 0, sizeof( insamp ) );
}

// FIR init
FIR_FLOAT_1Ch::~FIR_FLOAT_1Ch()
{
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


Processing::Processing(HQ_Context *ctx){
    right_ch_in=(uint16_t*)malloc((output_size/2)*sizeof(uint16_t));
    left_ch_in=(uint16_t*)malloc((output_size/2)*sizeof(uint16_t));
    right_ch_out=(int16_t*)malloc((output_size/2)*sizeof(int16_t));
    left_ch_out=(int16_t*)malloc((output_size/2)*sizeof(int16_t));

    f_left_ch_in=(double*)malloc((output_size/2)*sizeof(double));
    f_left_ch_out_tmp=(double*)malloc(NUM_EQ_BANDS*(output_size/2)*sizeof(double));
    f_left_ch_out=(double*)malloc((output_size/2)*sizeof(double));

    f_right_ch_in=(double*)malloc((output_size/2)*sizeof(double));
    f_right_ch_out_tmp=(double*)malloc(NUM_EQ_BANDS*(output_size/2)*sizeof(double));
    f_right_ch_out=(double*)malloc((output_size/2)*sizeof(double));


    right_FIR=new FIR_FLOAT_1Ch();
    left_FIR=new FIR_FLOAT_1Ch();

    m_mutex_decode_process=&ctx->m_mutex_decode_to_process;
    m_signal_decode_process=&ctx->m_signal_decode_to_process;
    m_buffer_decode_process=ctx->Buffer_decode_process;
    m_mutex_process_render=&ctx->m_mutex_process_to_render;
    m_signal_process_render=&ctx->m_signal_process_to_render;
    m_buffer_process_render=ctx->Buffer_process_render;

    m_ctx=ctx;

}

void *Processing::processing_thread(void *x_void_ptr)
{

    //Processing* processor=new Processing(output_size);
    //const int buffer_size=AVCODEC_MAX_AUDIO_FRAME_SIZE+ FF_INPUT_BUFFER_PADDING_SIZE;
    unsigned int chn;

    uint8_t *samples;
    samples=(uint8_t*)malloc(output_size*sizeof(uint8_t));

    //signed short *samples_out;

    int err;

    while(1){
        if(m_ctx->state==PLAY){
            pthread_mutex_lock(m_mutex_decode_process);
            //printf("INPUT_PROCESSING: %d \n",m_buffer_decode_process->GetReadAvail());
            while(m_buffer_decode_process->GetReadAvail()<output_size){
                if(m_ctx->state==END_OF_DECODING)
                    break;
                pthread_cond_wait(m_signal_decode_process, m_mutex_decode_process);
            }
            if(m_ctx->state==END_OF_DECODING)
                break;
            m_buffer_decode_process->Read(samples,output_size);
            this->process(&samples,output_size, m_ctx);
            //samples_out=(signed short*)samples;
            //            for(int i=0;i<(output_size/4);i++)
            //               printf("%d \n",*(samples_out+i));

//            if ((err = snd_pcm_writei (handle, samples_out, output_size/4)) != (output_size/4)) {
//                fprintf (stderr, "write to audio interface failed (%s)\n",
//                         snd_strerror (err));
//                //exit (1);
//            }
            pthread_mutex_unlock(m_mutex_decode_process);
            pthread_mutex_lock(m_mutex_process_render);
            //printf("OUTPUT_PROCESSING: %d \n",m_buffer_process_render->GetWriteAvail());
            while(m_buffer_process_render->GetWriteAvail()<output_size){
                if(m_ctx->state==END_OF_DECODING)
                    break;
                pthread_cond_wait(m_signal_decode_process, m_mutex_process_render);
            }
            if(m_ctx->state==END_OF_DECODING)
                break;
            m_buffer_process_render->Write(samples, output_size);
            pthread_mutex_unlock(m_mutex_process_render);
            pthread_cond_signal(m_signal_process_render);


        }
        else{
            usleep(1000000);
        }

    }

    //delete processor;

    free(samples);
}

void Processing::InternalThreadEntry(){
    this->processing_thread(NULL);
    //return;
}

Processing::~Processing(){
    free(right_ch_in);
    free(left_ch_in);
    free(right_ch_out);
    free(left_ch_out);

    free(f_left_ch_in);
    free(f_left_ch_out_tmp);
    free(f_left_ch_out);

    free(f_right_ch_in);
    free(f_right_ch_out_tmp);
    free(f_right_ch_out);
    delete right_FIR;
    delete left_FIR;
}

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
        output[n] = Gain * acc;
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

void mix_samples(double *ch, double *out, int num_samples)
{
    int s,c;
    double tmp;
    for (s=0;s<num_samples;s++){
        tmp=0.0;
        for(c=0;c<NUM_EQ_BANDS;c++)
           tmp+=ch[s+c*num_samples];
        out[s]=tmp/NUM_EQ_BANDS;
    }

}

#define GENERATE_FUNCTION(SAMPLING_RATE) void Processing::EQ_stereo_##SAMPLING_RATE(int size, processing_options options){ \
    double *inp; \
    intToFloat( (int16_t*)left_ch_in, f_left_ch_in, (size/4) ); \
    inp = left_FIR->firStoreNewSamples( f_left_ch_in, (size/4) );\
    left_FIR->firFloat(coeffs_lp_0_2000_FS_##SAMPLING_RATE, inp, f_left_ch_out_tmp, (size/4), FILTER_LEN_0_2000, options.GAIN[0]);\
    left_FIR->firFloat(coeffs_bp_2000_4000_FS_##SAMPLING_RATE, inp, f_left_ch_out_tmp+(size/4), (size/4), FILTER_LEN_2000_4000, options.GAIN[1]);\
    left_FIR->firFloat(coeffs_bp_4000_6000_FS_##SAMPLING_RATE, inp, f_left_ch_out_tmp+2*(size/4), (size/4), FILTER_LEN_4000_6000, options.GAIN[2]);\
    left_FIR->firFloat(coeffs_bp_6000_10000_FS_##SAMPLING_RATE, inp, f_left_ch_out_tmp+3*(size/4), (size/4), FILTER_LEN_6000_10000, options.GAIN[3]);\
    left_FIR->firFloat(coeffs_bp_10000_22000_FS_##SAMPLING_RATE, inp, f_left_ch_out_tmp+4*(size/4), (size/4), FILTER_LEN_10000_20000, options.GAIN[4]);\
    left_FIR->firMoveProcSamples((size/4));\
    mix_samples(f_left_ch_out_tmp,f_left_ch_out,(size/4));\
    floatToInt( f_left_ch_out, left_ch_out, (size/4) );\
    intToFloat( (int16_t*)right_ch_in, f_right_ch_in, (size/4) );\
    inp = right_FIR->firStoreNewSamples( f_right_ch_in, (size/4) );\
    right_FIR->firFloat(coeffs_lp_0_2000_FS_##SAMPLING_RATE, inp, f_right_ch_out_tmp, (size/4), FILTER_LEN_0_2000, options.GAIN[0]);\
    right_FIR->firFloat(coeffs_bp_2000_4000_FS_##SAMPLING_RATE, inp, f_right_ch_out_tmp+(size/4), (size/4), FILTER_LEN_2000_4000, options.GAIN[1]);\
    right_FIR->firFloat(coeffs_bp_4000_6000_FS_##SAMPLING_RATE, inp, f_right_ch_out_tmp+2*(size/4), (size/4), FILTER_LEN_4000_6000, options.GAIN[2]);\
    right_FIR->firFloat(coeffs_bp_6000_10000_FS_##SAMPLING_RATE, inp, f_right_ch_out_tmp+3*(size/4), (size/4), FILTER_LEN_6000_10000, options.GAIN[3]);\
    right_FIR->firFloat(coeffs_bp_10000_22000_FS_##SAMPLING_RATE, inp, f_right_ch_out_tmp+4*(size/4), (size/4), FILTER_LEN_10000_20000, options.GAIN[4]);\
    right_FIR->firMoveProcSamples((size/4));\
    mix_samples(f_right_ch_out_tmp,f_right_ch_out,(size/4));\
    floatToInt( f_right_ch_out, right_ch_out, (size/4) );\
}\

GENERATE_FUNCTION(44100)
GENERATE_FUNCTION(48000)

void Processing::process(uint8_t **samples_in, int size, HQ_Context *ctx){

    uint16_t **in = (uint16_t **)samples_in;
    processing_options options=ctx->proc_opt;

#ifdef HQ_PROFILING
    uint64_t before,after;
    before=rdtsc();
#endif

    if(options.do_process){

        for(int i=0;i<size/2;i++){
            if((i%2)==0){
                left_ch_in[i/2]=(*in)[i];
            }
            else{
                right_ch_in[i/2]=(*in)[i];
            }
        }


        if(ctx->Sampling_rate==44100)
            EQ_stereo_44100(size,options);
        else if(ctx->Sampling_rate==48000)
            EQ_stereo_48000(size,options);
        else{
            printf("[Processing] Sampling rate is not supported \n");
            return;
        }


        for(int i=0;i<size/2;i++){
            if((i%2)==0)
                (*in)[i]=left_ch_out[i/2];
            else
                (*in)[i]=right_ch_out[i/2];
        }
    }

#ifdef HQ_PROFILING
    after=rdtsc();
    printf("%d \n",after-before);
#endif


}
