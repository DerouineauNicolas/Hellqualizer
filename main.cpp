#include <demuxing_decoding.h>
#include <rendering.h>
#include <record.h>
#include <Hellqualizer.h>
#include <processing.h>


#ifdef HQ_GUI
#include <gui.h>
#else
#include <controler.h>
#endif

int main (int argc, char **argv)
{
    HQ_Context Ctx;
    DemuxDecode *decoder;
    char *src_filename=NULL;

    if ( (argc <2)) {
        fprintf(stderr, "Wrong Usage \n");
        exit(EXIT_SUCCESS);
    }

    InitHellqualizer();

    src_filename = argv[1];

    if (argc > 2) {
        if (!strcmp(argv[2], "-v")) {
            fprintf(stderr, "Wrong Usage \n");
            SetHellVerbosity(1);
        }
    }



    decoder = new DemuxDecode(src_filename);

    Rendering *renderer = new Rendering();
    Processing *processor = new Processing();

#ifdef HQ_GUI
    GUI *gui_control = new GUI(src_filename,&Ctx);
#else
    Controler *control = new Controler(src_filename);
#endif

    decoder->StartInternalThread();
    renderer->StartInternalThread();
    processor->StartInternalThread();

#ifdef HQ_GUI
    gui_control->StartInternalThread();
#else
    control->StartInternalThread();
#endif


    decoder->WaitForInternalThreadToExit();

    renderer->WaitForInternalThreadToExit();

#ifdef HQ_GUI
    gui_control->WaitForInternalThreadToExit();
#else
    control->WaitForInternalThreadToExit();
#endif


    delete decoder;
    delete renderer;
    delete processor;
    
#ifdef HQ_GUI
    delete gui_control;
#else
    delete control;
#endif

    DestroyHellqualizer();

}
