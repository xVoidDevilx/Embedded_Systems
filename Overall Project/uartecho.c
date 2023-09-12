
#include <stdint.h>
#include <stddef.h>
#include <string.h>
/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include "ti_drivers_config.h"
/* My custom header file */
#include "chf.h"

//Driver function
void *mainThread(void *arg0){

    //init variables
    memset(glo.terminal_in, 0, sizeof(glo.terminal_in));
    memset(glo.terminal_out, 0, sizeof(glo.terminal_out));

    char input, proc, *token;
    const char  echoPrompt[] = "MSP432 Ready:\r\n";

    GlobalConfig(&glo);

    UART_write(glo.uart, echoPrompt, strlen(echoPrompt));

    for (;;) {
        //Echo typing
        UART_read(glo.uart, &input, 1);
        UART_write(glo.uart, &input, 1);
        // manage char from putty
        proc = ProcInp(input, sizeof(glo.terminal_in), &glo);

        switch (proc) {
            // Not processing
            case 'n':
                break;
            // Overflow occured
            case 'e':
                BFROVF.count++;
                UART_write(glo.uart, glo.terminal_out, strlen(glo.terminal_out));
                memset(glo.terminal_in, 0, sizeof(glo.terminal_in));
                memset(glo.terminal_out, 0, sizeof(glo.terminal_out));
                break;
            case 'y':
                /*Process the buffer*/
                token = strtok(glo.terminal_in, " \r\n");   //seperate by spaces + endlines
                UART_write(glo.uart, "\n\r", sizeof("\n\r"));

                // CLI CMD mapping
                if (token != NULL) {
                    if (!strcmp(token, "-about")) {
                        PrintAbout(glo.terminal_out, sizeof(glo.terminal_out));
                        UART_write(glo.uart, glo.terminal_out, strlen(glo.terminal_out));
                    }
                    else if (!strcmp(token, "-help")) {
                        HelpCMD(token, &glo);
                        UART_write(glo.uart, glo.terminal_out, strlen(glo.terminal_out));
                    }
                    else if (!strcmp(token, "-print")) {
                        PrintCMD(token, glo.terminal_out, sizeof(glo.terminal_out));
                        UART_write(glo.uart, glo.terminal_out, strlen(glo.terminal_out));
                    }
                    else if (!strcmp(token, "-memr")) {
                        MemrCMD(token, glo.terminal_out, sizeof(glo.terminal_out), &BADMEM);
                        UART_write(glo.uart, glo.terminal_out, strlen(glo.terminal_out));
                    }
                    else {
                        if (strlen(token)) {
                            BADCMD.count++;  //increase the errors occurred in operations
                            UART_write(glo.uart, "Error: Command not supported\n\r", sizeof("Error: Command not supported\n\r"));
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
