/*
 * HEAD.c
 *
 *  Created on: Aug 27, 2023
 *      Author: Silas Rodriguez
 *      Purpose: Define commands that will be used on the MSP432 for the Embedded Systems course @ TTU
 */
#include <string.h>
#include "HEAD.h"

// -------------------------------------------------------------------\\

// Global object
GLOBAL glo = {
       .TERMINAL_IN ="",
       .TERMINAL_OUT = ""
};

// -------------------------------------------------------------------\\

// Commands + descriptions
CMD aboutCMD = {
    .name = "-about",
    .description = "Get information about the program currently running."
};

CMD helpCMD = {
    .name = "-help",
    .description = "Display available commands & their descriptions."
};

// -------------------------------------------------------------------\\

// Error Objects
ERROR BFR_OVF = {
      .code = -69,
      .msg = "String Overflow - Buffer Dumped"
};
// -------------------------------------------------------------------\\

// Supporting functions

/*
 *  Global obj configurator that sets up handlers & other data
 */
void GlobalConfig(GLOBAL *obj){
    /* Call driver init functions */
    GPIO_init();
    UART_init();
    /* Configure the LED pin */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    /* Turn on user LED */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    /* Create a UART with data processing off. */
    UART_Params_init(&obj->uartParams);
    obj->uartParams.writeDataMode = UART_DATA_BINARY;
    obj->uartParams.readDataMode = UART_DATA_BINARY;
    obj->uartParams.readReturnMode = UART_RETURN_FULL;
    obj->uartParams.baudRate = 115200;
    obj->uart = UART_open(CONFIG_UART_0, &obj->uartParams);

    if (obj->uart == NULL) {
        /* UART_open() failed */
        while (1);
    }
}
/*
    modifies OUTPUT with formatted string of all the supported commands
*/
void PrintAllCommands(char* OUTPUT, size_t buffer_size, const CMD COMMANDS[], size_t num_cmds) {
    //Define local vars
    const char MSG[] = "\n\rCommands currently supported:\r\n\t";
    size_t space_left = buffer_size - strlen(MSG) - 2; // Account for newline and null-terminator

    if (space_left <= 1) {
        OUTPUT[0] = '\0';
        return;
    }
    strcpy(OUTPUT, MSG);

    //itterate through commands and add them to the buffer
    size_t i;
    for (i = 0; i < num_cmds && space_left > 0; i++) {
        int chars_written = snprintf(OUTPUT + strlen(OUTPUT), space_left,
                                     "%-8s | %s\r\n\t",
                                     COMMANDS[i].name,
                                     COMMANDS[i].description);

        space_left = (chars_written >= 0 && (size_t)chars_written < space_left) ? space_left - chars_written : 0;
    }

    // Ensure null-termination & return
    OUTPUT[buffer_size - space_left] = '\r';
    OUTPUT[buffer_size - space_left - 1] = '\0';
}

/*
    search for the passed string in the commands const and return the index of found command, returns len+1 over if not found
*/
size_t ReturnCommandIndex(char *str, const CMD COMMANDS[], size_t num_cmds){
    size_t i;
    for (i = 0; i < num_cmds; i++){
        if (strncmp(str, COMMANDS[i].name, MAXLEN)==0){
            break;
        }
    }
    return i;
}

/*
 *  Print information about the build of the program
 */
void PrintAbout(char* OUTPUT, size_t buffer_size) {
    //Define local vars
    size_t space_left = buffer_size - 2; // Account for newline and null-terminator
    int chars_made = snprintf(OUTPUT, space_left, "\r\n\tEngineer: %20s"
                            "\n\r\tDate | Time: %13s | %s"
                            "\n\r\tVersion: %9.1f"
                            "\n\r\tAssignment %d: %12s\n\r", "Silas Rodriguez",__DATE__, __TIME__, 0.1, 1,"Get Started");

    // Ensure null-termination
    OUTPUT[buffer_size - space_left - (size_t)chars_made-1] = '\0';
}

/*
 *  Print error information
 */
void PrintERR(char* OUTPUT, size_t buffer_size, ERROR err) {
    //Define local vars
    size_t space_left = buffer_size - 2; // Account for newline and null-terminator
    int chars_made = snprintf(OUTPUT, space_left, "\n\rERROR %d: %s\n\r", err.code, err.msg);

    // Ensure null-termination
    OUTPUT[buffer_size - space_left - (size_t)chars_made-1] = '\0';
}
