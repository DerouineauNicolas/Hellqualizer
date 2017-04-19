#include <gui.h>
#include <stdio.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>

using namespace std;


GUI::GUI(char* src_file_name, int *endofdecoding, processing_options *processing_opt)
{
   m_src_file_name=src_file_name;
   m_end_of_decoding=endofdecoding;
   m_processing_options=processing_opt;
}

void *GUI::gui_thread(void *x_void_ptr)
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        //return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);


    // Open a window and create its OpenGL context
    m_window = glfwCreateWindow( 1024, 768, "Hellqualizer", NULL, NULL);
    if( m_window == NULL ){
        fprintf( stderr, "Failed to open GLFW window.\n" );
        getchar();
        glfwTerminate();
        //return -1;
    }
    glfwMakeContextCurrent(m_window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        //return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    do{
        // Clear the screen. It's not mentioned before Tutorial 02, but it can cause flickering, so it's there nonetheless.
        glClear( GL_COLOR_BUFFER_BIT );

        // Draw nothing, see you in tutorial 2 !


        // Swap buffers
        glfwSwapBuffers(m_window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(m_window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(m_window) == 0 );

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;

}

GUI::~GUI(){

}

void GUI::InternalThreadEntry(){
   this->gui_thread(NULL);
}
