#include <controler.h>
#include <stdio.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>

using namespace std;


Controler::Controler(char* src_file_name)
{
   m_src_file_name=src_file_name;
}

void *Controler::control_thread(void *x_void_ptr)
{
    static struct termios oldt, newt;

    /*tcgetattr gets the parameters of the current terminal
    STDIN_FILENO will tell tcgetattr that it should write the settings
    of stdin to oldt*/
    tcgetattr( STDIN_FILENO, &oldt);
    /*now the settings will be copied*/
    newt = oldt;

    /*ICANON normally takes care that one line at a time will be processed
    that means it will return if it sees a "\n" or an EOF or an EOL*/
    newt.c_lflag &= ~(ICANON | ECHO);

    /*Those new settings will be set to STDIN
    TCSANOW tells tcsetattr to change attributes immediately. */
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

    while(1){
        if(context.state==END_OF_DECODING)
            break;
        char event=getchar();

        switch(event){
        case 'a':
            context.proc_opt.GAIN[0]+=0.1;
            printf("G[0]=%f \n", context.proc_opt.GAIN[0]);
            break;
        case 'q':
            context.proc_opt.GAIN[0]-=0.1;
            printf("G[0]=%f \n", context.proc_opt.GAIN[0]);
            break;
        case 'z':
            context.proc_opt.GAIN[1]+=0.1;
            printf("G[1]=%f \n", context.proc_opt.GAIN[1]);
            break;
        case 's':
            context.proc_opt.GAIN[1]-=0.1;
            printf("G[1]=%f \n", context.proc_opt.GAIN[1]);
            break;
        case 'e':
            context.proc_opt.GAIN[2]+=0.1;
            printf("G[2]=%f \n", context.proc_opt.GAIN[2]);
            break;
        case 'd':
            context.proc_opt.GAIN[2]-=0.1;
            printf("G[2]=%f \n", context.proc_opt.GAIN[2]);
            break;
        case 'r':
            context.proc_opt.GAIN[3]+=0.1;
            printf("G[3]=%f \n", context.proc_opt.GAIN[3]);
            break;
        case 'f':
            context.proc_opt.GAIN[3]-=0.1;
            printf("G[3]=%f \n", context.proc_opt.GAIN[3]);
            break;
        case 't':
            context.proc_opt.GAIN[4]+=0.1;
            printf("G[4]=%f \n", context.proc_opt.GAIN[4]);
            break;
        case 'g':
            context.proc_opt.GAIN[4]-=0.1;
            printf("G[4]=%f \n", context.proc_opt.GAIN[4]);
            break;
        case ' ':
            if(context.state==PLAY){
                printf("Pause \n");
                context.state=PAUSE;
            }else if(context.state==PAUSE){
                printf("Play \n");
                context.state=PLAY;
            }
            break;
        case 'x':
            //context.proc_opt.do_process=(context.proc_opt.do_process==1)?0:1;
            if(context.proc_opt.do_process){
                printf("Disable processing \n");
                context.proc_opt.do_process=0;
            }
            else
            {
                printf("Enable processing \n");
                context.proc_opt.do_process=1;
            }
            break;
        default:
            break;
        }

    }

    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);

    return NULL;

}

Controler::~Controler(){

}

void Controler::InternalThreadEntry(){
   this->control_thread(NULL);
}
