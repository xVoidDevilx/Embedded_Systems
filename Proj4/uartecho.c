//Standard imports
#include <stdint.h>
#include <stddef.h>
#include <string.h>
/* My custom header file */
#include "GoodFortune.h"

//Driver function
void *mainThread (void *arg0){

    //init variables
    memset(glo.terminal_in, 0, sizeof(glo.terminal_in));
    memset(glo.terminal_out, 0, sizeof(glo.terminal_out));

    char input, proc, *token;
    const char  echoPrompt[] = "MSP432 Ready:\r\n";

    InitializeDrivers();

    UART_write(glo.DEVICES.uart, echoPrompt, strlen(echoPrompt));

    for (;;) {
        //Echo typing
        UART_read(glo.DEVICES.uart, &input, 1);
        UART_write(glo.DEVICES.uart, &input, 1);
        // manage char from putty
        proc = ProcInp(input, sizeof(glo.terminal_in), &glo);

        switch (proc) {
            // Not processing
            case 'n':
                break;
            // Overflow occured
            case 'e':
                BFROVF.count++;
                UART_write(glo.DEVICES.uart0, glo.terminal_out, strlen(glo.terminal_out));
                memset(glo.terminal_in, 0, sizeof(glo.terminal_in));
                memset(glo.terminal_out, 0, sizeof(glo.terminal_out));
                break;
            case 'y':
                /*Process the buffer*/
                token = strtok(glo.terminal_in, " \r\n");   //seperate by spaces + endlines
                UART_write(glo.DEVICES.uart, "\n\r", sizeof("\n\r"));

                // CLI CMD mapping
                if (token != NULL) {
                    //about cmd
                    if (!strcmp(token, "-about")) {
                        PrintAbout(glo.terminal_out, sizeof(glo.terminal_out));
                        UART_write(glo.DEVICES.uart, glo.terminal_out, strlen(glo.terminal_out));
                    }
                    //help cmd
                    else if (!strcmp(token, "-help")) {
                        HelpCMD(token, &glo);
                        UART_write(glo.DEVICES.uart, glo.terminal_out, strlen(glo.terminal_out));
                    }
                    //print cmd
                    else if (!strcmp(token, "-print")) {
                        PrintCMD(token, glo.terminal_out, sizeof(glo.terminal_out));
                        UART_write(glo.DEVICES.uart, glo.terminal_out, strlen(glo.terminal_out));
                    }
                    //memr cmd
                    else if (!strcmp(token, "-memr")) {
                        MemrCMD(token, glo.terminal_out, sizeof(glo.terminal_out), &BADMEM);
                        UART_write(glo.DEVICES.uart, glo.terminal_out, strlen(glo.terminal_out));
                    }
                    //gpio cmd
                    else if(!strcmp(token, "-gpio")){
                        GPIOCMD(token, &glo, &BADCMD);
                        UART_write(glo.DEVICES.uart, glo.terminal_out, strlen(glo.terminal_out));
                    }
                    //errors cmd
                    else if(!strcmp(token, "-error")){
                        PrintErrs(token, &glo);
                    }
                    else {
                        if (strlen(token)) {
                            BADCMD.count++;  //increase the errors occurred in operations
                            UART_write(glo.DEVICES.uart, "Command not recognized\n\r", sizeof("Command not recognized\n\r"));
                        }
                    }
                }

                // Clear buffers
                memset(glo.terminal_in, 0, sizeof(glo.terminal_in));
                memset(glo.terminal_out, 0, sizeof(glo.terminal_out));
                break;
        }
    }
}
