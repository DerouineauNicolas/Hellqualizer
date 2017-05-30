/* 
  A Minimal Capture Program
  This program opens an audio interface for capture, configures it for
  stereo, 16 bit, 44.1kHz, interleaved conventional read/write
  access. Then its reads a chunk of random data from it, and exits. It
  isn't meant to be a real program.
  From on Paul David's tutorial : http://equalarea.com/paul/alsa-audio.html
  Fixes rate and buffer problems
  sudo apt-get install libasound2-dev
  gcc -o alsa-record-example -lasound alsa-record-example.c && ./alsa-record-example hw:0
*/

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <record.h>


static unsigned int rate = 44100;
static snd_pcm_t *capture_handle;
static snd_pcm_hw_params_t *hw_params;
static snd_pcm_format_t format = SND_PCM_FORMAT_S16;
int i;
int err;
char *buffer;
	      
RECORDER::RECORDER(const char* device_name, HQ_Context *ctx)
{
  if ((err = snd_pcm_open (&capture_handle,"default", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    fprintf (stderr, "cannot open audio device %s (%s)\n", 
             "default",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "audio interface opened\n");
		   
  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params allocated\n");
				 
  if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params initialized\n");
	
  if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "cannot set access type (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params access setted\n");
	
  if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
    fprintf (stderr, "cannot set sample format (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params format setted\n");
	
  if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
    fprintf (stderr, "cannot set sample rate (%s)\n",
             snd_strerror (err));
    exit (1);
  }
	
  fprintf(stdout, "hw_params rate setted\n");

  if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2)) < 0) {
    fprintf (stderr, "cannot set channel count (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params channels setted\n");
	
  if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params setted\n");
	
  snd_pcm_hw_params_free (hw_params);

  fprintf(stdout, "hw_params freed\n");
	
  if ((err = snd_pcm_prepare (capture_handle)) < 0) {
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "audio interface prepared\n");

  buffer = (char*)malloc(128 * snd_pcm_format_width(format) / 8 * 2);

  fprintf(stdout, "buffer allocated\n");

  m_mutex=&ctx->m_mutex_decode_to_process;//mutex;
  m_signal=&ctx->m_signal_decode_to_process;
  m_buffer=ctx->Buffer_decode_process;

  ctx->Sampling_rate=rate;

  //exit (0);
}

void RECORDER::InternalThreadEntry(){
    this->record_thread(NULL);
    //return;
}


RECORDER::~RECORDER(){

}

void *RECORDER::record_thread(void *x_void_ptr)
{
    int buffer_frames = 8192;
    unsigned char *tmp=NULL;
    static int first_time=1;
    int current_buffer_size=0;

    while(1){
        if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
          fprintf (stderr, "read from audio interface failed (%s)\n",
                   err, snd_strerror (err));
          exit (1);
        }
        tmp=(unsigned char*)buffer;

        pthread_mutex_lock(m_mutex);
        m_buffer->Write(tmp, buffer_frames);
        pthread_mutex_unlock(m_mutex);
//        if(first_time){
//            pthread_mutex_lock(m_mutex);
//            current_buffer_size=m_buffer->GetReadAvail();
//            pthread_mutex_lock(m_mutex);
//            printf("[Record] Buff size =  %d",current_buffer_size);
//            if(current_buffer_size<44100)
//                continue;
//            else
//                first_time=0;

//        }
        pthread_cond_signal(m_signal);
        //fprintf(stdout, "read %d done\n", i);
    }

    free(buffer);

    fprintf(stdout, "buffer freed\n");

    snd_pcm_close (capture_handle);
    fprintf(stdout, "audio interface closed\n");

    return NULL;
}
