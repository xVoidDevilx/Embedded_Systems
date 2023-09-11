/**
 * @file uartecho.c
 * @author Silas Rodriguez
 * @brief "Hello World" for MSP432
 * @version 0.2
 * @date 2023-08-26
 */

//header declarations
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "HEAD.h"

//Driver function
void *mainThread(void *arg0){

    // Supported functions array - keep this updated
    const CMD COMMANDS[] = {aboutCMD, helpCMD, printCMD, memrCMD};

    //init variables
    memset(glo.TERMINAL_IN, 0, sizeof(glo.TERMINAL_IN));
    memset(glo.TERMINAL_OUT, 0, sizeof(glo.TERMINAL_OUT));

    char input, proc, *token;
    const char  echoPrompt[] = "MSP432 Ready:\r\n";

    GlobalConfig(&glo);

    UART_write(glo.uart, echoPrompt, strlen(echoPrompt));

    for (;;) {
        //Echo typing
        UART_read(glo.uart, &input, 1);
        UART_write(glo.uart, &input, 1);
        // manage char from putty
        proc = ProcInp(input, sizeof(glo.TERMINAL_IN), &glo);
        switch (proc) {
            // Not processing
            case 'n':
                break;
            // Overflow occured
            case 'e':
                UART_write(glo.uart, glo.TERMINAL_OUT, strlen(glo.TERMINAL_OUT));
                memset(glo.TERMINAL_IN, 0, sizeof(glo.TERMINAL_IN));
                memset(glo.TERMINAL_OUT, 0, sizeof(glo.TERMINAL_OUT));
                break;
            case 'y':
                /*Process the buffer*/
                token = strtok(glo.TERMINAL_IN, " \r\n");   //seperate by spaces + endlines
                UART_write(glo.uart, "\n\r", sizeof("\n\r"));

                //CLI CMD mapping
                switch (ReturnCommandIndex(token, COMMANDS, sizeof(COMMANDS)/sizeof(COMMANDS[0]))){
                    //-about command
                    case 0:
                        PrintAbout(glo.TERMINAL_OUT, sizeof(glo.TERMINAL_OUT));
                        UART_write(glo.uart, glo.TERMINAL_OUT, strlen(glo.TERMINAL_OUT));
                        break;
                    //-help command
                    case 1:
                        HelpCMD(token, glo.TERMINAL_OUT, sizeof(glo.TERMINAL_OUT), COMMANDS, sizeof(COMMANDS)/sizeof(COMMANDS[0]));
                        UART_write(glo.uart, glo.TERMINAL_OUT, strlen(glo.TERMINAL_OUT));
                        break;
                    //-print command
                    case 2:
                        PrintCMD(token, glo.TERMINAL_OUT, sizeof(glo.TERMINAL_OUT));
                        UART_write(glo.uart, glo.TERMINAL_OUT, strlen(glo.TERMINAL_OUT));
                        break;
                    //-memr command
                    case 3:
                        MemrCMD(token, glo.TERMINAL_OUT, sizeof(glo.TERMINAL_OUT),glo.ERRORS);
                        UART_write(glo.uart, glo.TERMINAL_OUT, sizeof(glo.TERMINAL_OUT));
                        break;
                    //unsupported/empty command
                    default:
                        if(strlen(token) == 0){
                            UART_write(glo.uart, "\n\r", sizeof("\n\r"));
                        }
                        else {
                            glo.ERRORS+=1;  //increase the errors occured in operations
                            UART_write(glo.uart, "Error: Command not supported\n\r", sizeof("Error: Command not supported\n\r"));
                            break;
                        }
                }
                // clear buffers
                memset(glo.TERMINAL_IN, 0, sizeof(glo.TERMINAL_IN));
                memset(glo.TERMINAL_OUT, 0, sizeof(glo.TERMINAL_OUT));
                break;
            // -- Should be unreachable, but shit happens
            default:
                glo.ERRORS++;
                UART_write(glo.uart, "Proc Char failed\n\r", sizeof("Proc Char failed\n\r"));
                memset(glo.TERMINAL_IN, 0, sizeof(glo.TERMINAL_IN));
                memset(glo.TERMINAL_OUT, 0, sizeof(glo.TERMINAL_OUT));
                break;
        }
    }
}
