/**
 * @file parser.c
 * @author Silas Rodriguez
 * @brief "Hello World" for MSP432
 * @version 0.1
 * @date 2023-08-26
 */

//header declarations
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "HEAD.h"

//constants
#define BACKSPACE 127   //ascii for backspace

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>

/* Driver configuration */
#include "ti_drivers_config.h"

//function declarations
char *PrintCommands(char* OUTPUT, CMD COMMANDS[], size_t num_cmds);
size_t ReturnCommandIndex(char *str, CMD COMMANDS[], size_t num_cmds);

void *mainThread(void *arg0){
    //commands declaration - keep this updated
    CMD COMMANDS[] = {aboutCMD, helpCMD};

    //init variables
    char        uart_in[MAXLEN];
    memset(uart_in, 0, MAXLEN); //clear buffer

    char        *token;
    size_t      i=0;
    const char  echoPrompt[] = "MSP432 Ready:\r\n";
    UART_Handle uart;
    UART_Params uartParams;

    /* Call driver init functions */
    GPIO_init();
    UART_init();

    /* Configure the LED pin */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Turn on user LED */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.baudRate = 115200;

    uart = UART_open(CONFIG_UART_0, &uartParams);

    if (uart == NULL) {
        /* UART_open() failed */
        while (1);
    }

    UART_write(uart, echoPrompt, sizeof(echoPrompt));

    /* Loop forever echoing */
    while (1) {
        //Echo typing
        UART_read(uart, &uart_in[i], 1);
        UART_write(uart, &uart_in[i], 1);

        // manage char from putty
        switch(uart_in[i]){

        //backspace
        case BACKSPACE:
            uart_in[i] = 0;
            i = i>0 ? i-1:i;
            break;

        //enter key (jumps down)
        case '\r':
        case '\n':
            /*Process the buffer*/
            token = strtok(uart_in, " \r\n");   //seperate by spaces + endlines
            UART_write(uart, "\n\r", sizeof("\n\r"));

            //CLI CMD mapping
            switch (ReturnCommandIndex(token, COMMANDS, sizeof(COMMANDS)/sizeof(COMMANDS[0]))){
            //-about command
            case 0:
                UART_write(uart, aboutCMD.output, sizeof(aboutCMD.output));
                break;
            //-help command
            case 1:
                PrintCommands(COMMANDS[1].output, COMMANDS, sizeof(COMMANDS)/sizeof(COMMANDS[0]));
                UART_write(uart, COMMANDS[1].output, sizeof(COMMANDS[1].output));
                break;
            //unsupported command
            default:
                UART_write(uart, "Error: Command not supported\n\r", sizeof("Error: Command not supported\n\r"));
                break;
            }

            //clear buffer
            memset(token, 0, MAXLEN);
            i = 0;
            break;

        //general input (lets you keep typing)
        default:
            i = i<MAXLEN ? i+1:i;
            break;
        }
    }
}

/* Function Definitions */

/*
    return a formatted string of all the supported commands
*/
char* PrintCommands(char* OUTPUT, CMD COMMANDS[], size_t num_cmds) {
    const char MSG[] = "Commands currently supported:\r\n\t";
    size_t i;
    strncpy(OUTPUT, MSG, sizeof(MSG));
    for (i = 0; i < num_cmds; i++) {
        // Format and concatenate the command name and description
        snprintf(OUTPUT + strlen(OUTPUT),
                 MAXLEN,                 // Maximum characters to write
                 "%-8s | %s",              // Format string
                 COMMANDS[i].name,      // Command name
                 COMMANDS[i].description // Command description
        );
        strncat(OUTPUT, "\r\n\t", sizeof("\r\n\t"));
    }

    // Remove the last tab and add a new line
    strncat(OUTPUT, "\r\n", sizeof("\r\n"));
    return OUTPUT;
}

/*
    search for the passed string in the commands const and return the index of found command, returns len+1 over if not found
*/
size_t ReturnCommandIndex(char *str, CMD COMMANDS[], size_t num_cmds){
    size_t i;
    for (i = 0; i < num_cmds; i++){
        if (strncmp(str, COMMANDS[i].name, MAXLEN)==0){
            break;
        }
    }
    return i;
}
