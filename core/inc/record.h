#ifndef __RECORD_H
#define __RECORD_H

#include <pthread.h>
#include <ring_buffer.h>
#include <threadclass.h>
#include <Hellqualizer.h>

class RECORDER: public MyThreadClass
{
public:
    RECORDER(const char* src_file_name, HQ_Context *ctx);
    RECORDER();
    ~RECORDER();
    void *record_thread(void *x_void_ptr);
private:
    void InternalThreadEntry();
};

#endif

