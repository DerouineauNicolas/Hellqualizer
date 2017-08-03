#ifndef __RECORD_H
#define __RECORD_H

#include <pthread.h>
#include <ring_buffer.h>
#include <threadclass.h>
#include <Hellqualizer.h>

class RECORDER: public MyThreadClass
{
public:
    RECORDER(const char* device_name, HQ_Context *ctx);
    RECORDER();
    ~RECORDER();
    void *record_thread(void *x_void_ptr);
private:
    void InternalThreadEntry();
    pthread_mutex_t *m_mutex;
    pthread_cond_t *m_signal;
    RingBuffer *m_buffer;
    HQ_Context *m_ctx;
};

#endif

