#ifndef __GUI_H
#define __GUI_H

#include <pthread.h>
#include <threadclass.h>
#include <processing.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Include GLM
//#include <glm/glm.hpp>

class GUI: public MyThreadClass
{
public:
    GUI(char* src_file_name, int *endofdecoding, processing_options *processing_opt);
    GUI();
    ~GUI();
    void *gui_thread(void *x_void_ptr);
private:
    void InternalThreadEntry();
    processing_options *m_processing_options;
    int *m_end_of_decoding;
    char* m_src_file_name;
    GLFWwindow* m_window;
};

#endif
