#ifndef __CONTROLER_H
#define __CONTROLER_H

#include <pthread.h>
#include <threadclass.h>
#include <processing.h>

class Controler: public MyThreadClass
{
public:
    Controler(char* src_file_name, HQ_Context *ctx);
    Controler();
    ~Controler();
    void *control_thread(void *x_void_ptr);
private:
    void InternalThreadEntry();
    HQ_Context *m_ctx;
    char* m_src_file_name;
};

#endif
