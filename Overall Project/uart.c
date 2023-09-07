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

//Macros
#define BACKSPACE 127   //ascii for backspace on putty

//Driver function
void *mainThread(void *arg0){

    // Supported functions array - keep this updated
    const CMD COMMANDS[] = {aboutCMD, helpCMD, printCMD, memrCMD};

    //init variables
    memset(glo.TERMINAL_IN, 0, sizeof(glo.TERMINAL_IN));
    memset(glo.TERMINAL_OUT, 0, sizeof(glo.TERMINAL_OUT));

    char input, *token;
    size_t i=0;
    const char  echoPrompt[] = "MSP432 Ready:\r\n";

    GlobalConfig(&glo);

    UART_write(glo.uart, echoPrompt, strlen(echoPrompt));

    for (;;) {
        //Echo typing
        UART_read(glo.uart, &input, 1);
        UART_write(glo.uart, &input, 1);
        // manage char from putty
        switch(input){

        //backspace
        case BACKSPACE: //putty set to 127 or backspace
        case '\b':
            i = i>0 ? i-1:i;
            glo.TERMINAL_IN[i] = 0;  //remove char
            break;

        //enter key (jumps down)
        case '\r':
        case '\n':
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
                    token = strtok(NULL, " -\n\r"); //update token
                    HelpCMD(token, glo.TERMINAL_OUT, sizeof(glo.TERMINAL_OUT), COMMANDS, sizeof(COMMANDS)/sizeof(COMMANDS[0]));
                    UART_write(glo.uart, glo.TERMINAL_OUT, strlen(glo.TERMINAL_OUT));
                    break;
                //-print command
                case 2:
                    token = strtok(NULL, " \n\r"); //update the token
                    PrintCMD(token, glo.TERMINAL_OUT, sizeof(glo.TERMINAL_OUT));
                    UART_write(glo.uart, glo.TERMINAL_OUT, strlen(glo.TERMINAL_OUT));
                    break;
                case 3:
                    token = strtok(NULL, " \n\r"); //update the token
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

                //clear buffers
                memset(glo.TERMINAL_IN, 0, sizeof(glo.TERMINAL_IN));
                memset(glo.TERMINAL_OUT, 0, sizeof(glo.TERMINAL_OUT));
                i = 0;
                break;

        //general input (lets you keep typing)
        default:
            if (i<MAXLEN) {
                glo.TERMINAL_IN[i] = input; //update the buffer
                i++;    //update the index
            }
            else {
                PrintERR(glo.TERMINAL_OUT, sizeof(glo.TERMINAL_OUT), BFR_OVF);
                UART_write(glo.uart, glo.TERMINAL_OUT, strlen(glo.TERMINAL_OUT));
                memset(glo.TERMINAL_IN, 0, sizeof(glo.TERMINAL_IN));
                i =0;
            }
            break;
        }
    }
}
