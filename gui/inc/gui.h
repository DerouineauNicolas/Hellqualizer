#ifndef __GUI_H
#define __GUI_H

#include <pthread.h>
#include <threadclass.h>
#include <processing.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <limits.h>


//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"


// Include GLM
//#include <glm/glm.hpp>

class GUI: public MyThreadClass
{
public:
    GUI(char* src_file_name, HQ_Context *ctx);
    GUI();
    ~GUI();
    void *gui_thread(void *x_void_ptr);
private:
    void InternalThreadEntry();
    processing_options *m_processing_options;
    char* m_src_file_name;
    HQ_Context *m_ctx;
    //GLFWwindow* m_window;
};

#endif
