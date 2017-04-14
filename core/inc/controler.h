#ifndef __CONTROLER_H
#define __CONTROLER_H

#include <pthread.h>
#include <threadclass.h>
#include <processing.h>

class Controler: public MyThreadClass
{
public:
    Controler(char* src_file_name, int *endofdecoding, processing_options *processing_opt);
    Controler();
    ~Controler();
    void *control_thread(void *x_void_ptr);
private:
    void InternalThreadEntry();
    processing_options *m_processing_options;
    int *m_end_of_decoding;
    char* m_src_file_name;
};

#endif
